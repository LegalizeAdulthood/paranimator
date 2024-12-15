#include <ParFile/ParFile.h>

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
    if (contents)
    {
        m_param_sets.push_back({});
    }
}

} // namespace

ParFilePtr create(std::istream &contents)
{
    return std::make_shared<StreamParFile>(contents);
}

} // namespace ParFile
