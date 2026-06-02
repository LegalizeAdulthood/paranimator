// SPDX-License-Identifier: GPL-3.0-only
//
#pragma once

#include <ParFile/Config.h>

#include <memory>
#include <string>
#include <vector>

namespace ParFile
{

struct ParameterMetadata;
struct Parameter;

class Interpolant
{
public:
    virtual ~Interpolant() = default;

    virtual const std::string &name() const = 0;
    virtual std::string step() = 0;
};

using InterpolantPtr = std::shared_ptr<Interpolant>;

InterpolantPtr create_interpolant(
    const ParameterMetadata &metadata, const std::vector<KeyframeConfig> &keys, int num_steps);

} // namespace ParFile
