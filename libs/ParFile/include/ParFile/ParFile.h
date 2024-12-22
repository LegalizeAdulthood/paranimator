#pragma once

#include <memory>
#include <ostream>
#include <string>
#include <vector>

namespace ParFile
{

struct Parameter
{
    std::string name;
    std::string value;
};

inline bool operator==(const Parameter &lhs, const Parameter &rhs)
{
    return lhs.name == rhs.name && lhs.value == rhs.value;
}
inline bool operator!=(const Parameter &lhs, const Parameter &rhs)
{
    return !(lhs == rhs);
}

inline std::string to_string(const Parameter &par)
{
    if (par.value.empty())
    {
        return par.name;
    }
    return par.name + '=' + par.value;
}

inline std::ostream &operator<<(std::ostream &str, const Parameter &value)
{
    return str << to_string(value);
}

struct ParSet
{
    std::string name;
    std::vector<Parameter> params;
};

bool operator==(const ParSet &lhs, const ParSet &rhs);
inline bool operator!=(const ParSet &lhs, const ParSet &rhs)
{
    return !(lhs == rhs);
}

std::ostream &operator<<(std::ostream &str, const ParSet &value);
std::string to_string(const ParSet &value);

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

ParFilePtr createParFile(std::istream &contents);

} // namespace ParFile
