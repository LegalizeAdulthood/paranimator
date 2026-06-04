// SPDX-License-Identifier: GPL-3.0-only
//
#pragma once

#include <ParFile/ParFile.h>

#include <memory>
#include <string>
#include <string_view>
#include <vector>

namespace ParFile
{

struct Config;
class Interpolant;
struct NamedFileParSet;
struct ResolvedAnimation;
using InterpolantPtr = std::shared_ptr<Interpolant>;

class Interpolator
{
public:
    Interpolator() = default;
    Interpolator(const Config &config);
    Interpolator(const Config &config, std::string_view layer_id);
    Interpolator(const Interpolator &rhs) = default;
    Interpolator(Interpolator &&rhs) = default;
    Interpolator &operator=(const Interpolator &rhs) = default;
    Interpolator &operator=(Interpolator &&rhs) = default;

    const ParSet &source() const
    {
        return m_source;
    }

    ParSet operator()();

private:
    Interpolator(const ResolvedAnimation &animation);
    Interpolator(const ResolvedAnimation &animation, std::string_view layer_id);

    static std::vector<InterpolantPtr> load_interpolants(const ResolvedAnimation &animation);
    void load_color_map_interpolants(const Config &config);
    std::string m_frame_name;
    std::string m_layer_image_name;
    std::string m_layer_id;
    std::string m_video;
    ParSet m_source;
    std::vector<InterpolantPtr> m_interpolants;
    int m_frame{};
};

} // namespace ParFile
