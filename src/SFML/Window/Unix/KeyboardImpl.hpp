#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

#pragma once

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Window/Keyboard.hpp>

#include <X11/Xlib.h> // XKeyEvent


////////////////////////////////////////////////////////////
/// \brief sf::priv::KeyboardImpl helper
///
/// This class implements keyboard handling functions
/// to help sf::priv::InputImpl class.
////////////////////////////////////////////////////////////
namespace sf::priv::KeyboardImpl
{
////////////////////////////////////////////////////////////
/// \copydoc sf::Keyboard::isKeyPressed(Key)
///
////////////////////////////////////////////////////////////
bool isKeyPressed(Keyboard::Key key);

////////////////////////////////////////////////////////////
/// \copydoc sf::Keyboard::isKeyPressed(Scancode)
///
////////////////////////////////////////////////////////////
bool isKeyPressed(Keyboard::Scancode code);

////////////////////////////////////////////////////////////
/// \copydoc sf::Keyboard::localize
///
////////////////////////////////////////////////////////////
Keyboard::Scancode delocalize(Keyboard::Key key);

////////////////////////////////////////////////////////////
/// \copydoc sf::Keyboard::delocalize
///
////////////////////////////////////////////////////////////
Keyboard::Key localize(Keyboard::Scancode code);

////////////////////////////////////////////////////////////
/// \copydoc sf::Keyboard::getDescription
///
////////////////////////////////////////////////////////////
String getDescription(Keyboard::Scancode code);

////////////////////////////////////////////////////////////
/// \brief Get the sf::Keyboard::Key from XKeyEvent
///
/// \param event Event from which key is gotten
///
/// \return A key being pressed or released
///
////////////////////////////////////////////////////////////
Keyboard::Key getKeyFromEvent(XKeyEvent& event);

////////////////////////////////////////////////////////////
/// \brief Get the sf::Keyboard::Scancode from XKeyEvent
///
/// \param event Event from which scancode is gotten
///
/// \return A scancode of a key being pressed or released
///
////////////////////////////////////////////////////////////
Keyboard::Scancode getScancodeFromEvent(XKeyEvent& event);

} // namespace sf::priv::KeyboardImpl
