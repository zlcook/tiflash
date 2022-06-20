/*
 * Copyright 2022 PingCAP, Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <Storages/DeltaMerge/DeltaMergeDefines.h>
#include <Storages/DeltaMerge/DeltaTree.h>
#include <benchmark/benchmark.h>

#include <thread>

using namespace DB;
using namespace DB::DM;

class BenchValueSpace;
using ArenaTree = DeltaTree<BenchValueSpace, DT_M, DT_F, DT_S, ArenaAllocator>;
using SystemTree = DeltaTree<BenchValueSpace, DT_M, DT_F, DT_S, SystemAllocator>;
using LevelTree = DeltaTree<BenchValueSpace, DT_M, DT_F, DT_S, MultiLevelAllocator>;

class BenchValueSpace
{
    using ValueSpace = BenchValueSpace;

public:
    void removeFromInsert(UInt64)
    {
    }

    void removeFromModify(UInt64, size_t)
    {
    }

    UInt64 withModify(UInt64 old_tuple_id, const ValueSpace & /*modify_value_space*/, const RefTuple &) // NOLINT(readability-convert-member-functions-to-static)
    {
        return old_tuple_id;
    }
};

template <class Tree>
static void BM_deltatree(benchmark::State & state)
{
    static std::shared_ptr<Tree> center = std::make_shared<Tree>();
    static std::mutex center_lock;
    static std::atomic_size_t finished;
    for (auto _ : state)
    {
        for (int j = 0; j < 64; ++j)
        {
            std::shared_ptr<Tree> tree;
            {
                std::unique_lock lock{center_lock};
                tree = std::make_shared<Tree>(*center);
            }
            for (int i = 0; i < 8192; ++i)
            {
                tree->addInsert(i, i);
            }
            for (int i = 0; i < 8192; ++i)
            {
                tree->addDelete(i);
            }
            {
                std::unique_lock lock{center_lock};
                std::swap(center, tree);
            }
        }
        finished++;

        if (state.thread_index() == 0)
        {
            while (finished.load() != static_cast<size_t>(state.threads()))
            {
                std::this_thread::yield();
            }
            center = std::make_shared<Tree>();
            finished = 0;
        }
        else
        {
            while (finished.load())
            {
                std::this_thread::yield();
            }
        }
    }
}
BENCHMARK_TEMPLATE(BM_deltatree, LevelTree)->Threads(std::thread::hardware_concurrency())->Iterations(50);
BENCHMARK_TEMPLATE(BM_deltatree, SystemTree)->Threads(std::thread::hardware_concurrency())->Iterations(50);
BENCHMARK_TEMPLATE(BM_deltatree, ArenaTree)->Threads(std::thread::hardware_concurrency())->Iterations(50);