// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/ParFile.h>

#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/trim.hpp>

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

void get_content_line(std::istream &contents, std::string &line)
{
    std::getline(contents, line);
    boost::algorithm::trim(line);
    while (contents && !line.empty() && line.back() == '\\')
    {
        std::string continuation;
        std::getline(contents, continuation);
        boost::algorithm::trim(continuation);
        line.pop_back();
        line += continuation;
    }
    if (const auto semi = line.find_first_of(';'); semi != std::string::npos)
    {
        line.erase(semi, std::string::npos);
    }
    boost::algorithm::trim(line);
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
            boost::algorithm::trim(param_set.name);
            while (contents && !line.empty() && line.find_first_of('}') == std::string::npos)
            {
                get_content_line(contents, line);
                std::vector<std::string> params;
                split(params, line, [](char c) { return c == ' '; }, boost::algorithm::token_compress_on);
                for (const std::string &name_value : params)
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
        str << "    " << param << '\n';
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

ParFilePtr createParFile(std::istream &contents)
{
    return std::make_shared<StreamParFile>(contents);
}

} // namespace ParFile
