// SPDX-License-Identifier: GPL-3.0-only
//
#include <ConfigSchema.h>

#include <ParFile/ComposeScript.h>
#include <ParFile/Config.h>
#include <ParFile/Interpolator.h>
#include <ParFile/JsonSchema.h>
#include <ParFile/NumberTrack.h>
#include <ParFile/OutputLayout.h>
#include <ParFile/ParFile.h>
#include <ParFile/Script.h>

#include <cstddef>
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

ParFile::Config layer_config(const ParFile::Config &config, const ParFile::LayerConfig &layer)
{
    ParFile::Config result{config};
    result.source = layer.source;
    result.tracks = layer.tracks;
    result.layers.clear();
    return result;
}

std::vector<ParFile::Interpolator> layer_interpolators(const ParFile::Config &config)
{
    std::vector<ParFile::Interpolator> result;
    result.reserve(config.layers.size());
    const bool include_layer_id{config.layers.size() > 1U || config.output.layers};
    for (const ParFile::LayerConfig &layer : config.layers)
    {
        if (include_layer_id)
        {
            result.emplace_back(layer_config(config, layer), layer.id);
        }
        else
        {
            result.emplace_back(layer_config(config, layer));
        }
    }
    return result;
}

bool should_render_layer(const ParFile::LayerConfig &layer, int frame)
{
    if (!layer.opacity)
    {
        return true;
    }
    return layer.write_when_hidden || ParFile::number_track_value_at_frame(*layer.opacity, frame) > 0.0;
}

void interpolate_layers(const ParFile::Config &config)
{
    std::vector<ParFile::Interpolator> lerpers{layer_interpolators(config)};
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
    bool first_entry{true};
    for (int i = 0; i < config.num_frames; ++i)
    {
        for (std::size_t layer = 0; layer < lerpers.size(); ++layer)
        {
            const ParFile::ParSet frame{lerpers[layer]()};
            if (should_render_layer(config.layers[layer], i))
            {
                if (!first_entry)
                {
                    out << '\n';
                }
                first_entry = false;
                out << frame;
                *current_script << script.commands(frame.name);
                ++current_script;
                if (current_script == scripts.end())
                {
                    current_script = scripts.begin();
                }
            }
        }
    }
}

void write_compose_script(const ParFile::Config &config)
{
    if (!config.output.compose_script)
    {
        return;
    }
    const ParFile::OutputLayout output{config};
    const ParFile::ComposeScript compose{config};
    std::ofstream out{output.compose_script_file().string().c_str()};
    for (int i = 0; i < config.num_frames; ++i)
    {
        out << compose.commands(i);
    }
}

void render(const ParFile::Config &config)
{
    if (!config.layers.empty())
    {
        interpolate_layers(config);
        write_compose_script(config);
        return;
    }
    interpolate(config);
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
        render(load_config(std::filesystem::path{std::string{json_file}}));

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
