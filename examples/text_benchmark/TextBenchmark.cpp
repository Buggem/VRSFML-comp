
#include <GL/gl.h>
#if 1

#include "imgui.h" // necessary for ImGui::*, imgui-SFML.h doesn't include imgui.h

#include <SFML/ImGui/ImGui.hpp>

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/GraphicsContext.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include <SFML/Window/Event.hpp>
#include <SFML/Window/EventUtils.hpp>
#include <SFML/Window/GameLoop.hpp>
#include <SFML/Window/VideoMode.hpp>

#include <SFML/System/Clock.hpp>

#include <SFML/Base/Optional.hpp>

int main()
{
    sf::GraphicsContext     graphicsContext;
    sf::ImGui::ImGuiContext imGuiContext(graphicsContext);

#if 0
    sf::RenderWindow window(graphicsContext, sf::VideoMode({1280, 720}), "ImGui + SFML = <3");
    sf::base::Optional<sf::RenderWindow>
        childWindow(sf::base::inPlace, graphicsContext, sf::VideoMode({640, 480}), "ImGui-SFML Child window");

    window.clear();
    window.display();

    childWindow.reset();

    window.clear();
    window.display();

#else

#if 1
    sf::RenderWindow window(graphicsContext, sf::VideoMode({1280, 720}), "ImGui + SFML = <3");
    window.setFramerateLimit(60);
    if (!imGuiContext.init(window))
        return -1;

    sf::base::Optional<sf::RenderWindow>
        childWindow(sf::base::inPlace, graphicsContext, sf::VideoMode({640, 480}), "ImGui-SFML Child window");
    childWindow->setFramerateLimit(60);
    if (!imGuiContext.init(*childWindow))
        return -1;

    sf::Clock deltaClock;
    SFML_GAME_LOOP
    {
        // Main window event processing
        while (const auto event = window.pollEvent())
        {
            imGuiContext.processEvent(window, *event);

            if (sf::EventUtils::isClosedOrEscapeKeyPressed(*event))
                SFML_GAME_LOOP_BREAK;
        }

        // Update
        const sf::Time dt = deltaClock.restart();
        imGuiContext.update(window, dt);

        // Add ImGui widgets in the first window
        imGuiContext.setCurrentWindow(window);
        ImGui::Begin("Hello, world!");
        ImGui::Button("Look at this pretty button");
        ImGui::End();

        ImGui::ShowDemoWindow();

        // Main window drawing
        sf::CircleShape shape(100.f);
        shape.setFillColor(sf::Color::Green);

        window.clear();
        window.draw(shape, /* texture */ nullptr);
        imGuiContext.render(window);
        window.display();

        // Child window event processing
        if (childWindow.hasValue())
        {
            while (const auto event = childWindow->pollEvent())
            {
                imGuiContext.processEvent(*childWindow, *event);

                if (event->is<sf::Event::Closed>())
                {
                    imGuiContext.shutdown(*childWindow);
                    childWindow.reset();
                    SFML_GAME_LOOP_CONTINUE;
                }
            }

            imGuiContext.update(*childWindow, dt);

            imGuiContext.setCurrentWindow(*childWindow);
            ImGui::Begin("Works in a second window!");
            ImGui::Button("Example button");
            ImGui::End();

            sf::CircleShape shape2(50.f);
            shape2.setFillColor(sf::Color::Red);

            childWindow->clear();
            childWindow->draw(shape2, /* texture */ nullptr);
            imGuiContext.render(*childWindow);
            childWindow->display();
        }

        SFML_GAME_LOOP_CONTINUE;
    };
#else
    sf::RenderWindow window(graphicsContext, sf::VideoMode({640, 480}), "ImGui + SFML = <3");
    window.setFramerateLimit(60);

    if (!imGuiContext.init(window))
        return -1;

    sf::CircleShape shape(100.f);
    shape.setFillColor(sf::Color::Green);

    sf::Clock deltaClock;

    SFML_GAME_LOOP
    {
        while (const auto event = window.pollEvent())
        {
            imGuiContext.processEvent(window, *event);

            if (sf::EventUtils::isClosedOrEscapeKeyPressed(*event))
                SFML_GAME_LOOP_BREAK;
        }

        imGuiContext.update(window, deltaClock.restart());

        ImGui::ShowDemoWindow();

        ImGui::Begin("Hello, world!");
        ImGui::Button("Look at this pretty button");
        ImGui::End();

        window.clear();
        window.draw(shape, nullptr /* texture */);
        imGuiContext.render(window);
        window.display();

        SFML_GAME_LOOP_CONTINUE;
    };
#endif
#endif
}


#else


#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/PrimitiveType.hpp>
#include <SFML/Graphics/RenderStates.hpp>
#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/Vertex.hpp>

#include <SFML/Window/ContextSettings.hpp>
#include <SFML/Window/EventUtils.hpp>

#include <SFML/Base/Optional.hpp>
#if 1

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/GraphicsContext.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/RenderWindow.hpp>
#include <SFML/Graphics/Text.hpp>
#include <SFML/Graphics/Texture.hpp>

#include <SFML/Window/Event.hpp>
#include <SFML/Window/VideoMode.hpp>

#include <SFML/System/Path.hpp>
#include <SFML/System/String.hpp>

#include <array>
#include <sstream>
#include <vector>


////////////////////////////////////////////////////////////

// change this to 1 to trigger the bug
#define TRIGGER_THE_BUG 1

int main()
{
    const float screenWidth  = 800.0f;
    const float screenHeight = 600.0f;

    const sf::Vector2u screenSize{static_cast<unsigned int>(screenWidth), static_cast<unsigned int>(screenHeight)};

    sf::GraphicsContext graphicsContext;

    // TODO P0: aa level of 4 causes glcheck assert fail on opengl

    sf::RenderWindow window(graphicsContext,
                            sf::VideoMode(screenSize),
                            "Window",
                            sf::Style::Default,
                            sf::State::Windowed,
                            sf::ContextSettings(0, 0, 4));

    window.setVerticalSyncEnabled(true);

    auto image   = sf::Image::create(screenSize, sf::Color::White).value();
    auto texture = sf::Texture::loadFromImage(graphicsContext, image).value();

    auto baseRenderTexture = sf::RenderTexture::create(graphicsContext, screenSize, sf::ContextSettings{0, 0, 4 /* AA level*/})
                                 .value();

    sf::RenderTexture renderTextures
        [2]{sf::RenderTexture::create(graphicsContext, screenSize, sf::ContextSettings{0, 0, 4 /* AA level*/}).value(),
            sf::RenderTexture::create(graphicsContext, screenSize, sf::ContextSettings{0, 0, 4 /* AA level*/}).value()};

    std::vector<sf::Vertex> vertexArrays[2];

    while (true)
    {
        while (sf::base::Optional event = window.pollEvent())
        {
            if (sf::EventUtils::isClosedOrEscapeKeyPressed(*event))
                return EXIT_SUCCESS;
        }

        window.clear();

        vertexArrays[0].clear();
        vertexArrays[1].clear();

        float xCenter = screenWidth / 2;

        vertexArrays[0].emplace_back(sf::Vector2f{0, 0}, sf::Color::White, sf::Vector2f{0, 0});
        vertexArrays[0].emplace_back(sf::Vector2f{xCenter, 0}, sf::Color::White, sf::Vector2f{xCenter, 0});
        vertexArrays[0].emplace_back(sf::Vector2f{0, screenHeight}, sf::Color::White, sf::Vector2f{0, screenHeight});

        vertexArrays[0].emplace_back(sf::Vector2f{0, screenHeight}, sf::Color::White, sf::Vector2f{0, screenHeight});
        vertexArrays[0].emplace_back(sf::Vector2f{xCenter, 0}, sf::Color::White, sf::Vector2f{xCenter, 0});
        vertexArrays[0].emplace_back(sf::Vector2f{xCenter, screenHeight}, sf::Color::White, sf::Vector2f{xCenter, screenHeight});

        // right half of screen
        vertexArrays[1].emplace_back(sf::Vector2f{xCenter, 0}, sf::Color::White, sf::Vector2f{xCenter, 0});
        vertexArrays[1].emplace_back(sf::Vector2f{screenWidth, 0}, sf::Color::White, sf::Vector2f{screenWidth, 0});
        vertexArrays[1].emplace_back(sf::Vector2f{xCenter, screenHeight}, sf::Color::White, sf::Vector2f{xCenter, screenHeight});

        vertexArrays[1].emplace_back(sf::Vector2f{xCenter, screenHeight}, sf::Color::White, sf::Vector2f{xCenter, screenHeight});
        vertexArrays[1].emplace_back(sf::Vector2f{screenWidth, 0}, sf::Color::White, sf::Vector2f{screenWidth, 0});
        vertexArrays[1].emplace_back(sf::Vector2f{screenWidth, screenHeight},
                                     sf::Color::White,
                                     sf::Vector2f{screenWidth, screenHeight});

        renderTextures[0].clear();
        renderTextures[1].clear();

        sf::Sprite sprite(texture.getRect());
        renderTextures[0].draw(sprite, texture);

        sprite.setColor(sf::Color::Green);
        renderTextures[1].draw(sprite, texture);

        baseRenderTexture.clear();


        renderTextures[0].display();
        baseRenderTexture.draw(vertexArrays[0],
                               sf::PrimitiveType::Triangles,
                               sf::RenderStates{&renderTextures[0].getTexture()});

        renderTextures[1].display();
        baseRenderTexture.draw(vertexArrays[1],
                               sf::PrimitiveType::Triangles,
                               sf::RenderStates{&renderTextures[1].getTexture()});

        baseRenderTexture.display();

        window.draw(sf::Sprite(baseRenderTexture.getTexture().getRect()), baseRenderTexture.getTexture());

        window.display();
    }

    return 0;
}

#elif defined(BARABARAR)

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/GraphicsContext.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Text.hpp>

#include <SFML/System/Path.hpp>
#include <SFML/System/String.hpp>

#include <sstream>

int main()
{
    sf::GraphicsContext graphicsContext;

    sf::RenderWindow window(graphicsContext, sf::VideoMode({800u, 600u}), "Test", sf::Style::Titlebar | sf::Style::Close);
    window.setVerticalSyncEnabled(false);

    const auto font = sf::Font::openFromFile(graphicsContext, "resources/tuffy.ttf").value();

    sf::Text text(font, "Test", 20);

    sf::RenderTexture renderTexture[10]{sf::RenderTexture::create(graphicsContext, {800u, 600u}).value(),
                                        sf::RenderTexture::create(graphicsContext, {800u, 600u}).value(),
                                        sf::RenderTexture::create(graphicsContext, {800u, 600u}).value(),
                                        sf::RenderTexture::create(graphicsContext, {800u, 600u}).value(),
                                        sf::RenderTexture::create(graphicsContext, {800u, 600u}).value(),
                                        sf::RenderTexture::create(graphicsContext, {800u, 600u}).value(),
                                        sf::RenderTexture::create(graphicsContext, {800u, 600u}).value(),
                                        sf::RenderTexture::create(graphicsContext, {800u, 600u}).value(),
                                        sf::RenderTexture::create(graphicsContext, {800u, 600u}).value(),
                                        sf::RenderTexture::create(graphicsContext, {800u, 600u}).value()};

    ;

    sf::Clock          clock;
    std::ostringstream sstr;

    while (true)
    {
        while (sf::base::Optional event = window.pollEvent())
        {
            if (sf::EventUtils::isClosedOrEscapeKeyPressed(*event))
                return EXIT_SUCCESS;
        }

        for (int i = 0; i < 20; ++i)
        {
            for (auto& j : renderTexture)
            {
                j.clear(sf::Color(0, 0, 0));

                for (auto& u : renderTexture)
                    u.draw(text);

                j.display();
            }

            window.clear(sf::Color(0, 0, 0));

            for (const auto& j : renderTexture)
            {
                sf::Sprite sprite(j.getTexture().getRect());

                for (int k = 0; k < 10; ++k)
                    window.draw(sprite, j.getTexture());
            }
        }

        window.display();

        sstr.str("");
        sstr << "Test -- Frame: " << clock.restart().asSeconds() << " sec";

        window.setTitle(sstr.str());
    }

    return EXIT_SUCCESS;
}

#else

#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/GraphicsContext.hpp>
#include <SFML/Graphics/RenderTexture.hpp>
#include <SFML/Graphics/Text.hpp>

#include <SFML/System/Path.hpp>
#include <SFML/System/String.hpp>

#include <cstdlib>

int main()
{
    sf::GraphicsContext graphicsContext;

    const auto       font         = sf::Font::openFromFile(graphicsContext, "resources/tuffy.ttf").value();
    const sf::String textContents = "abcdefghilmnopqrstuvz\nabcdefghilmnopqrstuvz\nabcdefghilmnopqrstuvz\n";

    auto text          = sf::Text(font, textContents);
    auto renderTexture = sf::RenderTexture::create(graphicsContext, {1680, 1050}).value();

    renderTexture.clear();

    for (std::size_t i = 0; i < 100'000; ++i)
    {
        text.setOutlineThickness(static_cast<float>(5 + (i % 2)));
        renderTexture.draw(text);
    }

    renderTexture.display();
}

#endif
#endif
