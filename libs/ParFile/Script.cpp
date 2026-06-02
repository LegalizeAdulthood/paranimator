// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/Script.h>

#include <ParFile/Config.h>

namespace ParFile
{

Script::Script(const Config &config) :
    m_directory(config.output().directory),
    m_par(config.output().par)
{
}

std::string Script::commands(const std::string &par_name) const
{
    return "start/wait id batch=yes librarydirs=" + m_directory + " @" + m_par + '/' + par_name + '\n' //
        + "if errorlevel 1 exit /b 1\n";
}

} // namespace ParFile
