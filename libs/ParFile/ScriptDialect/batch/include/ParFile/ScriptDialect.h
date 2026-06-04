// SPDX-License-Identifier: GPL-3.0-only
//
#pragma once

#include <string_view>

namespace ParFile::ScriptDialect
{

inline constexpr std::string_view PROLOGUE =
    "@echo off\n"
    "pushd \"%~dp0\"\n"
    "if errorlevel 1 exit /b 1\n";
inline constexpr std::string_view EPILOGUE = "popd\n";
inline constexpr std::string_view ERROR_CHECK = "if errorlevel 1 exit /b 1\n";
inline constexpr std::string_view MAKE_DIRECTORY_FORMAT = "if not exist \"%1%\" mkdir \"%1%\"\n";
inline constexpr std::string_view MOVE_FORMAT = "move /y \"%1%\" \"%2%\"\n";
inline constexpr std::string_view RENDER_EXECUTABLE = "start/wait id";
inline constexpr std::string_view OPEN_GROUP = "^(";
inline constexpr std::string_view CLOSE_GROUP = "^)";

} // namespace ParFile::ScriptDialect
