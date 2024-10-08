#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Transform.hpp"

#include "SFML/System/Angle.hpp"

#include "SFML/Base/Math/Cos.hpp"
#include "SFML/Base/Math/Sin.hpp"


namespace sf
{
////////////////////////////////////////////////////////////
Transform& Transform::rotate(Angle angle)
{
    const float rad = angle.asRadians();
    const float cos = base::cos(rad);
    const float sin = base::sin(rad);

    // clang-format off
    const Transform rotation(cos, -sin, 0,
                             sin,  cos, 0);
    // clang-format on

    return combine(rotation);
}


////////////////////////////////////////////////////////////
Transform& Transform::rotate(Angle angle, Vector2f center)
{
    const float rad = angle.asRadians();
    const float cos = base::cos(rad);
    const float sin = base::sin(rad);

    // clang-format off
    const Transform rotation(cos, -sin, center.x * (1 - cos) + center.y * sin,
                             sin,  cos, center.y * (1 - cos) - center.x * sin);
    // clang-format on

    return combine(rotation);
}

} // namespace sf
