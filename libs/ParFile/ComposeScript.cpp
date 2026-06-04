// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/ComposeScript.h>

#include <ParFile/Config.h>
#include <ParFile/NumberTrack.h>
#include <ParFile/ScriptDialect.h>

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

std::string output_path(const std::string &path)
{
    return path;
}

std::string opacity_text(double opacity)
{
    std::ostringstream out;
    out << std::setprecision(12) << opacity / 100.0;
    return out.str();
}

std::string script_prologue()
{
    return std::string{ScriptDialect::PROLOGUE};
}

std::string script_epilogue()
{
    return std::string{ScriptDialect::EPILOGUE};
}

std::string error_check()
{
    return std::string{ScriptDialect::ERROR_CHECK};
}

std::string open_group()
{
    return std::string{ScriptDialect::OPEN_GROUP};
}

std::string close_group()
{
    return std::string{ScriptDialect::CLOSE_GROUP};
}

std::string_view imagemagick_compose_operator(ComposeOperator op)
{
    switch (op)
    {
    case ComposeOperator::CLEAR:
        return "Clear";
    case ComposeOperator::COPY:
        return "Src";
    case ComposeOperator::DESTINATION:
        return "Dst";
    case ComposeOperator::SOURCE_OVER:
        return "Over";
    case ComposeOperator::DESTINATION_OVER:
        return "Dst_Over";
    case ComposeOperator::SOURCE_IN:
        return "Src_In";
    case ComposeOperator::DESTINATION_IN:
        return "Dst_In";
    case ComposeOperator::SOURCE_OUT:
        return "Src_Out";
    case ComposeOperator::DESTINATION_OUT:
        return "Dst_Out";
    case ComposeOperator::SOURCE_ATOP:
        return "Src_Atop";
    case ComposeOperator::DESTINATION_ATOP:
        return "Dst_Atop";
    case ComposeOperator::XOR:
        return "Xor";
    case ComposeOperator::ADD:
        return "Plus";
    case ComposeOperator::SUBTRACT:
        return "Minus_Src";
    case ComposeOperator::MULTIPLY:
        return "Multiply";
    case ComposeOperator::DIVIDE:
        return "Divide_Src";
    case ComposeOperator::MIN:
        return "Min";
    case ComposeOperator::MAX:
        return "Max";
    case ComposeOperator::DIFFERENCE:
        return "Difference";
    case ComposeOperator::AVERAGE:
        return "Average";
    case ComposeOperator::SCREEN:
        return "Screen";
    case ComposeOperator::OVERLAY:
        return "Overlay";
    }
    return "Over";
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

std::string ComposeScript::prologue() const
{
    return script_prologue();
}

std::string ComposeScript::epilogue() const
{
    return script_epilogue();
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
                result += " -compose " + std::string{imagemagick_compose_operator(layer.compose)} + " -composite";
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
    result += error_check();
    return result;
}

std::string ComposeScript::frame_file(int frame) const
{
    return output_path((boost::format(*m_config.output.frames) % (frame + 1)).str());
}

std::string ComposeScript::layer_file(const LayerConfig &layer, int frame) const
{
    return output_path((boost::format(*m_config.output.layers) % layer.id % (frame + 1)).str());
}

std::string ComposeScript::layer_image(const LayerConfig &layer, int frame) const
{
    return open_group() + " " + quote(layer_file(layer, frame)) + " -alpha set -channel A -evaluate multiply " +
        opacity_text(opacity(layer, frame)) + " +channel " + close_group();
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
