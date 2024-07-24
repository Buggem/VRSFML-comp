#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

#pragma once

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <X11/X.h> // KeySym

#include <cstdint>

namespace sf::priv
{

////////////////////////////////////////////////////////////
/// \brief Converts a KeySym to UTF-32
///
/// This code was autogenerated from the following table:
/// https://www.cl.cam.ac.uk/~mgk25/ucs/keysyms.txt
///
/// The generated table was edited so that dead keys are mapped to
/// non-combining characters instead of combining diacritical marks
/// because is it more suitable to describe those keys in user interfaces.
///
/// \param keysym keysym to be converted
///
/// \return corresponding UTF-32
///
////////////////////////////////////////////////////////////
char32_t keysymToUnicode(KeySym keysym);

} // namespace sf::priv
