#include <ParFile/Config.h>
#include <ParFile/Interpolator.h>
#include <ParFile/ParFile.h>

#include <boost/json.hpp>

#include <fstream>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace
{

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

void interpolate(const ParFile::Config &config)
{
    ParFile::Interpolator lerper{config};
    std::ofstream out{config.output().c_str()};
    std::ofstream bat{config.script().c_str()};
    for (int i = 0; i < config.num_frames(); ++i)
    {
        const ParFile::ParSet frame{lerper()};
        print(out, frame);
        out << '\n';
        bat << "start/wait id @" << config.output() << '/' << frame.name << '\n';
    }
}

int main(const std::vector<std::string_view> &args)
{
    try
    {
        if (args.size() != 2)
        {
            return usage(args[0]);
        }
        const std::string_view json_file{args[1]};
        interpolate(load_config(json_file));

        return 0;
    }
    catch (const std::exception &bang)
    {
        {
            std::cerr << bang.what() << '\n';
            return 2;
        }
    }
}

} // namespace

int main(int argc, char *argv[])
{
    return main(arguments(argc, argv));
}
