// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/ParFile.h>

#include <nlohmann/json.hpp>

#include <algorithm>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace
{

using Object = nlohmann::ordered_json;

struct ParReference
{
    std::string file;
    std::string name;
};

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
              << program << " @from.par/from_entry @to.par/to_entry num_frames center-mag|corners\n";
    return 1;
}

ParReference parse_par_reference(std::string_view text)
{
    if (text.empty() || text[0] != '@')
    {
        throw std::runtime_error("Par reference must start with @");
    }
    const std::size_t separator{text.rfind('/')};
    if (separator == std::string_view::npos || separator == 1U || separator + 1U == text.size())
    {
        throw std::runtime_error("Par reference must use @file/name syntax");
    }
    return {std::string{text.substr(1U, separator - 1U)}, std::string{text.substr(separator + 1U)}};
}

int parse_num_frames(std::string_view text)
{
    try
    {
        std::size_t length{};
        const int value{std::stoi(std::string{text}, &length)};
        if (length != text.size() || value < 2)
        {
            throw std::runtime_error("num_frames must be an integer greater than 1");
        }
        return value;
    }
    catch (const std::invalid_argument &)
    {
        throw std::runtime_error("num_frames must be an integer greater than 1");
    }
    catch (const std::out_of_range &)
    {
        throw std::runtime_error("num_frames is out of range");
    }
}

void validate_mode(std::string_view mode)
{
    if (mode != "center-mag" && mode != "corners")
    {
        throw std::runtime_error("Mode must be center-mag or corners");
    }
}

ParFile::ParSet load_par_set(const ParReference &reference)
{
    std::ifstream in{reference.file};
    if (!in)
    {
        throw std::runtime_error("Unable to read par file '" + reference.file + "'");
    }
    ParFile::ParFilePtr file{ParFile::create_par_file(in)};
    const auto it{std::find_if(
        file->cbegin(), file->cend(), [&](const ParFile::ParSet &params) { return params.name == reference.name; })};
    if (it == file->cend())
    {
        throw std::runtime_error("Unable to find par entry '" + reference.name + "' in '" + reference.file + "'");
    }
    return *it;
}

std::string parameter_value(const ParFile::ParSet &set, std::string_view parameter)
{
    const auto it{std::find_if(
        set.params.begin(), set.params.end(), [&](const ParFile::Parameter &item) { return item.name == parameter; })};
    if (it == set.params.end())
    {
        throw std::runtime_error("Par entry '" + set.name + "' is missing parameter '" + std::string{parameter} + "'");
    }
    return it->value;
}

Object keyframe(int frame, const std::string &value)
{
    Object result;
    result["frame"] = frame;
    result["value"] = value;
    return result;
}

Object starter_config(const ParReference &from_reference, int num_frames, std::string_view mode,
    const ParFile::ParSet &from_set, const ParFile::ParSet &to_set)
{
    Object track;
    track["parameter"] = std::string{mode};
    track["keys"] = Object::array(
        {keyframe(0, parameter_value(from_set, mode)), keyframe(num_frames - 1, parameter_value(to_set, mode))});

    Object source;
    source["file"] = from_reference.file;
    source["name"] = from_reference.name;

    Object output;
    output["directory"] = "output";
    output["par"] = "frames.par";
    output["entry"] = "frame-%04d";
    output["script"] = "frames.bat";

    Object result;
    result["parameter-catalogs"] = Object::array({"core-catalog.json"});
    result["source"] = source;
    result["output"] = output;
    result["video"] = "F6";
    result["num-frames"] = num_frames;
    result["tracks"] = Object::array({track});
    return result;
}

int main(const std::vector<std::string_view> &args)
{
    try
    {
        if (args.size() != 5U)
        {
            return usage(args[0]);
        }

        const ParReference from_reference{parse_par_reference(args[1])};
        const ParReference to_reference{parse_par_reference(args[2])};
        const int num_frames{parse_num_frames(args[3])};
        const std::string_view mode{args[4]};
        validate_mode(mode);

        const ParFile::ParSet from_set{load_par_set(from_reference)};
        const ParFile::ParSet to_set{load_par_set(to_reference)};
        std::cout << starter_config(from_reference, num_frames, mode, from_set, to_set).dump(2) << '\n';
        return 0;
    }
    catch (const std::exception &bang)
    {
        std::cerr << bang.what() << '\n';
        return 2;
    }
}

} // namespace

int main(int argc, char *argv[])
{
    return main(arguments(argc, argv));
}
