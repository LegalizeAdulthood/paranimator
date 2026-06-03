// SPDX-License-Identifier: GPL-3.0-only
//
#include <ParFile/ColorSpec.h>

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
#include <iterator>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace ParFile
{

namespace
{

struct NamedColor
{
    std::string_view name;
    RgbColor color;
};

// Derived from the W3C CSS Color Module Level 4 named-colors table.
// Source: https://www.w3.org/TR/css-color-4/#named-colors
// Retrieved: 2026-06-03
constexpr NamedColor CSS_NAMED_COLORS[]{
    {"aliceblue", {240, 248, 255}},
    {"antiquewhite", {250, 235, 215}},
    {"aqua", {0, 255, 255}},
    {"aquamarine", {127, 255, 212}},
    {"azure", {240, 255, 255}},
    {"beige", {245, 245, 220}},
    {"bisque", {255, 228, 196}},
    {"black", {0, 0, 0}},
    {"blanchedalmond", {255, 235, 205}},
    {"blue", {0, 0, 255}},
    {"blueviolet", {138, 43, 226}},
    {"brown", {165, 42, 42}},
    {"burlywood", {222, 184, 135}},
    {"cadetblue", {95, 158, 160}},
    {"chartreuse", {127, 255, 0}},
    {"chocolate", {210, 105, 30}},
    {"coral", {255, 127, 80}},
    {"cornflowerblue", {100, 149, 237}},
    {"cornsilk", {255, 248, 220}},
    {"crimson", {220, 20, 60}},
    {"cyan", {0, 255, 255}},
    {"darkblue", {0, 0, 139}},
    {"darkcyan", {0, 139, 139}},
    {"darkgoldenrod", {184, 134, 11}},
    {"darkgray", {169, 169, 169}},
    {"darkgreen", {0, 100, 0}},
    {"darkgrey", {169, 169, 169}},
    {"darkkhaki", {189, 183, 107}},
    {"darkmagenta", {139, 0, 139}},
    {"darkolivegreen", {85, 107, 47}},
    {"darkorange", {255, 140, 0}},
    {"darkorchid", {153, 50, 204}},
    {"darkred", {139, 0, 0}},
    {"darksalmon", {233, 150, 122}},
    {"darkseagreen", {143, 188, 143}},
    {"darkslateblue", {72, 61, 139}},
    {"darkslategray", {47, 79, 79}},
    {"darkslategrey", {47, 79, 79}},
    {"darkturquoise", {0, 206, 209}},
    {"darkviolet", {148, 0, 211}},
    {"deeppink", {255, 20, 147}},
    {"deepskyblue", {0, 191, 255}},
    {"dimgray", {105, 105, 105}},
    {"dimgrey", {105, 105, 105}},
    {"dodgerblue", {30, 144, 255}},
    {"firebrick", {178, 34, 34}},
    {"floralwhite", {255, 250, 240}},
    {"forestgreen", {34, 139, 34}},
    {"fuchsia", {255, 0, 255}},
    {"gainsboro", {220, 220, 220}},
    {"ghostwhite", {248, 248, 255}},
    {"gold", {255, 215, 0}},
    {"goldenrod", {218, 165, 32}},
    {"gray", {128, 128, 128}},
    {"green", {0, 128, 0}},
    {"greenyellow", {173, 255, 47}},
    {"grey", {128, 128, 128}},
    {"honeydew", {240, 255, 240}},
    {"hotpink", {255, 105, 180}},
    {"indianred", {205, 92, 92}},
    {"indigo", {75, 0, 130}},
    {"ivory", {255, 255, 240}},
    {"khaki", {240, 230, 140}},
    {"lavender", {230, 230, 250}},
    {"lavenderblush", {255, 240, 245}},
    {"lawngreen", {124, 252, 0}},
    {"lemonchiffon", {255, 250, 205}},
    {"lightblue", {173, 216, 230}},
    {"lightcoral", {240, 128, 128}},
    {"lightcyan", {224, 255, 255}},
    {"lightgoldenrodyellow", {250, 250, 210}},
    {"lightgray", {211, 211, 211}},
    {"lightgreen", {144, 238, 144}},
    {"lightgrey", {211, 211, 211}},
    {"lightpink", {255, 182, 193}},
    {"lightsalmon", {255, 160, 122}},
    {"lightseagreen", {32, 178, 170}},
    {"lightskyblue", {135, 206, 250}},
    {"lightslategray", {119, 136, 153}},
    {"lightslategrey", {119, 136, 153}},
    {"lightsteelblue", {176, 196, 222}},
    {"lightyellow", {255, 255, 224}},
    {"lime", {0, 255, 0}},
    {"limegreen", {50, 205, 50}},
    {"linen", {250, 240, 230}},
    {"magenta", {255, 0, 255}},
    {"maroon", {128, 0, 0}},
    {"mediumaquamarine", {102, 205, 170}},
    {"mediumblue", {0, 0, 205}},
    {"mediumorchid", {186, 85, 211}},
    {"mediumpurple", {147, 112, 219}},
    {"mediumseagreen", {60, 179, 113}},
    {"mediumslateblue", {123, 104, 238}},
    {"mediumspringgreen", {0, 250, 154}},
    {"mediumturquoise", {72, 209, 204}},
    {"mediumvioletred", {199, 21, 133}},
    {"midnightblue", {25, 25, 112}},
    {"mintcream", {245, 255, 250}},
    {"mistyrose", {255, 228, 225}},
    {"moccasin", {255, 228, 181}},
    {"navajowhite", {255, 222, 173}},
    {"navy", {0, 0, 128}},
    {"oldlace", {253, 245, 230}},
    {"olive", {128, 128, 0}},
    {"olivedrab", {107, 142, 35}},
    {"orange", {255, 165, 0}},
    {"orangered", {255, 69, 0}},
    {"orchid", {218, 112, 214}},
    {"palegoldenrod", {238, 232, 170}},
    {"palegreen", {152, 251, 152}},
    {"paleturquoise", {175, 238, 238}},
    {"palevioletred", {219, 112, 147}},
    {"papayawhip", {255, 239, 213}},
    {"peachpuff", {255, 218, 185}},
    {"peru", {205, 133, 63}},
    {"pink", {255, 192, 203}},
    {"plum", {221, 160, 221}},
    {"powderblue", {176, 224, 230}},
    {"purple", {128, 0, 128}},
    {"rebeccapurple", {102, 51, 153}},
    {"red", {255, 0, 0}},
    {"rosybrown", {188, 143, 143}},
    {"royalblue", {65, 105, 225}},
    {"saddlebrown", {139, 69, 19}},
    {"salmon", {250, 128, 114}},
    {"sandybrown", {244, 164, 96}},
    {"seagreen", {46, 139, 87}},
    {"seashell", {255, 245, 238}},
    {"sienna", {160, 82, 45}},
    {"silver", {192, 192, 192}},
    {"skyblue", {135, 206, 235}},
    {"slateblue", {106, 90, 205}},
    {"slategray", {112, 128, 144}},
    {"slategrey", {112, 128, 144}},
    {"snow", {255, 250, 250}},
    {"springgreen", {0, 255, 127}},
    {"steelblue", {70, 130, 180}},
    {"tan", {210, 180, 140}},
    {"teal", {0, 128, 128}},
    {"thistle", {216, 191, 216}},
    {"tomato", {255, 99, 71}},
    {"turquoise", {64, 224, 208}},
    {"violet", {238, 130, 238}},
    {"wheat", {245, 222, 179}},
    {"white", {255, 255, 255}},
    {"whitesmoke", {245, 245, 245}},
    {"yellow", {255, 255, 0}},
    {"yellowgreen", {154, 205, 50}},
};

constexpr std::string_view CSS_SPECIAL_COLOR_KEYWORDS[]{
    "accentcolor",
    "accentcolortext",
    "activeborder",
    "activecaption",
    "activetext",
    "appworkspace",
    "background",
    "buttonborder",
    "buttonface",
    "buttonhighlight",
    "buttonshadow",
    "buttontext",
    "canvas",
    "canvastext",
    "captiontext",
    "currentcolor",
    "field",
    "fieldtext",
    "graytext",
    "highlight",
    "highlighttext",
    "inactiveborder",
    "inactivecaption",
    "inactivecaptiontext",
    "infobackground",
    "infotext",
    "linktext",
    "mark",
    "marktext",
    "menu",
    "menutext",
    "scrollbar",
    "selecteditem",
    "selecteditemtext",
    "threeddarkshadow",
    "threedface",
    "threedhighlight",
    "threedlightshadow",
    "threedshadow",
    "transparent",
    "visitedtext",
    "window",
    "windowframe",
    "windowtext",
};

std::string trim(std::string_view text)
{
    const auto is_space = [](unsigned char c) { return std::isspace(c) != 0; };
    auto first{text.begin()};
    while (first != text.end() && is_space(static_cast<unsigned char>(*first)))
    {
        ++first;
    }
    auto last{text.end()};
    while (last != first && is_space(static_cast<unsigned char>(*(last - 1))))
    {
        --last;
    }
    return {first, last};
}

std::string ascii_lower(std::string_view text)
{
    std::string result{text};
    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return result;
}

std::vector<std::string> split_slash_values(std::string_view text)
{
    std::vector<std::string> result;
    std::size_t start{};
    while (start <= text.size())
    {
        const std::size_t slash{text.find('/', start)};
        const std::size_t end{slash == std::string_view::npos ? text.size() : slash};
        result.emplace_back(trim(text.substr(start, end - start)));
        if (slash == std::string_view::npos)
        {
            break;
        }
        start = slash + 1U;
    }
    return result;
}

void require_component_count(const std::vector<std::string> &values, std::string_view type)
{
    if (values.size() != 3U)
    {
        throw std::runtime_error("Color specification '" + std::string{type} + "' requires three components");
    }
}

int parse_int_component(const std::string &text, std::string_view name, int min, int max)
{
    try
    {
        std::size_t length{};
        const int value{std::stoi(text, &length)};
        if (length != text.size() || value < min || value > max)
        {
            throw std::runtime_error("Color component '" + std::string{name} + "' is outside the valid range");
        }
        return value;
    }
    catch (const std::invalid_argument &)
    {
        throw std::runtime_error("Color component '" + std::string{name} + "' is not an integer");
    }
    catch (const std::out_of_range &)
    {
        throw std::runtime_error("Color component '" + std::string{name} + "' is outside the valid range");
    }
}

double parse_double_component(const std::string &text, std::string_view name, double min, double max)
{
    try
    {
        std::size_t length{};
        const double value{std::stod(text, &length)};
        if (length != text.size() || !std::isfinite(value) || value < min || value > max)
        {
            throw std::runtime_error("Color component '" + std::string{name} + "' is outside the valid range");
        }
        return value;
    }
    catch (const std::invalid_argument &)
    {
        throw std::runtime_error("Color component '" + std::string{name} + "' is not a number");
    }
    catch (const std::out_of_range &)
    {
        throw std::runtime_error("Color component '" + std::string{name} + "' is outside the valid range");
    }
}

int byte_from_unit(double value)
{
    return std::clamp(static_cast<int>(std::lround(value * 255.0)), 0, 255);
}

RgbColor rgb_from_chroma(double hue, double chroma, double match)
{
    const double hue_sector{hue == 360.0 ? 0.0 : hue / 60.0};
    const double x{chroma * (1.0 - std::fabs(std::fmod(hue_sector, 2.0) - 1.0))};
    double red{};
    double green{};
    double blue{};
    if (hue_sector < 1.0)
    {
        red = chroma;
        green = x;
    }
    else if (hue_sector < 2.0)
    {
        red = x;
        green = chroma;
    }
    else if (hue_sector < 3.0)
    {
        green = chroma;
        blue = x;
    }
    else if (hue_sector < 4.0)
    {
        green = x;
        blue = chroma;
    }
    else if (hue_sector < 5.0)
    {
        red = x;
        blue = chroma;
    }
    else
    {
        red = chroma;
        blue = x;
    }
    return {byte_from_unit(red + match), byte_from_unit(green + match), byte_from_unit(blue + match)};
}

RgbColor parse_rgb(std::string_view text)
{
    const std::vector<std::string> values{split_slash_values(text)};
    require_component_count(values, "rgb");
    return {parse_int_component(values[0], "red", 0, 255), parse_int_component(values[1], "green", 0, 255),
        parse_int_component(values[2], "blue", 0, 255)};
}

RgbColor parse_hsv(std::string_view text)
{
    const std::vector<std::string> values{split_slash_values(text)};
    require_component_count(values, "hsv");
    const double hue{parse_double_component(values[0], "hue", 0.0, 360.0)};
    const double saturation{parse_double_component(values[1], "saturation", 0.0, 1.0)};
    const double value{parse_double_component(values[2], "value", 0.0, 1.0)};
    const double chroma{value * saturation};
    return rgb_from_chroma(hue, chroma, value - chroma);
}

RgbColor parse_hsl(std::string_view text)
{
    const std::vector<std::string> values{split_slash_values(text)};
    require_component_count(values, "hsl");
    const double hue{parse_double_component(values[0], "hue", 0.0, 360.0)};
    const double saturation{parse_double_component(values[1], "saturation", 0.0, 1.0)};
    const double lightness{parse_double_component(values[2], "lightness", 0.0, 1.0)};
    const double chroma{(1.0 - std::fabs(2.0 * lightness - 1.0)) * saturation};
    return rgb_from_chroma(hue, chroma, lightness - chroma / 2.0);
}

bool is_special_css_color_keyword(std::string_view keyword)
{
    return std::find(std::begin(CSS_SPECIAL_COLOR_KEYWORDS), std::end(CSS_SPECIAL_COLOR_KEYWORDS), keyword) !=
        std::end(CSS_SPECIAL_COLOR_KEYWORDS);
}

const RgbColor *find_named_color(std::string_view keyword)
{
    const auto it{std::find_if(std::begin(CSS_NAMED_COLORS), std::end(CSS_NAMED_COLORS),
        [&](const NamedColor &color) { return color.name == keyword; })};
    return it == std::end(CSS_NAMED_COLORS) ? nullptr : &it->color;
}

RgbColor parse_unprefixed(std::string_view text)
{
    const std::string keyword{ascii_lower(text)};
    if (is_special_css_color_keyword(keyword))
    {
        throw std::runtime_error("CSS color keyword '" + keyword + "' does not name one fixed color");
    }
    if (const RgbColor *color{find_named_color(keyword)})
    {
        return *color;
    }
    return parse_rgb(text);
}

} // namespace

RgbColor parse_color_spec(std::string_view text)
{
    const std::string spec{trim(text)};
    if (spec.empty())
    {
        throw std::runtime_error("Color specification is empty");
    }
    const std::size_t prefix_end{spec.find(':')};
    if (prefix_end == std::string::npos)
    {
        return parse_unprefixed(spec);
    }

    const std::string prefix{ascii_lower(std::string_view{spec}.substr(0, prefix_end))};
    const std::string_view value{std::string_view{spec}.substr(prefix_end + 1U)};
    if (prefix == "rgb")
    {
        return parse_rgb(value);
    }
    if (prefix == "hsv")
    {
        return parse_hsv(value);
    }
    if (prefix == "hsl")
    {
        return parse_hsl(value);
    }
    throw std::runtime_error("Unknown color specification prefix '" + prefix + "'");
}

} // namespace ParFile
