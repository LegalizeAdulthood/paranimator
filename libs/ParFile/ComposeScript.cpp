// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/ComposeScript.h>

#include <ParFile/Config.h>
#include <ParFile/NumberTrack.h>

#include <boost/format.hpp>

#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace ParFile
{

namespace
{

bool should_render_layer(const LayerConfig &layer, int frame)
{
    if (!layer.opacity)
    {
        return true;
    }
    return layer.write_when_hidden || number_track_value_at_frame(*layer.opacity, frame) > 0.0;
}

std::string quote(std::string_view text)
{
    return '"' + std::string{text} + '"';
}

std::string output_path(const Config &config, const std::string &path)
{
    return config.output.directory + '/' + path;
}

std::string opacity_text(double opacity)
{
    std::ostringstream out;
    out << std::setprecision(12) << opacity / 100.0;
    return out.str();
}

} // namespace

ComposeScript::ComposeScript(const Config &config) :
    m_config(config)
{
    if (!m_config.output.frames || !m_config.output.layers)
    {
        throw std::runtime_error("Compose script requires output frames and layers");
    }
}

std::string ComposeScript::commands(int frame) const
{
    std::string result{"magick"};
    bool first_layer{true};
    for (const LayerConfig &layer : m_config.layers)
    {
        if (should_render_layer(layer, frame))
        {
            result += ' ' + layer_image(layer, frame);
            if (first_layer)
            {
                first_layer = false;
            }
            else
            {
                result += " -compose over -composite";
            }
        }
    }
    if (first_layer)
    {
        result += " xc:none";
    }
    if (m_config.output.background)
    {
        result += " -background " + quote(*m_config.output.background) + " -alpha remove -alpha off";
    }
    result += ' ' + quote(frame_file(frame)) + '\n';
    result += "if errorlevel 1 exit /b 1\n";
    return result;
}

std::string ComposeScript::frame_file(int frame) const
{
    return output_path(m_config, (boost::format(*m_config.output.frames) % (frame + 1)).str());
}

std::string ComposeScript::layer_file(const LayerConfig &layer, int frame) const
{
    return output_path(m_config, (boost::format(*m_config.output.layers) % layer.id % (frame + 1)).str());
}

std::string ComposeScript::layer_image(const LayerConfig &layer, int frame) const
{
    return "^( " + quote(layer_file(layer, frame)) + " -alpha set -channel A -evaluate multiply " +
        opacity_text(opacity(layer, frame)) + " +channel ^)";
}

double ComposeScript::opacity(const LayerConfig &layer, int frame) const
{
    if (!layer.opacity)
    {
        return 100.0;
    }
    return number_track_value_at_frame(*layer.opacity, frame);
}

} // namespace ParFile
