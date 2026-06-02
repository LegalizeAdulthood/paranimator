// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/OutputLayout.h>

#include <ParFile/Config.h>

#include <string>

namespace ParFile
{

OutputLayout::OutputLayout(const Config &config) :
    m_directory(config.output().directory),
    m_par(config.output().par),
    m_script(config.output().script)
{
}

std::filesystem::path OutputLayout::script_file(int index) const
{
    return m_directory /
        (m_script.stem().string() + '-' + std::to_string(index) + m_script.extension().string());
}

void OutputLayout::create_directories() const
{
    std::filesystem::create_directories(par_directory());
    std::filesystem::create_directories(map_directory());
}

} // namespace ParFile
