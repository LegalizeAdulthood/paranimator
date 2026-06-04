// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/Script.h>

#include <ParFile/Config.h>

#include <boost/format.hpp>

#include <filesystem>

namespace ParFile
{

Script::Script(const Config &config) :
    m_par(config.output.par),
    m_layers(config.output.layers),
    m_video(config.video)
{
}

std::string Script::prologue() const
{
    return "@echo off\n"
           "pushd \"%~dp0\"\n"
           "if errorlevel 1 exit /b 1\n";
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
    const std::filesystem::path layer_directory{std::filesystem::path{destination}.parent_path()};
    std::string result;
    if (!layer_directory.empty())
    {
        result += "if not exist \"" + layer_directory.generic_string() + "\" mkdir \"" +
            layer_directory.generic_string() + "\"\n";
    }
    result += render_command(par_name, save_name);
    result += "move /y \"image/" + save_name + "\" \"" + destination + "\"\n";
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

} // namespace ParFile
