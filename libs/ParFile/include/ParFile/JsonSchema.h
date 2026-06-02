// SPDX-License-Identifier: GPL-3.0-only
//
#pragma once

#include <filesystem>

namespace ParFile
{

bool validate_json_schema(
    const std::filesystem::path &schema_path,
    const std::filesystem::path &instance_path);

} // namespace ParFile
