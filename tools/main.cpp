#include <ParFile/ParFile.h>

#include <tweeny/tweeny.h>

#include <boost/json.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace
{

using Object = boost::json::object;
using Config = boost::json::object;

std::vector<std::string_view> arguments(int argc, char *argv[])
{
    std::vector<std::string_view> result;
    result.reserve(argc);
    for (int i = 0; i < argc; ++i)
    {
        result.emplace_back(argv[i]);
    }
    return result;
}

int usage(std::string_view program)
{
    std::cerr << "Usage:\n" //
              << program << " <config-json> <num-frames>\n";
    return 1;
}

ParFile::ParFilePtr read_par_file(std::string_view file)
{
    std::ifstream in{std::string{file}};
    return ParFile::create(in);
}

struct NamedParam
{
    NamedParam(const Object &config);

    ParFile::ParFilePtr params;
    std::string entry_name;
};

NamedParam::NamedParam(const Object &config) :
    params(read_par_file(config.at("file").as_string())),
    entry_name(config.at("name").as_string())
{
}

class Interpolator
{
public:
    Interpolator(const Config &config, int num_frames);

    ParFile::ParSet operator()();

private:
    const Config &m_config;
    int m_num_frames;

    NamedParam m_begin;
    NamedParam m_end;
    int m_frame{};
};

Interpolator::Interpolator(const Config &config, int num_frames) :
    m_config(config),
    m_num_frames(num_frames),
    m_begin(config.at("begin").as_object()),
    m_end(config.at("end").as_object())
{
}

ParFile::ParSet Interpolator::operator()()
{
    ParFile::ParSet par_set{*(m_begin.params->begin())};
    par_set.name = "Frame_" + std::to_string(m_frame + 1);
    ++m_frame;
    return par_set;
}

void print(const ParFile::ParSet &par)
{
    std::cout << par.name << " {\n";
    for (const ParFile::Parameter &param : par.params)
    {
        std::cout << "    " << param.name;
        if (!param.value.empty())
        {
            std::cout << '=' << param.value;
        }
        std::cout << '\n';
    }
    std::cout << "}\n";
}

boost::json::value read_json( std::istream& is, boost::system::error_code& ec )
{
    boost::json::stream_parser p;
    std::string line;
    while( std::getline( is, line ) )
    {
        p.write( line, ec );
        if( ec )
            return nullptr;
    }
    p.finish( ec );
    if( ec )
        return nullptr;
    return p.release();
}

Config load_config(std::string_view path)
{
    std::ifstream in{std::string{path}};
    boost::system::error_code ec;
    return read_json(in, ec).as_object();
}

bool validate_config(const Config &config)
{
    return true;
}

int main(const std::vector<std::string_view> &args)
{
    if (args.size() != 3)
    {
        return usage(args[0]);
    }
    const std::string_view json_file{args[1]};
    const Config config{load_config(json_file)};
    if (!validate_config(config))
    {
        std::cerr << "Invalid JSON configuration " << json_file << '\n';
        return 2;
    }
    const int num_frames{std::stoi(std::string{args[2]})};
    if (num_frames < 1)
    {
        std::cerr << num_frames << " must be positive.\n";
        return 3;
    }
    
    Interpolator lerper{config, num_frames};
    for (int i = 0; i < num_frames; ++i)
    {
        ParFile::ParSet frame{lerper()};
        print(frame);
    }
    
    return 0;
}

}

int main(int argc, char *argv[])
{
    return main(arguments(argc, argv));    
}
