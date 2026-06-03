// SPDX-License-Identifier: GPL-3.0-only
//
#include <ConfigSchema.h>

#include <ParFile/Config.h>
#include <ParFile/Interpolator.h>
#include <ParFile/JsonSchema.h>
#include <ParFile/OutputLayout.h>
#include <ParFile/ParFile.h>
#include <ParFile/Script.h>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>
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
    ParFile::OutputLayout output{config};
    output.create_directories();
    ParFile::Script script{config};
    std::ofstream out{output.par_file().string().c_str()};
    std::vector<std::ofstream> scripts;
    if (config.parallel == 1)
    {
        scripts.emplace_back(output.script_file().string().c_str());
    }
    else
    {
        for (int i = 1; i <= config.parallel; ++i)
        {
            scripts.emplace_back(output.script_file(i).string().c_str());
        }
    }
    auto current_script{scripts.begin()};
    for (int i = 0; i < config.num_frames; ++i)
    {
        const ParFile::ParSet frame{lerper()};
        if (i != 0)
        {
            out << '\n';
        }
        out << frame;
        *current_script << script.commands(frame.name);
        ++current_script;
        if (current_script == scripts.end())
        {
            current_script = scripts.begin();
        }
    }
}

std::string read_text(const std::filesystem::path &path);

ParFile::Config load_config(const std::filesystem::path &path)
{
    const std::string config_json{read_text(path)};
    const std::string schema_json{read_text(ParAnimator::config_schema_json)};
    if (!ParFile::validate_json_schema(schema_json, config_json))
    {
        throw std::runtime_error(
            "Config file does not match schema '" + std::string{ParAnimator::config_schema_json} + "'");
    }
    return ParFile::read_config(config_json);
}

std::string read_text(const std::filesystem::path &path)
{
    std::ifstream in{path};
    if (!in)
    {
        throw std::runtime_error("Unable to read file '" + path.string() + "'");
    }
    return {std::istreambuf_iterator<char>{in}, std::istreambuf_iterator<char>{}};
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
        interpolate(load_config(std::filesystem::path{std::string{json_file}}));

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
