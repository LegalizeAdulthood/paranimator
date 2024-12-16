#pragma once

#include <iosfwd>
#include <memory>
#include <string>
#include <vector>

namespace ParFile
{

struct Parameter
{
    std::string name;
    std::string value;
};

struct ParSet
{
    std::string name;
    std::vector<Parameter> params;
};

class ParFile
{
public:
    using const_iterator = std::vector<ParSet>::const_iterator;
    using iterator = std::vector<ParSet>::iterator;

    virtual ~ParFile() = default;

    virtual bool empty() const = 0;
    virtual size_t size() const = 0;
    virtual const_iterator cbegin() const = 0;
    virtual iterator begin() = 0;
    virtual const_iterator cend() const = 0;
    virtual iterator end() = 0;
};

using ParFilePtr = std::shared_ptr<ParFile>;

ParFilePtr create(std::istream &contents);

}
