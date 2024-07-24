#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

#pragma once

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/System/Export.hpp>

#include <SFML/Base/InPlacePImpl.hpp>

#include <iosfwd>


namespace sf
{
////////////////////////////////////////////////////////////
class [[nodiscard]] Path
{
public:
#ifdef SFML_SYSTEM_EMSCRIPTEN
    using value_type = char;
#else
    using value_type = wchar_t;
#endif
    // using string_type = std::basic_string<value_type>;

    [[nodiscard]] static Path tempDirectoryPath();

    [[nodiscard]] explicit Path();

    template <typename T>
    [[nodiscard]] Path(const T& source);

    template <typename T>
    [[nodiscard]] Path(const T* source);

    // [[nodiscard]] Path(const value_type* str);
    // [[nodiscard]] Path(const string_type& str);
    // [[nodiscard]] Path(const char* str);
    // [[nodiscard]] Path(const std::string& str);

    ~Path();

    Path(const Path&);
    Path& operator=(const Path&);

    Path(Path&&) noexcept;
    Path& operator=(Path&&) noexcept;

    [[nodiscard]] Path filename() const;
    [[nodiscard]] Path extension() const;
    [[nodiscard]] Path absolute() const;

    [[nodiscard]] const value_type* c_str() const; // TODO P1: support other platforms
    // [[nodiscard]] std::string       string() const;

    // [[nodiscard]] operator std::string() const;

    template <typename T>
    T to() const;

    [[nodiscard]] bool remove() const;
    [[nodiscard]] bool empty() const;
    [[nodiscard]] bool exists() const;

    Path& operator/=(const Path& rhs);

    friend Path          operator/(const Path& lhs, const Path& rhs);
    friend std::ostream& operator<<(std::ostream& os, const Path& path);

    template <typename T>
    bool operator==(const T* str) const;

    template <typename T>
    bool operator!=(const T* str) const;

    template <typename T>
    bool operator==(const T& str) const;

    template <typename T>
    bool operator!=(const T& str) const;

private:
    [[nodiscard]] Path(int, const void* fsPath);

    struct Impl;
    base::InPlacePImpl<Impl, 64> m_impl;
};


} // namespace sf
