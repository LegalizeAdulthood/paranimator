// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/Config.h>
#include <ParFile/Interpolator.h>
#include <ParFile/Json.h>
#include <ParFile/ParFile.h>
#include <ParFile/Script.h>

#include <boost/json.hpp>

#include <filesystem>
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

void interpolate(const ParFile::Config &config)
{
    ParFile::Interpolator lerper{config};
    ParFile::Script script{config};
    std::ofstream out{config.output().c_str()};
    std::vector<std::ofstream> scripts;
    if(config.parallel() == 1)
    {
        scripts.emplace_back(config.script().c_str());
    }
    else
    {
        std::filesystem::path name{config.script()};
        std::filesystem::path filename{name};
        for (int i = 1; i <= config.parallel(); ++i)
        {
            filename.replace_filename(name.stem().string() + '-' + std::to_string(i) + name.extension().string());
            scripts.emplace_back(filename.string().c_str());
        }
    }
    auto current_script{scripts.begin()};
    for (int i = 0; i < config.num_frames(); ++i)
    {
        const ParFile::ParSet frame{lerper()};
        out << frame << '\n';
        *current_script << script.commands(frame.name);
        ++current_script;
        if (current_script == scripts.end())
        {
            current_script = scripts.begin();
        }
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
        interpolate(ParFile::read_json(json_file).as_object());

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
