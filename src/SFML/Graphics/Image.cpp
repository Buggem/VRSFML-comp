#include <SFML/Copyright.hpp> // LICENSE AND COPYRIGHT (C) INFORMATION

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics/Image.hpp>

#include <SFML/System/Err.hpp>
#include <SFML/System/InputStream.hpp>
#include <SFML/System/Path.hpp>
#include <SFML/System/PathUtils.hpp>
#include <SFML/System/StringUtils.hpp>

#include <SFML/Base/Algorithm.hpp>
#include <SFML/Base/Macros.hpp>
#include <SFML/Base/PassKey.hpp>
#include <SFML/Base/UniquePtr.hpp>

#ifdef SFML_SYSTEM_ANDROID
#include <SFML/System/Android/Activity.hpp>
#include <SFML/System/Android/ResourceStream.hpp>
#endif

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <SFML/Base/Assert.hpp>

#include <stb_image_write.h>

#include <algorithm>
#include <string>

#include <cstring>


namespace
{
// stb_image callbacks that operate on a sf::InputStream
int read(void* user, char* data, int size)
{
    auto&                    stream = *static_cast<sf::InputStream*>(user);
    const sf::base::Optional count  = stream.read(data, static_cast<std::size_t>(size));
    return count ? static_cast<int>(*count) : -1;
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

// stb_image callback for constructing a buffer
void bufferFromCallback(void* context, void* data, int size)
{
    const auto* source = static_cast<std::uint8_t*>(data);
    auto*       dest   = static_cast<std::vector<std::uint8_t>*>(context);

    sf::base::appendRangeIntoVector(source, source + size, *dest);
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
base::Optional<Image> Image::create(Vector2u size, const Color& color)
{
    if (size.x == 0 || size.y == 0)
    {
        priv::err() << "Failed to create image, invalid size (zero) provided";
        return base::nullOpt;
    }

    // Create a new pixel buffer first for exception safety's sake
    std::vector<std::uint8_t> newPixels(static_cast<std::size_t>(size.x) * static_cast<std::size_t>(size.y) * 4);

    // Fill it with the specified color
    std::uint8_t*       ptr = newPixels.data();
    std::uint8_t* const end = ptr + newPixels.size();

    while (ptr < end)
    {
        *ptr++ = color.r;
        *ptr++ = color.g;
        *ptr++ = color.b;
        *ptr++ = color.a;
    }

    return makeOptional<Image>(base::PassKey<Image>{}, size, SFML_BASE_MOVE(newPixels));
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

    return makeOptional<Image>(base::PassKey<Image>{}, size, std::vector<std::uint8_t>(pixels, pixels + size.x * size.y * 4));
}


////////////////////////////////////////////////////////////
Image::Image(base::PassKey<Image>&&, Vector2u size, std::vector<std::uint8_t>&& pixels) :
m_size(size),
m_pixels(SFML_BASE_MOVE(pixels))
{
    SFML_BASE_ASSERT(size.x > 0 && "Attempted to create an image with size.x == 0");
    SFML_BASE_ASSERT(size.y > 0 && "Attempted to create an image with size.y == 0");
    SFML_BASE_ASSERT(!m_pixels.empty() && "Attempted to create an image with no pixels");
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

        return sf::base::makeOptional<Image>(base::PassKey<Image>{},
                                             Vector2i{width, height}.to<Vector2u>(),
                                             std::vector<std::uint8_t>{ptr.get(), ptr.get() + width * height * 4});
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

    return sf::base::makeOptional<Image>(base::PassKey<Image>{},
                                         Vector2i{width, height}.to<Vector2u>(),
                                         std::vector<std::uint8_t>{ptr.get(), ptr.get() + width * height * 4});
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

    return sf::base::makeOptional<Image>(base::PassKey<Image>{},
                                         Vector2i{width, height}.to<Vector2u>(),
                                         std::vector<std::uint8_t>{ptr.get(), ptr.get() + width * height * 4});
}


////////////////////////////////////////////////////////////
bool Image::saveToFile(const Path& filename) const
{
    // Make sure the image is not empty
    SFML_BASE_ASSERT(!m_pixels.empty() && m_size.x > 0 && m_size.y > 0);

    // Extract the extension
    const Path extension     = filename.extension();
    const auto convertedSize = m_size.to<Vector2i>();

    // Deduce the image type from its extension
    if (extension == ".bmp")
    {
        // BMP format
        if (stbi_write_bmp(filename.to<std::string>().c_str(), convertedSize.x, convertedSize.y, 4, m_pixels.data()))
            return true;
    }
    else if (extension == ".tga")
    {
        // TGA format
        if (stbi_write_tga(filename.to<std::string>().c_str(), convertedSize.x, convertedSize.y, 4, m_pixels.data()))
            return true;
    }
    else if (extension == ".png")
    {
        // PNG format
        if (stbi_write_png(filename.to<std::string>().c_str(), convertedSize.x, convertedSize.y, 4, m_pixels.data(), 0))
            return true;
    }
    else if (extension == ".jpg" || extension == ".jpeg")
    {
        // JPG format
        if (stbi_write_jpg(filename.to<std::string>().c_str(), convertedSize.x, convertedSize.y, 4, m_pixels.data(), 90))
            return true;
    }
    else
    {
        priv::err() << "Image file extension " << extension << " not supported\n";
    }

    priv::err() << "Failed to save image\n" << priv::PathDebugFormatter{filename};
    return false;
}


////////////////////////////////////////////////////////////
std::vector<std::uint8_t> Image::saveToMemory(SaveFormat format) const
{
    // Make sure the image is not empty
    SFML_BASE_ASSERT(!m_pixels.empty() && m_size.x > 0 && m_size.y > 0);

    // Choose function based on format
    const auto convertedSize = m_size.to<Vector2i>();

    std::vector<std::uint8_t> buffer; // Use a single local variable for NRVO

    if (format == SaveFormat::BMP)
    {
        if (stbi_write_bmp_to_func(bufferFromCallback, &buffer, convertedSize.x, convertedSize.y, 4, m_pixels.data()))
            return buffer; // Non-empty
    }
    else if (format == SaveFormat::TGA)
    {
        if (stbi_write_tga_to_func(bufferFromCallback, &buffer, convertedSize.x, convertedSize.y, 4, m_pixels.data()))
            return buffer; // Non-empty
    }
    else if (format == SaveFormat::PNG)
    {
        if (stbi_write_png_to_func(bufferFromCallback, &buffer, convertedSize.x, convertedSize.y, 4, m_pixels.data(), 0))
            return buffer; // Non-empty
    }
    else if (format == SaveFormat::JPG)
    {
        if (stbi_write_jpg_to_func(bufferFromCallback, &buffer, convertedSize.x, convertedSize.y, 4, m_pixels.data(), 90))
            return buffer; // Non-empty
    }

    SFML_BASE_ASSERT(false);
    return buffer;
}


////////////////////////////////////////////////////////////
Vector2u Image::getSize() const
{
    return m_size;
}


////////////////////////////////////////////////////////////
void Image::createMaskFromColor(const Color& color, std::uint8_t alpha)
{
    // Make sure that the image is not empty
    SFML_BASE_ASSERT(!m_pixels.empty());

    // Replace the alpha of the pixels that match the transparent color
    std::uint8_t* ptr = m_pixels.data();
    std::uint8_t* end = ptr + m_pixels.size();

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
    SFML_BASE_ASSERT(source.m_size.x > 0 && source.m_size.y > 0 && m_size.x > 0 && m_size.y > 0);

    // Make sure the sourceRect components are non-negative before casting them to unsigned values
    if (sourceRect.position.x < 0 || sourceRect.position.y < 0 || sourceRect.size.x < 0 || sourceRect.size.y < 0)
        return false;

    auto srcRect = sourceRect.to<Rect<unsigned int>>();

    // Use the whole source image as srcRect if the provided source rectangle is empty
    if (srcRect.size.x == 0 || srcRect.size.y == 0)
    {
        srcRect = Rect<unsigned int>({0, 0}, source.m_size);
    }
    // Otherwise make sure the provided source rectangle fits into the source image
    else
    {
        // Checking the bottom right corner is enough because
        // left and top are non-negative and width and height are positive.
        if (source.m_size.x < srcRect.position.x + srcRect.size.x || source.m_size.y < srcRect.position.y + srcRect.size.y)
            return false;
    }

    // Make sure the destination position is within this image bounds
    if (m_size.x <= dest.x || m_size.y <= dest.y)
        return false;

    // Then find the valid size of the destination rectangle
    const Vector2u dstSize(base::min(m_size.x - dest.x, srcRect.size.x), base::min(m_size.y - dest.y, srcRect.size.y));

    // Precompute as much as possible
    const std::size_t  pitch     = static_cast<std::size_t>(dstSize.x) * 4;
    const unsigned int srcStride = source.m_size.x * 4;
    const unsigned int dstStride = m_size.x * 4;

    const std::uint8_t* srcPixels = source.m_pixels.data() + (srcRect.position.x + srcRect.position.y * source.m_size.x) * 4;
    std::uint8_t* dstPixels = m_pixels.data() + (dest.x + dest.y * m_size.x) * 4;

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
void Image::setPixel(Vector2u coords, const Color& color)
{
    SFML_BASE_ASSERT(coords.x < m_size.x && "Image::setPixel() x coordinate is out of bounds");
    SFML_BASE_ASSERT(coords.y < m_size.y && "Image::setPixel() y coordinate is out of bounds");

    const auto    index = (coords.x + coords.y * m_size.x) * 4;
    std::uint8_t* pixel = &m_pixels[index];

    *pixel++ = color.r;
    *pixel++ = color.g;
    *pixel++ = color.b;
    *pixel++ = color.a;
}


////////////////////////////////////////////////////////////
Color Image::getPixel(Vector2u coords) const
{
    SFML_BASE_ASSERT(coords.x < m_size.x && "Image::getPixel() x coordinate is out of bounds");
    SFML_BASE_ASSERT(coords.y < m_size.y && "Image::getPixel() y coordinate is out of bounds");

    const auto          index = (coords.x + coords.y * m_size.x) * 4;
    const std::uint8_t* pixel = &m_pixels[index];

    return {pixel[0], pixel[1], pixel[2], pixel[3]};
}


////////////////////////////////////////////////////////////
const std::uint8_t* Image::getPixelsPtr() const
{
    SFML_BASE_ASSERT(!m_pixels.empty());
    return m_pixels.data();
}


////////////////////////////////////////////////////////////
void Image::flipHorizontally()
{
    SFML_BASE_ASSERT(!m_pixels.empty());

    const std::size_t rowSize = m_size.x * 4;

    for (std::size_t y = 0; y < m_size.y; ++y)
    {
        auto left  = m_pixels.begin() + static_cast<decltype(m_pixels)::difference_type>(y * rowSize);
        auto right = m_pixels.begin() + static_cast<decltype(m_pixels)::difference_type>((y + 1) * rowSize - 4);

        for (std::size_t x = 0; x < m_size.x / 2; ++x)
        {
            std::swap_ranges(left, left + 4, right);

            left += 4;
            right -= 4;
        }
    }
}


////////////////////////////////////////////////////////////
void Image::flipVertically()
{
    SFML_BASE_ASSERT(!m_pixels.empty());

    const auto rowSize = static_cast<decltype(m_pixels)::difference_type>(m_size.x * 4);

    auto top    = m_pixels.begin();
    auto bottom = m_pixels.end() - rowSize;

    for (std::size_t y = 0; y < m_size.y / 2; ++y)
    {
        std::swap_ranges(top, top + rowSize, bottom);

        top += rowSize;
        bottom -= rowSize;
    }
}

} // namespace sf
