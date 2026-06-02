// SPDX-License-Identifier: GPL-3.0-only
//
#pragma once

#include <string_view>

namespace ParFile
{

bool validate_json_schema(
    std::string_view schema_json,
    std::string_view instance_json);

} // namespace ParFile
