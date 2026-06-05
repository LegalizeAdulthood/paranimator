// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/Script.h>

#include <ParFile/Config.h>
#include <ParFile/ScriptDialect.h>

#include <fmt/format.h>
#include <fmt/printf.h>

#include <filesystem>

namespace ParFile
{

namespace
{

std::string script_path(std::filesystem::path path)
{
    path.make_preferred();
    return path.string();
}

std::string script_prologue()
{
    return std::string{ScriptDialect::PROLOGUE};
}

std::string script_epilogue()
{
    return std::string{ScriptDialect::EPILOGUE};
}

std::string make_directory_command(const std::string &directory)
{
    return fmt::format(fmt::runtime(std::string{ScriptDialect::MAKE_DIRECTORY_FORMAT}), directory) +
        std::string{ScriptDialect::ERROR_CHECK};
}

std::string move_command(const std::string &source, const std::string &destination)
{
    return fmt::format(fmt::runtime(std::string{ScriptDialect::MOVE_FORMAT}), source, destination) +
        std::string{ScriptDialect::ERROR_CHECK};
}

std::string error_check()
{
    return std::string{ScriptDialect::ERROR_CHECK};
}

std::string render_executable()
{
    return std::string{ScriptDialect::RENDER_EXECUTABLE};
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
    result += move_command(script_path(std::filesystem::path{"image"} / save_name), script_path(destination));
    return result;
}

std::string Script::render_command(const std::string &par_name, const std::string &save_name) const
{
    return render_executable() + " batch=yes overwrite=yes savename=" + save_name +
        " savedir=. librarydirs=. video=" + m_video + " @" + m_par + '/' + par_name + '\n' + error_check();
}

std::string Script::layer_file(const std::string &layer_id, int frame) const
{
    return fmt::sprintf(*m_layers, layer_id, frame + 1);
}

std::string Script::layer_directory() const
{
    if (!m_layers)
    {
        return {};
    }
    return script_path(std::filesystem::path{*m_layers}.parent_path());
}

} // namespace ParFile
