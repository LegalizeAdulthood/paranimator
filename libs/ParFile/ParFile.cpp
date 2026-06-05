// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/ParFile.h>

#include <StringUtil.h>

#include <algorithm>
#include <istream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

namespace ParFile
{

namespace
{

class StreamParFile : public ParFile
{
public:
    StreamParFile(std::istream &contents);

    bool empty() const override
    {
        return m_param_sets.empty();
    }
    size_t size() const override
    {
        return m_param_sets.size();
    }
    const_iterator cbegin() const override
    {
        return m_param_sets.cbegin();
    }
    iterator begin() override
    {
        return m_param_sets.begin();
    }
    const_iterator cend() const override
    {
        return m_param_sets.cend();
    }
    iterator end() override
    {
        return m_param_sets.end();
    }

private:
    std::vector<ParSet> m_param_sets;
};

std::vector<std::string> split_space_values(std::string_view text)
{
    std::vector<std::string> result;
    std::size_t start{};
    while (start < text.size())
    {
        while (start < text.size() && text[start] == ' ')
        {
            ++start;
        }
        const std::size_t end{text.find(' ', start)};
        if (end == std::string_view::npos)
        {
            if (start < text.size())
            {
                result.emplace_back(text.substr(start));
            }
            break;
        }
        if (end != start)
        {
            result.emplace_back(text.substr(start, end - start));
        }
        start = end + 1U;
    }
    return result;
}

void get_content_line(std::istream &contents, std::string &line)
{
    std::getline(contents, line);
    line = trim(line);
    while (contents && !line.empty() && line.back() == '\\')
    {
        std::string continuation;
        std::getline(contents, continuation);
        continuation = trim(continuation);
        line.pop_back();
        line += continuation;
    }
    if (const auto semi = line.find_first_of(';'); semi != std::string::npos)
    {
        line.erase(semi, std::string::npos);
    }
    line = trim(line);
}

StreamParFile::StreamParFile(std::istream &contents)
{
    std::string line;
    while (contents)
    {
        get_content_line(contents, line);
        if (contents && !line.empty())
        {
            ParSet param_set;
            param_set.name = line.substr(0, line.find_first_of('{') - 1);
            param_set.name = trim(param_set.name);
            while (contents && !line.empty() && line.find_first_of('}') == std::string::npos)
            {
                get_content_line(contents, line);
                for (const std::string &name_value : split_space_values(line))
                {
                    if (name_value == "}")
                    {
                        break;
                    }
                    const auto equal{name_value.find('=')};
                    Parameter param;
                    if (equal == std::string::npos)
                    {
                        param.name = name_value;
                    }
                    else
                    {
                        param.name = name_value.substr(0, equal);
                        param.value = name_value.substr(equal + 1);
                    }
                    param_set.params.emplace_back(std::move(param));
                }
            }
            m_param_sets.emplace_back(std::move(param_set));
        }
    }
}

} // namespace

bool operator==(const ParSet &lhs, const ParSet &rhs)
{
    if (lhs.name != rhs.name || lhs.params.size() != rhs.params.size())
    {
        return false;
    }

    std::vector lhs_sorted{lhs.params};
    const auto less_names = [](const Parameter &left, const Parameter &right) { return left.name < right.name; };
    std::sort(lhs_sorted.begin(), lhs_sorted.end(), less_names);
    std::vector rhs_sorted{rhs.params};
    std::sort(rhs_sorted.begin(), rhs_sorted.end(), less_names);

    for (size_t i = 0; i < lhs_sorted.size(); ++i)
    {
        if (lhs_sorted[i] != rhs_sorted[i])
        {
            return false;
        }
    }

    return true;
}

std::ostream &operator<<(std::ostream &str, const ParSet &value)
{
    if (value.name.empty())
    {
        return str;
    }
    str << value.name << " {\n";
    for (const Parameter &param : value.params)
    {
        std::string line{"    " + to_string(param)};
        constexpr size_t LINE_LENGTH{70};
        while (line.length() >= LINE_LENGTH)
        {
            str << line.substr(0, LINE_LENGTH - 1) << "\\\n";
            line.erase(0, LINE_LENGTH - 1);
            line.insert(0, 8, ' ');
        }
        if (!line.empty())
        {
            str << line << '\n';
        }
    }
    return str << "}\n";
}

std::string to_string(const ParSet &value)
{
    if (value.name.empty())
    {
        return {};
    }
    std::ostringstream result;
    result << value;
    return result.str();
}

ParFilePtr create_par_file(std::istream &contents)
{
    return std::make_shared<StreamParFile>(contents);
}

} // namespace ParFile
