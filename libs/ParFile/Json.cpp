#include <ParFile/Json.h>

#include <fstream>
#include <istream>
#include <string>

namespace ParFile
{

boost::json::value read_json(std::istream &is)
{
    boost::system::error_code ec;
    boost::json::stream_parser p;
    std::string line;
    while (std::getline(is, line))
    {
        p.write(line, ec);
        if (ec)
        {
            return nullptr;
        }
    }
    p.finish(ec);
    if (ec)
    {
        return nullptr;
    }
    return p.release();
}

boost::json::value read_json(const std::filesystem::path &path)
{
    std::ifstream in{path};
    return read_json(in);
}

} // namespace ParFile
