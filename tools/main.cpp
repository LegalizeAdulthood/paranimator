#include <ParFile/Interpolator.h>
#include <ParFile/ParFile.h>

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
              << program << " <config-json>\n";
    return 1;
}

void print(std::ostream &str, const ParFile::ParSet &par)
{
    str << par.name << " {\n";
    for (const ParFile::Parameter &param : par.params)
    {
        str << "    " << param.name;
        if (!param.value.empty())
        {
            str << '=' << param.value;
        }
        str << '\n';
    }
    str << "}\n";
}

boost::json::value read_json(std::istream &is, boost::system::error_code &ec)
{
    boost::json::stream_parser p;
    std::string line;
    while (std::getline(is, line))
    {
        p.write(line, ec);
        if (ec)
            return nullptr;
    }
    p.finish(ec);
    if (ec)
        return nullptr;
    return p.release();
}

ParFile::Config load_config(std::string_view path)
{
    std::ifstream in{std::string{path}};
    boost::system::error_code ec;
    return read_json(in, ec).as_object();
}

bool validate_named_parset(const Object &config, std::string_view name)
{
    if (!config.try_at(name) || !config.at(name).is_object())
    {
        return false;
    }
    const Object &named_params{config.at(name).as_object()};
    return named_params.try_at("file")         //
        && named_params.at("file").is_string() //
        && named_params.try_at("name")         //
        && named_params.at("name").is_string();
}

bool validate_config(const ParFile::Config &config)
{
    return config.try_at("begin") && config.at("begin").is_object()   //
        && validate_named_parset(config, "begin")                     //
        && validate_named_parset(config, "end")                       //
        && config.try_at("output") && config.at("output").is_string() //
        && config.try_at("script") && config.at("script").is_string() //
        && config.try_at("frame") && config.at("frame").is_string()   //
        && config.try_at("num_frames") && config.at("num_frames").is_number();
}

int main(const std::vector<std::string_view> &args)
{
    if (args.size() != 2)
    {
        return usage(args[0]);
    }
    const std::string_view json_file{args[1]};
    const ParFile::Config config{load_config(json_file)};
    if (!validate_config(config))
    {
        std::cerr << "Invalid JSON configuration " << json_file << '\n';
        return 2;
    }

    ParFile::Interpolator lerper{config};
    const std::string output{config.at("output").as_string()};
    std::ofstream out{output.c_str()};
    std::ofstream bat{config.at("script").as_string().c_str()};
    for (int i = 0; i < lerper.num_frames(); ++i)
    {
        ParFile::ParSet frame{lerper()};
        print(out, frame);
        out << '\n';
        bat << "id @" << output << '/' << frame.name << '\n';
    }

    return 0;
}

} // namespace

int main(int argc, char *argv[])
{
    return main(arguments(argc, argv));
}
