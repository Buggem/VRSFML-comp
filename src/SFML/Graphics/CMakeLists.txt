set(INCROOT ${PROJECT_SOURCE_DIR}/include/SFML/Graphics)
set(SRCROOT ${PROJECT_SOURCE_DIR}/src/SFML/Graphics)

# all source files
set(SRC
    ${SRCROOT}/BlendMode.cpp
    ${INCROOT}/BlendMode.hpp
    ${INCROOT}/Color.hpp
    ${INCROOT}/Color.inl
    ${INCROOT}/CoordinateType.hpp
    ${INCROOT}/Export.hpp
    ${SRCROOT}/Font.cpp
    ${INCROOT}/Font.hpp
    ${SRCROOT}/Glsl.cpp
    ${INCROOT}/Glsl.hpp
    ${SRCROOT}/GraphicsContext.cpp
    ${INCROOT}/GraphicsContext.hpp
    ${INCROOT}/Glsl.inl
    ${INCROOT}/Glyph.hpp
    ${SRCROOT}/Image.cpp
    ${INCROOT}/Image.hpp
    ${SRCROOT}/ImageUtils.cpp
    ${INCROOT}/ImageUtils.hpp
    ${INCROOT}/PrimitiveType.hpp
    ${SRCROOT}/RenderStates.cpp
    ${INCROOT}/RenderStates.hpp
    ${SRCROOT}/RenderTexture.cpp
    ${INCROOT}/RenderTexture.hpp
    ${SRCROOT}/RenderTarget.cpp
    ${INCROOT}/RenderTarget.hpp
    ${SRCROOT}/RenderWindow.cpp
    ${INCROOT}/RenderWindow.hpp
    ${SRCROOT}/Shader.cpp
    ${INCROOT}/Shader.hpp
    ${SRCROOT}/StencilMode.cpp
    ${INCROOT}/StencilMode.hpp
    ${SRCROOT}/Texture.cpp
    ${INCROOT}/Texture.hpp
    ${SRCROOT}/TextureAtlas.cpp
    ${INCROOT}/TextureAtlas.hpp
    ${SRCROOT}/TextureSaver.cpp
    ${SRCROOT}/TextureSaver.hpp
    ${SRCROOT}/Transform.cpp
    ${INCROOT}/Transform.hpp
    ${INCROOT}/Transform.inl
    ${SRCROOT}/Transformable.cpp
    ${INCROOT}/Transformable.hpp
    ${SRCROOT}/View.cpp
    ${INCROOT}/View.hpp
    ${INCROOT}/Vertex.hpp
)
source_group("" FILES ${SRC})

# drawables sources
set(DRAWABLES_SRC
    ${SRCROOT}/Shape.cpp
    ${INCROOT}/Shape.hpp
    ${SRCROOT}/CircleShape.cpp
    ${INCROOT}/CircleShape.hpp
    ${SRCROOT}/RectangleShape.cpp
    ${INCROOT}/RectangleShape.hpp
    ${SRCROOT}/ConvexShape.cpp
    ${INCROOT}/ConvexShape.hpp
    ${SRCROOT}/Sprite.cpp
    ${INCROOT}/Sprite.hpp
    ${SRCROOT}/Text.cpp
    ${INCROOT}/Text.hpp
    ${SRCROOT}/VertexBuffer.cpp
    ${INCROOT}/VertexBuffer.hpp
)
source_group("drawables" FILES ${DRAWABLES_SRC})

# render-texture sources
set(RENDER_TEXTURE_SRC
    ${SRCROOT}/RenderTextureImplFBO.cpp
    ${SRCROOT}/RenderTextureImplFBO.cpp
    ${SRCROOT}/RenderTextureImplFBO.hpp
    ${SRCROOT}/RenderTextureImplDefault.cpp
    ${SRCROOT}/RenderTextureImplDefault.hpp
)
source_group("render texture" FILES ${RENDER_TEXTURE_SRC})


# define the sfml-graphics target
sfml_add_library(Graphics
                 SOURCES ${SRC} ${DRAWABLES_SRC} ${RENDER_TEXTURE_SRC})

# setup dependencies
target_link_libraries(sfml-graphics PUBLIC SFML::Window)

# stb_image sources
target_include_directories(sfml-graphics SYSTEM PRIVATE "${PROJECT_SOURCE_DIR}/extlibs/headers/stb_image")

# glad sources
target_include_directories(sfml-graphics SYSTEM PRIVATE "${PROJECT_SOURCE_DIR}/extlibs/headers/glad/include")

# find external libraries
if(SFML_OS_ANDROID)
    target_link_libraries(sfml-graphics PRIVATE z)
elseif(SFML_OS_IOS)
    target_link_libraries(sfml-graphics PRIVATE z bz2)
endif()

if(SFML_USE_SYSTEM_DEPS)
    find_package(Freetype REQUIRED)
else()
    # use an immediately invoked function to scope option variables we have to set
    function(sfml_add_graphics_dependencies)
        # remember whether we are building SFML as a shared library
        if(BUILD_SHARED_LIBS)
            set(SFML_BUILD_SHARED_LIBS ON)
        endif()

        set(CMAKE_POLICY_DEFAULT_CMP0077 NEW)
        set(BUILD_SHARED_LIBS OFF)
        set(FT_DISABLE_ZLIB ON)
        set(FT_DISABLE_BZIP2 ON)
        set(FT_DISABLE_PNG ON)
        set(FT_DISABLE_HARFBUZZ ON)
        set(FT_DISABLE_BROTLI ON)
        set(FT_ENABLE_ERROR_STRINGS ON)

        CPMAddPackage(
            NAME freetype
            GITHUB_REPOSITORY freetype/freetype
            GIT_TAG VER-2-13-2
            SYSTEM TRUE
        )

        set_target_properties(freetype PROPERTIES FOLDER "Dependencies")

        # add flags required for emscripten support (e.g. pthread)
        if(SFML_OS_EMSCRIPTEN)
            target_compile_options(freetype PRIVATE ${SFML_EMSCRIPTEN_TARGET_COMPILE_OPTIONS})
            target_link_options(freetype PRIVATE ${SFML_EMSCRIPTEN_TARGET_LINK_OPTIONS})
        endif()

        # if building SFML as a shared library and linking our dependencies in
        # as static libraries we need to build them with -fPIC
        if(SFML_BUILD_SHARED_LIBS)
            set_target_properties(freetype PROPERTIES POSITION_INDEPENDENT_CODE ON)
        endif()

        # disable building dependencies as part of a unity build, they don't support it
        set_target_properties(freetype PROPERTIES UNITY_BUILD OFF)

        sfml_set_stdlib(freetype)
        add_library(Freetype::Freetype ALIAS freetype)
    endfunction()
    sfml_add_graphics_dependencies()
endif()

target_link_libraries(sfml-graphics PRIVATE Freetype::Freetype)

# add preprocessor symbols
target_compile_definitions(sfml-graphics PRIVATE "STBI_FAILURE_USERMSG")

# Image.cpp must be compiled with the -fno-strict-aliasing
# when gcc is used; otherwise saving PNGs may crash in stb_image_write
if(SFML_COMPILER_GCC)
    set_source_files_properties(${SRCROOT}/Image.cpp PROPERTIES COMPILE_FLAGS -fno-strict-aliasing)
endif()
