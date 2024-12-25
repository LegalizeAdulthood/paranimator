// SPDX-License-Identifier: GPL-3.0-only
//
#pragma once

#include <string>

namespace ParFile
{

class Config;

class Script
{
public:
    Script(const Config &config);

    std::string commands(const std::string &par_name) const;

private:
    std::string m_output;
};

} // namespace ParFile
