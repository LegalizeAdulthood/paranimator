// SPDX-License-Identifier: GPL-3.0-only
//
#pragma once

#include <filesystem>

namespace ParFile
{

struct Config;

class OutputLayout
{
public:
    explicit OutputLayout(const Config &config);

    const std::filesystem::path &directory() const
    {
        return m_directory;
    }
    std::filesystem::path par_directory() const
    {
        return m_directory / "par";
    }
    std::filesystem::path map_directory() const
    {
        return m_directory / "map";
    }
    std::filesystem::path par_file() const
    {
        return par_directory() / m_par;
    }
    std::filesystem::path script_file() const
    {
        return m_directory / m_script;
    }
    std::filesystem::path script_file(int index) const;
    std::filesystem::path compose_script_file() const;

    void create_directories() const;

private:
    std::filesystem::path m_directory;
    std::filesystem::path m_par;
    std::filesystem::path m_script;
    std::filesystem::path m_frames;
    std::filesystem::path m_layers;
    std::filesystem::path m_compose_script;
};

} // namespace ParFile
