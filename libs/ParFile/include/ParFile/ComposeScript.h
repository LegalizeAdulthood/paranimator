// SPDX-License-Identifier: GPL-3.0-only
//
#pragma once

#include <string>

namespace ParFile
{

struct Config;
struct LayerConfig;

class ComposeScript
{
public:
    explicit ComposeScript(const Config &config);

    std::string commands(int frame) const;

private:
    std::string frame_file(int frame) const;
    std::string layer_file(const LayerConfig &layer, int frame) const;
    std::string layer_image(const LayerConfig &layer, int frame) const;
    double opacity(const LayerConfig &layer, int frame) const;

    const Config &m_config;
};

} // namespace ParFile
