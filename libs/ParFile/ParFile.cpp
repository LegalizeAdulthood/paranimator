#include <ParFile/ParFile.h>

namespace ParFile
{

namespace
{

class StreamParFile : public ParFile
{
public:
    StreamParFile(std::istream &contents)
    {
    }
};

}

ParFilePtr create(std::istream &contents)
{
    return std::make_shared<StreamParFile>(contents);
}

}
