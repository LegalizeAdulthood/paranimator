// SPDX-License-Identifier: GPL-3.0-only
//
#include "StringUtil.h"

#include <cctype>
#include <cstddef>

namespace ParFile
{

namespace
{

std::string trim(std::string_view text)
{
    const auto is_space{[](char c) { return std::isspace(static_cast<unsigned char>(c)) != 0; }};
    auto first{text.begin()};
    auto last{text.end()};
    while (first != last && is_space(*first))
    {
        ++first;
    }
    while (last != first && is_space(*(last - 1)))
    {
        --last;
    }
    return {first, last};
}

} // namespace

std::vector<std::string> split_slash_values(std::string_view text, TrimSlashValues trim_values)
{
    std::vector<std::string> result;
    std::size_t start{};
    while (start <= text.size())
    {
        const std::size_t slash{text.find('/', start)};
        const std::size_t end{slash == std::string_view::npos ? text.size() : slash};
        const std::string_view value{text.substr(start, end - start)};
        if (trim_values == TrimSlashValues::YES)
        {
            result.push_back(trim(value));
        }
        else
        {
            result.emplace_back(value.data(), value.size());
        }
        if (slash == std::string_view::npos)
        {
            break;
        }
        start = slash + 1U;
    }
    return result;
}

} // namespace ParFile
