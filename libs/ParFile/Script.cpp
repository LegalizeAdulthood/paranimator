// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/Script.h>

#include <ParFile/Config.h>

namespace ParFile
{

Script::Script(const Config &config) :
    m_output(config.output())
{
}

std::string Script::commands(const std::string &par_name) const
{
    return "start/wait id @" + m_output + '/' + par_name + '\n' //
        + "if errorlevel 1 exit /b 1\n";
}

} // namespace ParFile
