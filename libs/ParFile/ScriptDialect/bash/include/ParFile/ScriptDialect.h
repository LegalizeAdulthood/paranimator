// SPDX-License-Identifier: GPL-3.0-only
//
#pragma once

#include <string_view>

namespace ParFile::ScriptDialect
{

inline constexpr std::string_view PROLOGUE =
    "#!/usr/bin/env bash\n"
    "set -e\n"
    "pushd \"$(dirname \"$0\")\" >/dev/null\n";
inline constexpr std::string_view EPILOGUE = "popd >/dev/null\n";
inline constexpr std::string_view ERROR_CHECK = "";
inline constexpr std::string_view MAKE_DIRECTORY_FORMAT = "mkdir -p \"%1%\"\n";
inline constexpr std::string_view MOVE_FORMAT = "mv -f \"%1%\" \"%2%\"\n";
inline constexpr std::string_view RENDER_EXECUTABLE = "id";
inline constexpr std::string_view OPEN_GROUP = R"text(\()text";
inline constexpr std::string_view CLOSE_GROUP = R"text(\))text";

} // namespace ParFile::ScriptDialect
