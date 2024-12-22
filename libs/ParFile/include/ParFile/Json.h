// SPDX-License-Identifier: GPL-3.0-only
//
#pragma once

#include <boost/json.hpp>

#include <filesystem>
#include <iosfwd>

namespace ParFile
{

boost::json::value read_json(std::istream &is);
boost::json::value read_json(const std::filesystem::path &path);

} // namespace ParFile
