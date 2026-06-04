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
#ifdef _WIN32
    path.make_preferred();
#else
    path = path.lexically_normal();
#endif
    return path.string();
}

std::string script_prologue()
{
#ifdef _WIN32
    return "@echo off\n"
           "pushd \"%~dp0\"\n"
           "if errorlevel 1 exit /b 1\n";
#else
    return "#!/usr/bin/env bash\n"
           "set -e\n"
           "pushd \"$(dirname \"$0\")\" >/dev/null\n";
#endif
}

std::string script_epilogue()
{
#ifdef _WIN32
    return "popd\n";
#else
    return "popd >/dev/null\n";
#endif
}

std::string make_directory_command(const std::string &directory)
{
#ifdef _WIN32
    return "if not exist \"" + directory + "\" mkdir \"" + directory + "\"\n"
        "if errorlevel 1 exit /b 1\n";
#else
    return "mkdir -p \"" + directory + "\"\n";
#endif
}

std::string move_command(const std::string &source, const std::string &destination)
{
#ifdef _WIN32
    return "move /y \"" + source + "\" \"" + destination + "\"\n"
        "if errorlevel 1 exit /b 1\n";
#else
    return "mv -f \"" + source + "\" \"" + destination + "\"\n";
#endif
}

std::string error_check()
{
#ifdef _WIN32
    return "if errorlevel 1 exit /b 1\n";
#else
    return {};
#endif
}

std::string render_executable()
{
#ifdef _WIN32
    return "start/wait id";
#else
    return "id";
#endif
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
    std::string result{script_prologue()};
    const std::string directory{layer_directory()};
    if (!directory.empty())
    {
        result += make_directory_command(directory);
    }
    return result;
}

std::string Script::epilogue() const
{
    return script_epilogue();
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
    result += move_command(batch_path(std::filesystem::path{"image"} / save_name), batch_path(destination));
    return result;
}

std::string Script::render_command(const std::string &par_name, const std::string &save_name) const
{
    return render_executable() + " batch=yes overwrite=yes savename=" + save_name +
        " savedir=. librarydirs=. video=" + m_video + " @" + m_par + '/' + par_name + '\n' + error_check();
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
