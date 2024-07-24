#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

#pragma once

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Audio/SoundFileWriter.hpp>

#include <SFML/Base/InPlacePImpl.hpp>

#include <vector>

#include <cstdint>


namespace sf
{
class Path;
} // namespace sf


namespace sf::priv
{
////////////////////////////////////////////////////////////
/// \brief Implementation of sound file writer that handles FLAC files
///
////////////////////////////////////////////////////////////
class SoundFileWriterFlac : public SoundFileWriter
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    ////////////////////////////////////////////////////////////
    SoundFileWriterFlac();

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~SoundFileWriterFlac() override;

    ////////////////////////////////////////////////////////////
    /// \brief Check if this writer can handle a file on disk
    ///
    /// \param filename Path of the sound file to check
    ///
    /// \return True if the file can be written by this writer
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] static bool check(const Path& filename);

    ////////////////////////////////////////////////////////////
    /// \brief Open a sound file for writing
    ///
    /// \param filename     Path of the file to open
    /// \param sampleRate   Sample rate of the sound
    /// \param channelCount Number of channels of the sound
    /// \param channelMap   Map of position in sample frame to sound channel
    ///
    /// \return True if the file was successfully opened
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool open(const Path&                      filename,
                            unsigned int                     sampleRate,
                            unsigned int                     channelCount,
                            const std::vector<SoundChannel>& channelMap) override;

    ////////////////////////////////////////////////////////////
    /// \brief Write audio samples to the open file
    ///
    /// \param samples Pointer to the sample array to write
    /// \param count   Number of samples to write
    ///
    ////////////////////////////////////////////////////////////
    void write(const std::int16_t* samples, std::uint64_t count) override;

private:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    struct Impl;
    base::InPlacePImpl<Impl, 128> m_impl; //!< Implementation details
};

} // namespace sf::priv
