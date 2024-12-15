#pragma once

#include <iostream>
#include <memory>

namespace ParFile
{

class ParFile
{
public:
    virtual ~ParFile() = default;
};

using ParFilePtr = std::shared_ptr<ParFile>;

ParFilePtr create(std::istream &contents);

}
