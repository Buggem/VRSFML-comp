#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

#pragma once

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Window/Sensor.hpp>
#include <SFML/Window/SensorImpl.hpp>

#include <SFML/Base/EnumArray.hpp>


namespace sf::priv
{
////////////////////////////////////////////////////////////
/// \brief Global sensor manager
///
////////////////////////////////////////////////////////////
class SensorManager
{
public:
    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy constructor
    ///
    ////////////////////////////////////////////////////////////
    SensorManager(const SensorManager&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Deleted copy assignment
    ///
    ////////////////////////////////////////////////////////////
    SensorManager& operator=(const SensorManager&) = delete;

    ////////////////////////////////////////////////////////////
    /// \brief Get the global unique instance of the manager
    ///
    /// \return Unique instance of the sensor manager
    ///
    ////////////////////////////////////////////////////////////
    static SensorManager& getInstance();

    ////////////////////////////////////////////////////////////
    /// \brief Check if a sensor is available on the underlying platform
    ///
    /// \param sensor Sensor to check
    ///
    /// \return True if the sensor is available, false otherwise
    ///
    ////////////////////////////////////////////////////////////
    bool isAvailable(Sensor::Type sensor);

    ////////////////////////////////////////////////////////////
    /// \brief Enable or disable a sensor
    ///
    /// \param sensor  Sensor to modify
    /// \param enabled Whether it should be enabled or not
    ///
    ////////////////////////////////////////////////////////////
    void setEnabled(Sensor::Type sensor, bool enabled);

    ////////////////////////////////////////////////////////////
    /// \brief Check if a sensor is enabled
    ///
    /// \param sensor Sensor to check
    ///
    /// \return True if the sensor is enabled, false otherwise
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] bool isEnabled(Sensor::Type sensor) const;

    ////////////////////////////////////////////////////////////
    /// \brief Get the current value of a sensor
    ///
    /// \param sensor Sensor to read
    ///
    /// \return Current value of the sensor
    ///
    ////////////////////////////////////////////////////////////
    [[nodiscard]] Vector3f getValue(Sensor::Type sensor) const;

    ////////////////////////////////////////////////////////////
    /// \brief Update the state of all the sensors
    ///
    ////////////////////////////////////////////////////////////
    void update();

private:
    ////////////////////////////////////////////////////////////
    /// \brief Default constructor
    ///
    ////////////////////////////////////////////////////////////
    SensorManager();

    ////////////////////////////////////////////////////////////
    /// \brief Destructor
    ///
    ////////////////////////////////////////////////////////////
    ~SensorManager();

    ////////////////////////////////////////////////////////////
    /// \brief Sensor information and state
    ///
    ////////////////////////////////////////////////////////////
    struct Item
    {
        bool       available{}; //!< Is the sensor available on this device?
        bool       enabled{};   //!< Current enable state of the sensor
        SensorImpl sensor{};    //!< Sensor implementation
        Vector3f   value;       //!< The current sensor value
    };

    ////////////////////////////////////////////////////////////
    // Member data
    ////////////////////////////////////////////////////////////
    base::EnumArray<Sensor::Type, Item, Sensor::Count> m_sensors; //!< Sensors information and state
};

} // namespace sf::priv
