#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

#pragma once

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Config.hpp>


namespace sf
{
class Time;
}

namespace sf::priv
{
////////////////////////////////////////////////////////////
/// \brief Unix implementation of sf::Sleep
///
/// \param time Time to sleep
///
////////////////////////////////////////////////////////////
void sleepImpl(Time time);

} // namespace sf::priv
