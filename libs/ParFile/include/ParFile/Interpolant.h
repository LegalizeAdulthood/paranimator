// SPDX-License-Identifier: GPL-3.0-only
//
#pragma once

#include <ParFile/ResolvedAnimation.h>

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace ParFile
{

struct Parameter;

class Interpolant
{
public:
    virtual ~Interpolant() = default;

    virtual const std::string &name() const = 0;
    virtual const std::vector<int> &output_slots() const = 0;
    virtual bool has_value() const = 0;
    virtual std::string step() = 0;
};

using InterpolantPtr = std::shared_ptr<Interpolant>;

InterpolantPtr create_interpolant(const ResolvedTrack &track, int num_steps);

} // namespace ParFile
