// SPDX-License-Identifier: GPL-3.0-only
//
#pragma once

#include <TestParFile/test.h>

#include <gtest/gtest.h>

#include <cctype>
#include <filesystem>
#include <string>
#include <string_view>

namespace TestParFile
{

inline std::string test_path_component(std::string_view name)
{
    std::string result;
    result.reserve(name.size());
    for (const char ch : name)
    {
        const auto value{static_cast<unsigned char>(ch)};
        result.push_back(std::isalnum(value) || ch == '-' || ch == '_' ? ch : '_');
    }
    return result.empty() ? std::string{"unknown"} : result;
}

inline std::filesystem::path test_output_directory()
{
    const testing::TestInfo *const test{testing::UnitTest::GetInstance()->current_test_info()};
    std::filesystem::path result{TEST_OUTPUT_DIRECTORY};
    if (test == nullptr)
    {
        return result / "unknown";
    }
    return result / test_path_component(test->test_suite_name()) / test_path_component(test->name());
}

} // namespace TestParFile
