#include <ParFile/ParFile.h>

#include <cassert>
#include <iostream>

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

StreamParFile::StreamParFile(std::istream &contents)
{
    std::string line;
    while (contents)
    {
        std::getline(contents, line);
        if (contents && !line.empty())
        {
            const std::string name{line.substr(0, line.find_first_of('{')-1)};
            m_param_sets.push_back({name});
            ParSet &param_set{m_param_sets.back()};
            while (line.find_first_of('}') == std::string::npos)
            {
                while (contents)
                {
                    std::getline(contents, line);
                    while (contents && !line.empty())
                    {
                        while (contents && line.back() == '\\')
                        {
                            std::string continuation;
                            std::getline(contents, continuation);
                            line.pop_back();
                            auto not_space{continuation.find_first_not_of(' ')};
                            continuation.erase(0, not_space);
                            line += continuation;
                        }
                        if (const auto semi = line.find_first_of(';'); semi != std::string::npos)
                        {
                            const auto end = line.find_last_not_of(' ', semi);
                            line.erase(end, std::string::npos);
                        }
                        const auto not_space{line.find_first_not_of(' ')};
                        line.erase(0, not_space);
                        assert(line[0] != ' ');
                        if (line.empty() || line[0] == '}')
                        {
                            break;
                        }
                        const auto next_space{line.find_first_of(' ')};
                        const std::string name_value{line.substr(0, next_space)};
                        line.erase(0, line.find_first_not_of(' ', next_space));
                        const auto equal{name_value.find('=')};
                        Parameter param;
                        if (equal == std::string::npos)
                        {
                            param.name = name_value;
                        }
                        else
                        {
                            param.name = name_value.substr(0, equal);
                            param.value = name_value.substr(equal+1);
                        }
                        param_set.params.push_back(param);
                    }
                }
            }
        }
    }
}

} // namespace

ParFilePtr create(std::istream &contents)
{
    return std::make_shared<StreamParFile>(contents);
}

} // namespace ParFile
