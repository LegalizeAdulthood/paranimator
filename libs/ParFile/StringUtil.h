// SPDX-License-Identifier: GPL-3.0-only
//
#pragma once

#include <string>
#include <string_view>
#include <vector>

namespace ParFile
{

enum class TrimSlashValues
{
    NO,
    YES
};

std::string trim(std::string_view text);
std::vector<std::string> split_slash_values(std::string_view text, TrimSlashValues trim_values = TrimSlashValues::NO);

} // namespace ParFile
