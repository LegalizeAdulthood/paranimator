// SPDX-License-Identifier: GPL-3.0-only
//
#pragma once

#include <optional>
#include <string>

namespace ParFile
{

struct Config;

class Script
{
public:
    Script(const Config &config);

    std::string prologue() const;
    std::string epilogue() const;
    std::string commands(const std::string &par_name) const;
    std::string layer_commands(const std::string &par_name, const std::string &layer_id, int frame) const;

private:
    std::string render_command(const std::string &par_name, const std::string &save_name) const;
    std::string layer_file(const std::string &layer_id, int frame) const;

    std::string m_par;
    std::optional<std::string> m_layers;
    std::string m_video;
};

} // namespace ParFile
