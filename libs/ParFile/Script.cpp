// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/Script.h>

#include <ParFile/Config.h>

#include <boost/format.hpp>

#include <filesystem>

namespace ParFile
{

namespace
{

std::string batch_path(std::filesystem::path path)
{
    path.make_preferred();
    return path.string();
}

} // namespace

Script::Script(const Config &config) :
    m_par(config.output.par),
    m_layers(config.output.layers),
    m_video(config.video)
{
}

std::string Script::prologue() const
{
    std::string result{"@echo off\n"
                       "pushd \"%~dp0\"\n"
                       "if errorlevel 1 exit /b 1\n"};
    const std::string directory{layer_directory()};
    if (!directory.empty())
    {
        result += "if not exist \"" + directory + "\" mkdir \"" + directory + "\"\n";
        result += "if errorlevel 1 exit /b 1\n";
    }
    return result;
}

std::string Script::epilogue() const
{
    return "popd\n";
}

std::string Script::commands(const std::string &par_name) const
{
    return render_command(par_name, par_name + ".gif");
}

std::string Script::layer_commands(const std::string &par_name, const std::string &layer_id, int frame) const
{
    if (!m_layers)
    {
        return commands(par_name);
    }
    const std::string destination{layer_file(layer_id, frame)};
    const std::string save_name{std::filesystem::path{destination}.filename().string()};
    std::string result;
    result += render_command(par_name, save_name);
    result += "move /y \"" + batch_path(std::filesystem::path{"image"} / save_name) + "\" \"" +
        batch_path(destination) + "\"\n";
    result += "if errorlevel 1 exit /b 1\n";
    return result;
}

std::string Script::render_command(const std::string &par_name, const std::string &save_name) const
{
    return "start/wait id batch=yes overwrite=yes savename=" + save_name + " savedir=. librarydirs=. video=" + m_video +
        " @" + m_par + '/' + par_name + '\n' //
        + "if errorlevel 1 exit /b 1\n";
}

std::string Script::layer_file(const std::string &layer_id, int frame) const
{
    return (boost::format(*m_layers) % layer_id % (frame + 1)).str();
}

std::string Script::layer_directory() const
{
    if (!m_layers)
    {
        return {};
    }
    return batch_path(std::filesystem::path{*m_layers}.parent_path());
}

} // namespace ParFile
