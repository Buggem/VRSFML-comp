#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Audio/ChannelMap.hpp"
#include "SFML/Audio/SoundChannel.hpp"

#include <vector>


namespace sf
{
////////////////////////////////////////////////////////////
struct ChannelMap::Impl
{
    std::vector<SoundChannel> channels;
};


////////////////////////////////////////////////////////////
ChannelMap::ChannelMap() = default;


////////////////////////////////////////////////////////////
ChannelMap::ChannelMap(std::initializer_list<SoundChannel> channels) : m_impl(channels)
{
}


////////////////////////////////////////////////////////////
ChannelMap::~ChannelMap() = default;


////////////////////////////////////////////////////////////
ChannelMap::ChannelMap(const ChannelMap&) = default;


////////////////////////////////////////////////////////////
ChannelMap::ChannelMap(ChannelMap&&) noexcept = default;


////////////////////////////////////////////////////////////
ChannelMap& ChannelMap::operator=(const ChannelMap&) = default;


////////////////////////////////////////////////////////////
ChannelMap& ChannelMap::operator=(ChannelMap&&) noexcept = default;


////////////////////////////////////////////////////////////
[[nodiscard]] std::size_t ChannelMap::getSize() const
{
    return m_impl->channels.size();
}


////////////////////////////////////////////////////////////
[[nodiscard]] bool ChannelMap::isEmpty() const
{
    return m_impl->channels.empty();
}


////////////////////////////////////////////////////////////
[[nodiscard]] SoundChannel* ChannelMap::begin()
{
    return m_impl->channels.data();
}


////////////////////////////////////////////////////////////
[[nodiscard]] SoundChannel* ChannelMap::end()
{
    return m_impl->channels.data() + m_impl->channels.size();
}


////////////////////////////////////////////////////////////
[[nodiscard]] const SoundChannel* ChannelMap::begin() const
{
    return m_impl->channels.data();
}


////////////////////////////////////////////////////////////
[[nodiscard]] const SoundChannel* ChannelMap::end() const
{
    return m_impl->channels.data() + m_impl->channels.size();
}


////////////////////////////////////////////////////////////
[[nodiscard]] const SoundChannel& ChannelMap::operator[](std::size_t index) const
{
    return m_impl->channels.at(index);
}


////////////////////////////////////////////////////////////
void ChannelMap::reserve(std::size_t count)
{
    m_impl->channels.reserve(count);
}


////////////////////////////////////////////////////////////
void ChannelMap::append(SoundChannel channel)
{
    m_impl->channels.push_back(channel);
}

} // namespace sf
