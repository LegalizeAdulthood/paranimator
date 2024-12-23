// SPDX-License-Identifier: GPL-3.0-only
//
#pragma once

#include <memory>
#include <string>

namespace ParFile
{

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
    const std::string &name, const std::string &from, const std::string &to, int num_steps);

} // namespace ParFile
