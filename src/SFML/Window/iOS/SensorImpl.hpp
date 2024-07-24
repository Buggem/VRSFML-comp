#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

#pragma once

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Window/Sensor.hpp>


namespace sf::priv
{
////////////////////////////////////////////////////////////
/// \brief iOS implementation of sensors
///
////////////////////////////////////////////////////////////
class SensorImpl
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Perform the global initialization of the sensor module
    ///
    ////////////////////////////////////////////////////////////
    static void initialize();

    ////////////////////////////////////////////////////////////
    /// \brief Perform the global cleanup of the sensor module
    ///
    ////////////////////////////////////////////////////////////
    static void cleanup();

    ////////////////////////////////////////////////////////////
    /// \brief Check if a sensor is available
    ///
    /// \param sensor Sensor to check
    ///
    /// \return True if the sensor is available, false otherwise
    ///
    ////////////////////////////////////////////////////////////
    static bool isAvailable(Sensor::Type sensor);

    ////////////////////////////////////////////////////////////
    /// \brief Open the sensor
    ///
    /// \param sensor Type of the sensor
    ///
    /// \return True on success, false on failure
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool open(Sensor::Type sensor);

    ////////////////////////////////////////////////////////////
    /// \brief Close the sensor
    ///
    ////////////////////////////////////////////////////////////
    void close();

    ////////////////////////////////////////////////////////////
    /// \brief Update the sensor and get its new value
    ///
    /// \return Sensor value
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Vector3f update();

    ////////////////////////////////////////////////////////////
    /// \brief Enable or disable the sensor
    ///
    /// \param enabled True to enable, false to disable
    ///
    ////////////////////////////////////////////////////////////
    void setEnabled(bool enabled);

private:
    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    Sensor::Type m_sensor;  ///< Type of the sensor
    bool         m_enabled; ///< Enable state of the sensor
};

} // namespace sf::priv
