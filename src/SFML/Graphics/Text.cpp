////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2024 Laurent Gomila (laurent@sfml-dev.org)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Glyph.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>
#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <SFML/Graphics/Vertex.hpp>

#include <utility>

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstring>


namespace
{
// Add an underline or strikethrough line to the vertex array
void addLine(std::vector<sf::Vertex>& vertices,
             std::size_t&             index,
             float                    lineLength,
             float                    lineTop,
             const sf::Color&         color,
             float                    offset,
             float                    thickness,
             float                    outlineThickness = 0)
{
    const float top    = std::floor(lineTop + offset - (thickness / 2) + 0.5f);
    const float bottom = top + std::floor(thickness + 0.5f);

    const sf::Vertex vertexData[] = {{{-outlineThickness, top - outlineThickness}, color, {1.0f, 1.0f}},
                                     {{lineLength + outlineThickness, top - outlineThickness}, color, {1.0f, 1.0f}},
                                     {{-outlineThickness, bottom + outlineThickness}, color, {1.0f, 1.0f}},
                                     {{-outlineThickness, bottom + outlineThickness}, color, {1.0f, 1.0f}},
                                     {{lineLength + outlineThickness, top - outlineThickness}, color, {1.0f, 1.0f}},
                                     {{lineLength + outlineThickness, bottom + outlineThickness}, color, {1.0f, 1.0f}}};

    std::memcpy(vertices.data() + index, vertexData, sizeof(sf::Vertex) * 6);
    index += 6;
}

// Add a glyph quad to the vertex array
void addGlyphQuad(std::vector<sf::Vertex>& vertices,
                  std::size_t&             index,
                  sf::Vector2f             position,
                  const sf::Color&         color,
                  const sf::Glyph&         glyph,
                  float                    italicShear)
{
    const sf::Vector2f padding(1.f, 1.f);

    const sf::Vector2f p1 = glyph.bounds.position - padding;
    const sf::Vector2f p2 = glyph.bounds.position + glyph.bounds.size + padding;

    const auto uv1 = sf::Vector2f(glyph.textureRect.position) - padding;
    const auto uv2 = sf::Vector2f(glyph.textureRect.position + glyph.textureRect.size) + padding;

    const sf::Vertex vertexData[] = {{position + sf::Vector2f(p1.x - italicShear * p1.y, p1.y), color, {uv1.x, uv1.y}},
                                     {position + sf::Vector2f(p2.x - italicShear * p1.y, p1.y), color, {uv2.x, uv1.y}},
                                     {position + sf::Vector2f(p1.x - italicShear * p2.y, p2.y), color, {uv1.x, uv2.y}},
                                     {position + sf::Vector2f(p1.x - italicShear * p2.y, p2.y), color, {uv1.x, uv2.y}},
                                     {position + sf::Vector2f(p2.x - italicShear * p1.y, p1.y), color, {uv2.x, uv1.y}},
                                     {position + sf::Vector2f(p2.x - italicShear * p2.y, p2.y), color, {uv2.x, uv2.y}}};

    std::memcpy(vertices.data() + index, vertexData, sizeof(sf::Vertex) * 6);
    index += 6;
}

} // namespace


namespace sf
{
////////////////////////////////////////////////////////////
Text::Text(const Font& font, String string, unsigned int characterSize) :
m_string(std::move(string)),
m_font(&font),
m_characterSize(characterSize)
{
    SFML_UPDATE_LIFETIME_DEPENDANT(Font, Text, m_font);
}


////////////////////////////////////////////////////////////
void Text::setString(const String& string)
{
    if (m_string == string)
        return;

    m_string             = string;
    m_geometryNeedUpdate = true;
}


////////////////////////////////////////////////////////////
void Text::setFont(const Font& font)
{
    if (m_font == &font)
        return;

    m_font               = &font;
    m_geometryNeedUpdate = true;
}


////////////////////////////////////////////////////////////
void Text::setCharacterSize(unsigned int size)
{
    if (m_characterSize == size)
        return;

    m_characterSize      = size;
    m_geometryNeedUpdate = true;
}


////////////////////////////////////////////////////////////
void Text::setLetterSpacing(float spacingFactor)
{
    if (m_letterSpacingFactor == spacingFactor)
        return;

    m_letterSpacingFactor = spacingFactor;
    m_geometryNeedUpdate  = true;
}


////////////////////////////////////////////////////////////
void Text::setLineSpacing(float spacingFactor)
{
    if (m_lineSpacingFactor == spacingFactor)
        return;

    m_lineSpacingFactor  = spacingFactor;
    m_geometryNeedUpdate = true;
}


////////////////////////////////////////////////////////////
void Text::setStyle(std::uint32_t style)
{
    if (m_style == style)
        return;

    m_style              = style;
    m_geometryNeedUpdate = true;
}


////////////////////////////////////////////////////////////
void Text::setFillColor(const Color& color)
{
    if (color == m_fillColor)
        return;

    m_fillColor = color;

    // Change vertex colors directly, no need to update whole geometry
    // (if geometry is updated anyway, we can skip this step)
    if (!m_geometryNeedUpdate)
    {
        for (std::size_t i = m_fillVerticesStartIndex; i < m_vertices.size(); ++i)
            m_vertices[i].color = m_fillColor;
    }
}


////////////////////////////////////////////////////////////
void Text::setOutlineColor(const Color& color)
{
    if (color == m_outlineColor)
        return;

    m_outlineColor = color;

    // Change vertex colors directly, no need to update whole geometry
    // (if geometry is updated anyway, we can skip this step)
    if (!m_geometryNeedUpdate)
    {
        for (std::size_t i = 0; i < m_fillVerticesStartIndex; ++i)
            m_vertices[i].color = m_outlineColor;
    }
}


////////////////////////////////////////////////////////////
void Text::setOutlineThickness(float thickness)
{
    if (thickness == m_outlineThickness)
        return;

    m_outlineThickness   = thickness;
    m_geometryNeedUpdate = true;
}


////////////////////////////////////////////////////////////
const String& Text::getString() const
{
    return m_string;
}


////////////////////////////////////////////////////////////
const Font& Text::getFont() const
{
    return *m_font;
}


////////////////////////////////////////////////////////////
unsigned int Text::getCharacterSize() const
{
    return m_characterSize;
}


////////////////////////////////////////////////////////////
float Text::getLetterSpacing() const
{
    return m_letterSpacingFactor;
}


////////////////////////////////////////////////////////////
float Text::getLineSpacing() const
{
    return m_lineSpacingFactor;
}


////////////////////////////////////////////////////////////
std::uint32_t Text::getStyle() const
{
    return m_style;
}


////////////////////////////////////////////////////////////
const Color& Text::getFillColor() const
{
    return m_fillColor;
}


////////////////////////////////////////////////////////////
const Color& Text::getOutlineColor() const
{
    return m_outlineColor;
}


////////////////////////////////////////////////////////////
float Text::getOutlineThickness() const
{
    return m_outlineThickness;
}


////////////////////////////////////////////////////////////
Vector2f Text::findCharacterPos(std::size_t index) const
{
    // Adjust the index if it's out of range
    index = priv::min(index, m_string.getSize());

    // Precompute the variables needed by the algorithm
    const bool  isBold          = m_style & Bold;
    float       whitespaceWidth = m_font->getGlyph(U' ', m_characterSize, isBold).advance;
    const float letterSpacing   = (whitespaceWidth / 3.f) * (m_letterSpacingFactor - 1.f);
    whitespaceWidth += letterSpacing;
    const float lineSpacing = m_font->getLineSpacing(m_characterSize) * m_lineSpacingFactor;

    // Compute the position
    Vector2f      position;
    std::uint32_t prevChar = 0;
    for (std::size_t i = 0; i < index; ++i)
    {
        const std::uint32_t curChar = m_string[i];

        // Apply the kerning offset
        position.x += m_font->getKerning(prevChar, curChar, m_characterSize, isBold);
        prevChar = curChar;

        // Handle special characters
        switch (curChar)
        {
            case U' ':
                position.x += whitespaceWidth;
                continue;
            case U'\t':
                position.x += whitespaceWidth * 4;
                continue;
            case U'\n':
                position.y += lineSpacing;
                position.x = 0;
                continue;
        }

        // For regular characters, add the advance offset of the glyph
        position.x += m_font->getGlyph(curChar, m_characterSize, isBold).advance + letterSpacing;
    }

    // Transform the position to global coordinates
    position = getTransform().transformPoint(position);

    return position;
}


////////////////////////////////////////////////////////////
const FloatRect& Text::getLocalBounds() const
{
    ensureGeometryUpdate();

    return m_bounds;
}


////////////////////////////////////////////////////////////
FloatRect Text::getGlobalBounds() const
{
    return getTransform().transformRect(getLocalBounds());
}


////////////////////////////////////////////////////////////
void Text::draw(RenderTarget& target, RenderStates states) const
{
    ensureGeometryUpdate();

    states.transform *= getTransform();
    states.texture        = &m_font->getTexture(m_characterSize);
    states.coordinateType = CoordinateType::Pixels;

    target.draw(m_vertices.data(), m_vertices.size(), PrimitiveType::Triangles, states);
}


////////////////////////////////////////////////////////////
void Text::ensureGeometryUpdate() const
{
    // Do nothing, if geometry has not changed and the font texture has not changed
    if (!m_geometryNeedUpdate && m_font->getTexture(m_characterSize).m_cacheId == m_fontTextureId)
        return;

    // Save the current fonts texture id
    m_fontTextureId = m_font->getTexture(m_characterSize).m_cacheId;

    // Mark geometry as updated
    m_geometryNeedUpdate = false;

    // Clear the previous geometry
    m_vertices.clear();
    m_fillVerticesStartIndex = 0u;
    m_bounds                 = {};

    // No text: nothing to draw
    if (m_string.isEmpty())
        return;

    // Compute values related to the text style
    const bool  isBold             = m_style & Bold;
    const bool  isUnderlined       = m_style & Underlined;
    const bool  isStrikeThrough    = m_style & StrikeThrough;
    const float italicShear        = (m_style & Italic) ? degrees(12).asRadians() : 0.f;
    const float underlineOffset    = m_font->getUnderlinePosition(m_characterSize);
    const float underlineThickness = m_font->getUnderlineThickness(m_characterSize);

    // Compute the location of the strike through dynamically
    // We use the center point of the lowercase 'x' glyph as the reference
    // We reuse the underline thickness as the thickness of the strike through as well
    const float strikeThroughOffset = m_font->getGlyph(U'x', m_characterSize, isBold).bounds.getCenter().y;

    // Precompute the variables needed by the algorithm
    float       whitespaceWidth = m_font->getGlyph(U' ', m_characterSize, isBold).advance;
    const float letterSpacing   = (whitespaceWidth / 3.f) * (m_letterSpacingFactor - 1.f);
    whitespaceWidth += letterSpacing;
    const float lineSpacing = m_font->getLineSpacing(m_characterSize) * m_lineSpacingFactor;

    // TODO
    std::size_t fillQuadCount    = 0;
    std::size_t outlineQuadCount = 0;

    {
        float x = 0.f;

        const std::size_t outlineQuadIncrement = (m_outlineThickness == 0) ? 0 : 1;

        const auto addLinesFake = [&]
        {
            outlineQuadCount += outlineQuadIncrement;
            ++fillQuadCount;
        };

        const auto addGlyphsFake = addLinesFake;

        std::uint32_t prevChar = 0;

        for (const std::uint32_t curChar : m_string)
        {
            // Skip the \r char to avoid weird graphical issues
            if (curChar == U'\r')
                continue;

            // Apply the kerning offset
            x += 1.f;

            // If we're using the underlined or strike through style and there's a new line, draw a line
            if ((curChar == U'\n' && prevChar != U'\n') && (isUnderlined || isStrikeThrough))
                addLinesFake();

            prevChar = curChar;

            // Handle special characters
            if ((curChar == U' ') || (curChar == U'\n') || (curChar == U'\t'))
            {
                switch (curChar)
                {
                    case U' ':
                        x += 1.f;
                        break;
                    case U'\t':
                        x += 1.f;
                        break;
                    case U'\n':
                        x = 0;
                        break;
                }

                // Next glyph, no need to create a quad for whitespace
                continue;
            }

            // Apply the outline
            addGlyphsFake();

            // Advance to the next character
            x += letterSpacing;
        }

        // If we're using the underlined style, add the last line
        if (isUnderlined && (x > 0))
            addLinesFake();

        // If we're using the strike through style, add the last line across all characters
        if (isStrikeThrough && (x > 0))
            addLinesFake();
    }

    const std::size_t outlineVertexCount = outlineQuadCount * 6;
    const std::size_t fillVertexCount    = fillQuadCount * 6;

    m_vertices.resize(outlineVertexCount + fillVertexCount);
    m_fillVerticesStartIndex = outlineVertexCount;

    std::size_t currFillIndex    = outlineVertexCount;
    std::size_t currOutlineIndex = 0;

    float x = 0.f;
    auto  y = static_cast<float>(m_characterSize);

    // Create one quad for each character
    auto  minX = static_cast<float>(m_characterSize);
    auto  minY = static_cast<float>(m_characterSize);
    float maxX = 0.f;
    float maxY = 0.f;

    std::uint32_t prevChar = 0;

    const auto addLines = [this, &currFillIndex, &currOutlineIndex, &x, &y, &underlineThickness](float offset)
    {
        addLine(m_vertices, currFillIndex, x, y, m_fillColor, offset, underlineThickness);

        if (m_outlineThickness != 0)
            addLine(m_vertices, currOutlineIndex, x, y, m_outlineColor, offset, underlineThickness, m_outlineThickness);
    };

    for (const std::uint32_t curChar : m_string)
    {
        // Skip the \r char to avoid weird graphical issues
        if (curChar == U'\r')
            continue;

        // Apply the kerning offset
        x += m_font->getKerning(prevChar, curChar, m_characterSize, isBold);

        if (curChar == U'\n' && prevChar != U'\n')
        {
            // If we're using the underlined style and there's a new line, draw a line
            if (isUnderlined)
                addLines(underlineOffset);

            // If we're using the strike through style and there's a new line, draw a line across all characters
            if (isStrikeThrough)
                addLines(strikeThroughOffset);
        }

        prevChar = curChar;

        // Handle special characters
        if ((curChar == U' ') || (curChar == U'\n') || (curChar == U'\t'))
        {
            // Update the current bounds (min coordinates)
            minX = priv::min(minX, x);
            minY = priv::min(minY, y);

            switch (curChar)
            {
                case U' ':
                    x += whitespaceWidth;
                    break;
                case U'\t':
                    x += whitespaceWidth * 4;
                    break;
                case U'\n':
                    y += lineSpacing;
                    x = 0;
                    break;
            }

            // Update the current bounds (max coordinates)
            maxX = priv::max(maxX, x);
            maxY = priv::max(maxY, y);

            // Next glyph, no need to create a quad for whitespace
            continue;
        }

        // Apply the outline
        if (m_outlineThickness != 0)
        {
            const Glyph& glyph = m_font->getGlyph(curChar, m_characterSize, isBold, m_outlineThickness);

            // Add the outline glyph to the vertices
            addGlyphQuad(m_vertices, currOutlineIndex, Vector2f(x, y), m_outlineColor, glyph, italicShear);
        }

        // Extract the current glyph's description
        const Glyph& glyph = m_font->getGlyph(curChar, m_characterSize, isBold);

        // Add the glyph to the vertices
        addGlyphQuad(m_vertices, currFillIndex, Vector2f(x, y), m_fillColor, glyph, italicShear);

        // Update the current bounds
        const Vector2f p1 = glyph.bounds.position;
        const Vector2f p2 = glyph.bounds.position + glyph.bounds.size;

        minX = priv::min(minX, x + p1.x - italicShear * p2.y);
        maxX = priv::max(maxX, x + p2.x - italicShear * p1.y);
        minY = priv::min(minY, y + p1.y);
        maxY = priv::max(maxY, y + p2.y);

        // Advance to the next character
        x += glyph.advance + letterSpacing;
    }

    // If we're using outline, update the current bounds
    if (m_outlineThickness != 0)
    {
        const float outline = std::abs(std::ceil(m_outlineThickness));
        minX -= outline;
        maxX += outline;
        minY -= outline;
        maxY += outline;
    }

    // If we're using the underlined style, add the last line
    if (isUnderlined && (x > 0))
        addLines(underlineOffset);

    // If we're using the strike through style, add the last line across all characters
    if (isStrikeThrough && (x > 0))
        addLines(strikeThroughOffset);

    // Update the bounding rectangle
    m_bounds.position = Vector2f(minX, minY);
    m_bounds.size     = Vector2f(maxX, maxY) - Vector2f(minX, minY);
}

} // namespace sf
