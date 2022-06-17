// Copyright 2022 PingCAP, Ltd.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <common/arithmeticOverflow.h>
#include <gtest/gtest.h>

TEST(OVERFLOW_Suite, SimpleTest)
{
    /// mul int128
    __int128 res128;
    bool is_overflow;
    /// 2^126
    static constexpr __int128 int_126 = __int128(__int128(1) << 126);

    /// 2^126 << 0 = 2^126
    is_overflow = common::mulOverflow(int_126, __int128(1), res128);
    ASSERT_EQ(is_overflow, false);

    /// 2^126 << 1 = 2^127
    is_overflow = common::mulOverflow(int_126, __int128(2), res128);
    ASSERT_EQ(is_overflow, true);

    /// 2^126 << 2 = 2^128
    is_overflow = common::mulOverflow(int_126, __int128(4), res128);
    ASSERT_EQ(is_overflow, true);
}