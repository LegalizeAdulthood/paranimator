// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/OutputLayout.h>

#include <ParFile/Config.h>

#include <string>

namespace ParFile
{

OutputLayout::OutputLayout(const Config &config) :
    m_directory(config.output.directory),
    m_par(config.output.par),
    m_script(config.output.script),
    m_frames(config.output.frames.value_or(std::string{})),
    m_layers(config.output.layers.value_or(std::string{})),
    m_compose_script(config.output.compose_script.value_or(std::string{}))
{
}

std::filesystem::path OutputLayout::script_file(int index) const
{
    return m_directory / (m_script.stem().string() + '-' + std::to_string(index) + m_script.extension().string());
}

std::filesystem::path OutputLayout::compose_script_file() const
{
    return m_directory / m_compose_script;
}

static void create_parent_directory(const std::filesystem::path &path)
{
    const std::filesystem::path parent{path.parent_path()};
    if (!parent.empty())
    {
        std::filesystem::create_directories(parent);
    }
}

void OutputLayout::create_directories() const
{
    std::filesystem::create_directories(par_directory());
    std::filesystem::create_directories(map_directory());
    create_parent_directory(m_directory / m_frames);
    create_parent_directory(m_directory / m_layers);
}

} // namespace ParFile
