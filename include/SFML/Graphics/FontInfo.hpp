#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

#pragma once

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics/Export.hpp>

#include <string>


namespace sf
{
////////////////////////////////////////////////////////////
/// \brief Holds various information about a font
///
////////////////////////////////////////////////////////////
struct SFML_GRAPHICS_API FontInfo
{
    std::string family; //!< The font family
};

} // namespace sf
