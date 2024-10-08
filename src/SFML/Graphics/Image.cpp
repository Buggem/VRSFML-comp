#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include "SFML/Graphics/Image.hpp"

#include "SFML/System/Err.hpp"
#include "SFML/System/InputStream.hpp"
#include "SFML/System/Path.hpp"
#include "SFML/System/PathUtils.hpp"
#include "SFML/System/Vector2.hpp"

#include "SFML/Base/Algorithm.hpp"
#include "SFML/Base/Assert.hpp"
#include "SFML/Base/Optional.hpp"
#include "SFML/Base/PassKey.hpp"
#include "SFML/Base/UniquePtr.hpp"

#ifdef SFML_SYSTEM_ANDROID
#include "SFML/System/Android/Activity.hpp"
#include "SFML/System/Android/ResourceStream.hpp"
#endif

#define STB_IMAGE_STATIC
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <string>
#include <vector>

#include <cstring>


namespace
{
// stb_image callbacks that operate on a sf::InputStream
int read(void* user, char* data, int size)
{
    auto&                    stream = *static_cast<sf::InputStream*>(user);
    const sf::base::Optional count  = stream.read(data, static_cast<std::size_t>(size));
    return count.hasValue() ? static_cast<int>(*count) : -1;
}

void skip(void* user, int size)
{
    auto& stream = *static_cast<sf::InputStream*>(user);
    if (!stream.seek(stream.tell().value() + static_cast<std::size_t>(size)).hasValue())
        sf::priv::err() << "Failed to seek image loader input stream";
}

int eof(void* user)
{
    auto& stream = *static_cast<sf::InputStream*>(user);
    return stream.tell().value() >= stream.getSize().value();
}

// Deleter for STB pointers
struct StbDeleter
{
    void operator()(stbi_uc* image) const
    {
        stbi_image_free(image);
    }
};
using StbPtr = sf::base::UniquePtr<stbi_uc, StbDeleter>;
} // namespace


namespace sf
{
////////////////////////////////////////////////////////////
struct [[nodiscard]] Image::Impl
{
    Vector2u                  size;   //!< Image size
    std::vector<std::uint8_t> pixels; //!< Pixels of the image

    template <typename... VectorArgs>
    [[nodiscard]] explicit Impl(Vector2u theSize, VectorArgs&&... vectorArgs) :
    size(theSize),
    pixels(static_cast<VectorArgs&&>(vectorArgs)...)
    {
    }
};


////////////////////////////////////////////////////////////
Image::~Image() = default;


////////////////////////////////////////////////////////////
Image::Image(const Image& rhs) = default;


////////////////////////////////////////////////////////////
Image& Image::operator=(const Image&) = default;


////////////////////////////////////////////////////////////
Image::Image(Image&&) noexcept = default;


////////////////////////////////////////////////////////////
Image& Image::operator=(Image&&) noexcept = default;


////////////////////////////////////////////////////////////
base::Optional<Image> Image::create(Vector2u size, Color color)
{
    base::Optional<Image> result; // Use a single local variable for NRVO

    if (size.x == 0 || size.y == 0)
    {
        priv::err() << "Failed to create image, invalid size (zero) provided";
        return result; // Empty optional
    }

    result.emplace(base::PassKey<Image>{}, size, static_cast<std::size_t>(size.x) * static_cast<std::size_t>(size.y) * 4);

    // Fill it with the specified color
    std::uint8_t*       ptr = result->m_impl->pixels.data();
    std::uint8_t* const end = ptr + result->m_impl->pixels.size();

    while (ptr < end)
    {
        *ptr++ = color.r;
        *ptr++ = color.g;
        *ptr++ = color.b;
        *ptr++ = color.a;
    }

    return result;
}


////////////////////////////////////////////////////////////
base::Optional<Image> Image::create(Vector2u size, const std::uint8_t* pixels)
{
    if (size.x == 0 || size.y == 0)
    {
        priv::err() << "Failed to create image, invalid size (zero) provided";
        return base::nullOpt;
    }

    if (pixels == nullptr)
    {
        priv::err() << "Failed to create image, null pixels pointer provided";
        return base::nullOpt;
    }

    return base::makeOptional<Image>(base::PassKey<Image>{}, size, pixels, pixels + size.x * size.y * 4);
}


////////////////////////////////////////////////////////////
template <typename... VectorArgs>
Image::Image(base::PassKey<Image>&&, Vector2u size, VectorArgs&&... vectorArgs) :
m_impl(size, static_cast<VectorArgs&&>(vectorArgs)...)
{
    SFML_BASE_ASSERT(size.x > 0 && "Attempted to create an image with size.x == 0");
    SFML_BASE_ASSERT(size.y > 0 && "Attempted to create an image with size.y == 0");
    SFML_BASE_ASSERT(!m_impl->pixels.empty() && "Attempted to create an image with no pixels");
}


////////////////////////////////////////////////////////////
base::Optional<Image> Image::loadFromFile(const Path& filename)
{
#ifdef SFML_SYSTEM_ANDROID

    if (priv::getActivityStatesPtr() != nullptr)
    {
        priv::ResourceStream stream(filename);
        return loadFromStream(stream);
    }

#endif

    // Load the image and get a pointer to the pixels in memory
    int        width    = 0;
    int        height   = 0;
    int        channels = 0;
    const auto ptr = StbPtr(stbi_load(filename.to<std::string>().c_str(), &width, &height, &channels, STBI_rgb_alpha));

    if (ptr)
    {
        SFML_BASE_ASSERT(width > 0 && "Loaded image from file with width == 0");
        SFML_BASE_ASSERT(height > 0 && "Loaded image from file with height == 0");

        return base::makeOptional<Image>(base::PassKey<Image>{},
                                         Vector2i{width, height}.to<Vector2u>(),
                                         ptr.get(),
                                         ptr.get() + width * height * 4);
    }

    // Error, failed to load the image
    priv::err() << "Failed to load image\n"
                << priv::PathDebugFormatter{filename} << "\nReason: " << stbi_failure_reason();

    return base::nullOpt;
}


////////////////////////////////////////////////////////////
base::Optional<Image> Image::loadFromMemory(const void* data, std::size_t size)
{
    // Check input parameters
    if (data == nullptr || size == 0)
    {
        priv::err() << "Failed to load image from memory, no data provided";
        return base::nullOpt;
    }

    // Load the image and get a pointer to the pixels in memory
    int width    = 0;
    int height   = 0;
    int channels = 0;

    const auto*  buffer = static_cast<const unsigned char*>(data);
    const StbPtr ptr(stbi_load_from_memory(buffer, static_cast<int>(size), &width, &height, &channels, STBI_rgb_alpha));

    if (ptr == nullptr)
    {
        priv::err() << "Failed to load image from memory. Reason: " << stbi_failure_reason();
        return base::nullOpt;
    }

    SFML_BASE_ASSERT(width > 0 && "Loaded image from memory with width == 0");
    SFML_BASE_ASSERT(height > 0 && "Loaded image from memory with height == 0");

    return base::makeOptional<Image>(base::PassKey<Image>{},
                                     Vector2i{width, height}.to<Vector2u>(),
                                     ptr.get(),
                                     ptr.get() + width * height * 4);
}


////////////////////////////////////////////////////////////
base::Optional<Image> Image::loadFromStream(InputStream& stream)
{
    // Make sure that the stream's reading position is at the beginning
    if (!stream.seek(0).hasValue())
    {
        priv::err() << "Failed to seek image stream";
        return base::nullOpt;
    }

    // Setup the stb_image callbacks
    stbi_io_callbacks callbacks;
    callbacks.read = read;
    callbacks.skip = skip;
    callbacks.eof  = eof;

    // Load the image and get a pointer to the pixels in memory
    int width    = 0;
    int height   = 0;
    int channels = 0;

    const StbPtr ptr(stbi_load_from_callbacks(&callbacks, &stream, &width, &height, &channels, STBI_rgb_alpha));

    if (ptr == nullptr)
    {
        priv::err() << "Failed to load image from stream. Reason: " << stbi_failure_reason();
        return base::nullOpt;
    }

    SFML_BASE_ASSERT(width > 0 && "Loaded image from stream with width == 0");
    SFML_BASE_ASSERT(height > 0 && "Loaded image from stream with height == 0");

    return base::makeOptional<Image>(base::PassKey<Image>{},
                                     Vector2i{width, height}.to<Vector2u>(),
                                     ptr.get(),
                                     ptr.get() + width * height * 4);
}


////////////////////////////////////////////////////////////
Vector2u Image::getSize() const
{
    return m_impl->size;
}


////////////////////////////////////////////////////////////
void Image::createMaskFromColor(Color color, std::uint8_t alpha)
{
    // Make sure that the image is not empty
    SFML_BASE_ASSERT(!m_impl->pixels.empty());

    // Replace the alpha of the pixels that match the transparent color
    std::uint8_t* ptr = m_impl->pixels.data();
    std::uint8_t* end = ptr + m_impl->pixels.size();

    while (ptr < end)
    {
        if ((ptr[0] == color.r) && (ptr[1] == color.g) && (ptr[2] == color.b) && (ptr[3] == color.a))
            ptr[3] = alpha;
        ptr += 4;
    }
}


////////////////////////////////////////////////////////////
bool Image::copy(const Image& source, Vector2u dest, const IntRect& sourceRect, bool applyAlpha)
{
    // Make sure that both images are valid
    SFML_BASE_ASSERT(source.m_impl->size.x > 0 && source.m_impl->size.y > 0 && m_impl->size.x > 0 && m_impl->size.y > 0);

    // Make sure the sourceRect components are non-negative before casting them to unsigned values
    if (sourceRect.position.x < 0 || sourceRect.position.y < 0 || sourceRect.size.x < 0 || sourceRect.size.y < 0)
        return false;

    auto srcRect = sourceRect.to<Rect<unsigned int>>();

    // Use the whole source image as srcRect if the provided source rectangle is empty
    if (srcRect.size.x == 0 || srcRect.size.y == 0)
    {
        srcRect = Rect<unsigned int>({0, 0}, source.m_impl->size);
    }
    // Otherwise make sure the provided source rectangle fits into the source image
    else
    {
        // Checking the bottom right corner is enough because
        // left and top are non-negative and width and height are positive.
        if (source.m_impl->size.x < srcRect.position.x + srcRect.size.x ||
            source.m_impl->size.y < srcRect.position.y + srcRect.size.y)
            return false;
    }

    // Make sure the destination position is within this image bounds
    if (m_impl->size.x <= dest.x || m_impl->size.y <= dest.y)
        return false;

    // Then find the valid size of the destination rectangle
    const Vector2u dstSize(base::min(m_impl->size.x - dest.x, srcRect.size.x),
                           base::min(m_impl->size.y - dest.y, srcRect.size.y));

    // Precompute as much as possible
    const std::size_t  pitch     = static_cast<std::size_t>(dstSize.x) * 4;
    const unsigned int srcStride = source.m_impl->size.x * 4;
    const unsigned int dstStride = m_impl->size.x * 4;

    const std::uint8_t* srcPixels = source.m_impl->pixels.data() +
                                    (srcRect.position.x + srcRect.position.y * source.m_impl->size.x) * 4;
    std::uint8_t* dstPixels = m_impl->pixels.data() + (dest.x + dest.y * m_impl->size.x) * 4;

    // Copy the pixels
    if (applyAlpha)
    {
        // Interpolation using alpha values, pixel by pixel (slower)
        for (unsigned int i = 0; i < dstSize.y; ++i)
        {
            for (unsigned int j = 0; j < dstSize.x; ++j)
            {
                // Get a direct pointer to the components of the current pixel
                const std::uint8_t* src = srcPixels + j * 4;
                std::uint8_t*       dst = dstPixels + j * 4;

                // Interpolate RGBA components using the alpha values of the destination and source pixels
                const std::uint8_t srcAlpha = src[3];
                const std::uint8_t dstAlpha = dst[3];
                const auto outAlpha = static_cast<std::uint8_t>(srcAlpha + dstAlpha - srcAlpha * dstAlpha / 255);

                dst[3] = outAlpha;

                if (outAlpha)
                    for (int k = 0; k < 3; k++)
                        dst[k] = static_cast<std::uint8_t>((src[k] * srcAlpha + dst[k] * (outAlpha - srcAlpha)) / outAlpha);
                else
                    for (int k = 0; k < 3; k++)
                        dst[k] = src[k];
            }

            srcPixels += srcStride;
            dstPixels += dstStride;
        }
    }
    else
    {
        // Optimized copy ignoring alpha values, row by row (faster)
        for (unsigned int i = 0; i < dstSize.y; ++i)
        {
            std::memcpy(dstPixels, srcPixels, pitch);
            srcPixels += srcStride;
            dstPixels += dstStride;
        }
    }

    return true;
}


////////////////////////////////////////////////////////////
void Image::setPixel(Vector2u coords, Color color)
{
    SFML_BASE_ASSERT(coords.x < m_impl->size.x && "Image::setPixel() x coordinate is out of bounds");
    SFML_BASE_ASSERT(coords.y < m_impl->size.y && "Image::setPixel() y coordinate is out of bounds");

    const auto    index = (coords.x + coords.y * m_impl->size.x) * 4;
    std::uint8_t* pixel = &m_impl->pixels[index];

    *pixel++ = color.r;
    *pixel++ = color.g;
    *pixel++ = color.b;
    *pixel++ = color.a;
}


////////////////////////////////////////////////////////////
Color Image::getPixel(Vector2u coords) const
{
    SFML_BASE_ASSERT(coords.x < m_impl->size.x && "Image::getPixel() x coordinate is out of bounds");
    SFML_BASE_ASSERT(coords.y < m_impl->size.y && "Image::getPixel() y coordinate is out of bounds");

    const auto          index = (coords.x + coords.y * m_impl->size.x) * 4;
    const std::uint8_t* pixel = &m_impl->pixels[index];

    return {pixel[0], pixel[1], pixel[2], pixel[3]};
}


////////////////////////////////////////////////////////////
const std::uint8_t* Image::getPixelsPtr() const
{
    SFML_BASE_ASSERT(!m_impl->pixels.empty());
    return m_impl->pixels.data();
}


////////////////////////////////////////////////////////////
void Image::flipHorizontally()
{
    SFML_BASE_ASSERT(!m_impl->pixels.empty());

    const std::size_t rowSize = m_impl->size.x * 4;

    for (std::size_t y = 0; y < m_impl->size.y; ++y)
    {
        auto left = m_impl->pixels.begin() + static_cast<decltype(m_impl->pixels)::difference_type>(y * rowSize);
        auto right = m_impl->pixels.begin() + static_cast<decltype(m_impl->pixels)::difference_type>((y + 1) * rowSize - 4);

        for (std::size_t x = 0; x < m_impl->size.x / 2; ++x)
        {
            base::swapRanges(left, left + 4, right);

            left += 4;
            right -= 4;
        }
    }
}


////////////////////////////////////////////////////////////
void Image::flipVertically()
{
    SFML_BASE_ASSERT(!m_impl->pixels.empty());

    const auto rowSize = static_cast<decltype(m_impl->pixels)::difference_type>(m_impl->size.x * 4);

    auto top    = m_impl->pixels.begin();
    auto bottom = m_impl->pixels.end() - rowSize;

    for (std::size_t y = 0; y < m_impl->size.y / 2; ++y)
    {
        base::swapRanges(top, top + rowSize, bottom);

        top += rowSize;
        bottom -= rowSize;
    }
}

} // namespace sf
