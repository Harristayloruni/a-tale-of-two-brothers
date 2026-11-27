// engine.cpp - see engine.hpp
#include "engine.hpp"
#include "audio.hpp"
#include "input.hpp"
#include "renderer.hpp"
#include "resources.hpp"
#include "settings.hpp"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <typeinfo>

// -----------Scene-----------
Scene::~Scene() = default;

void Scene::unload()
{
    _entities.list.clear(); // destroying entities destroys their components too
    _loaded = false;
}

void Scene::update(const float &dt)
{
    _entities.update(dt);
}
void Scene::render()
{
    _entities.render();
}

std::shared_ptr<Entity> Scene::make_entity()
{
    auto e = std::make_shared<Entity>(this);
    _entities.list.push_back(e);
    return e;
}

EntityManager &Scene::get_entities()
{
    return _entities;
}
bool Scene::is_loaded() const
{
    return _loaded;
}

//----------GameSystem--------------
std::shared_ptr<Scene> GameSystem::_active_scene;
std::shared_ptr<Scene> GameSystem::_next_scene;
std::unique_ptr<sf::RenderWindow> GameSystem::_window;
sf::Vector2f GameSystem::_logical_size(1920.f, 1080.f);
std::string GameSystem::_name;
bool GameSystem::_physics_enabled = false;
bool GameSystem::_quit = false;
bool GameSystem::_show_debug = false;
float GameSystem::_fps = 0.f;
std::string GameSystem::_debug_font_path;

void GameSystem::start(unsigned int logical_width, unsigned int logical_height,
                       const std::string &name, bool physics_enabled)
{
    _logical_size =
        {
            static_cast<float>(logical_width), static_cast<float>(logical_height)};
    _name = name;
    _physics_enabled = physics_enabled;
    _window = std::make_unique<sf::RenderWindow>();
    _create_window();
    _init();

    // if the environment variable TTB_DEBUG is set, print every scene change (for debugging)
    const bool debug_log = std::getenv("TTB_DEBUG") != nullptr;

    sf::Clock clock;

    // fps_timer = how long we have been counting, fps_frames = how many frames in that time
    float fps_timer = 0.f;
    int fps_frames = 0;

    while (_window->isOpen() && !_quit)
    {
        // 1. delta time - secounds since the last frame
        float dt = clock.restart().asSeconds();
        dt = std::min(dt, 0.1f);

        // count this frame, and every half second work out the frames per second
        fps_timer += dt;
        ++fps_frames;
        if (fps_timer >= 0.5f)
        {
            _fps = static_cast<float>(fps_frames) / fps_timer;
            fps_timer = 0.f;
            fps_frames = 0;
        }

        // 2. switch scene if one was requested last frame
        if (_next_scene)
        {
            if (debug_log)
                std::cout << "[engine] scene -> " << typeid(*_next_scene).name() << std::endl;
            if (_active_scene)
                _active_scene->on_exit();
            _active_scene = _next_scene;
            _next_scene = nullptr;
            if (!_active_scene->is_loaded())
            {
                _active_scene->load();
                _active_scene->_loaded = true;
            }
            _active_scene->on_enter();
        }

        // 3. events - every event is also handed to the Input system

        // tell Input a new frame is starting
        Input::begin_frame();

        sf::Event event;
        while (_window->pollEvent(event))
        {
            switch (event.type)
            {
            // the window's close button was clicked
            case sf::Event::Closed:
                _quit = true;
                break;

            // the window changed size, so refit the game into it
            case sf::Event::Resized:
                _update_view();
                break;

            // the player clicked away from the game, so Input ignores keys
            case sf::Event::LostFocus:
                Input::set_focus(false);
                break;

            // the player came back to the game
            case sf::Event::GainedFocus:
                Input::set_focus(true);
                break;

            // a key was pressed, if it was F3 turn the overlay on or off
            case sf::Event::KeyPressed:
                if (event.key.code == sf::Keyboard::F3)
                    _show_debug = !_show_debug;
                break;

            default:
                break;
            }

            // hand the event to Input
            Input::handle_event(event);
        }

        // tell Input that is all the events for this frame
        Input::end_events();

        // 4. Update, then render
        _window->clear(sf::Color::Black);
        _update(dt);
        _render();
        _window->display();
    }
    _clean();
}

void GameSystem::_init()
{
    Renderer::initialise(*_window);
}

void GameSystem::_update(const float &dt)
{
    if (!_active_scene)
        return;
    _active_scene->update(dt);
    Renderer::update(dt);
}

void GameSystem::_render()
{
    if (_active_scene)
        _active_scene->render();
    Renderer::render();

    // F3 overlay - FPS, draw calls and entity count (evidence of performance analysis)
    // only shows if F3 is on and a font has been set
    if (_show_debug && !_debug_font_path.empty())
    {
        static sf::Text text;

        // try to load the font, if it is missing just don't show the overlay
        try
        {
            text.setFont(*Resources::get<sf::Font>(_debug_font_path));
        }
        catch (...)
        {
            return;
        }

        // build the line of text
        std::ostringstream ss;
        ss << "FPS " << static_cast<int>(std::round(_fps)) << "   draws "
           << Renderer::get_last_draw_count() << "   entities "
           << (_active_scene ? _active_scene->get_entities().list.size() : 0);

        // draw it in yellow at the bottom left of the screen
        text.setString(ss.str());
        text.setCharacterSize(22);
        text.setFillColor(sf::Color::Yellow);
        text.setPosition(8.f, _logical_size.y - 32.f);
        _window->draw(text);
    }
}

void GameSystem::_clean()
{
    if (_active_scene)
    {
        _active_scene->on_exit();
        _active_scene->unload();
    }
    _active_scene = nullptr;
    _next_scene = nullptr;
    Renderer::shutdown();
    if (_window && _window->isOpen())
        _window->close();
    _window.reset();
}

void GameSystem::set_active_scene(const std::shared_ptr<Scene> &scene)
{
    _next_scene = scene;
}
std::shared_ptr<Scene> GameSystem::get_active_scene()
{
    return _active_scene;
}
void GameSystem::quit()
{
    _quit = true;
}
sf::Vector2f GameSystem::get_logical_size()
{
    return _logical_size;
}

// gives back the frames per second
float GameSystem::get_fps()
{
    return _fps;
}

// stores which font the F3 overlay should use
void GameSystem::set_debug_font(const std::string &path)
{
    _debug_font_path = path;
}

sf::Vector2f GameSystem::get_mouse_position()
{
    if (!_window || !_window->isOpen())
        return {0.f, 0.f};
    return _window->mapPixelToCoords(sf::Mouse::getPosition(*_window));
}

// rebuilds the window from Settings
// if the window exists, make it again with the current settings
void GameSystem::apply_video_settings()
{
    if (_window)
        _create_window();
}

// makes the window using what is stored in Settings
void GameSystem::_create_window()
{
    // mode = windowed, fullscreen or borderless
    const WindowMode mode = Settings::get_window_mode();

    // video = the width and height from Settings
    sf::VideoMode video(Settings::get_resolution().x, Settings::get_resolution().y);

    // style = a normal window with a title bar, close button and resizing
    sf::Uint32 style = sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize;

    // if it is fullscreen
    if (mode == WindowMode::Fullscreen)
    {
        // fullscreen needs a size the monitor supports, if ours is not one use the desktop size
        if (!video.isValid())
            video = sf::VideoMode::getDesktopMode();
        style = sf::Style::Fullscreen;
    }
    // if it is borderless, use the desktop size with no frame
    // it looks fullscreen but alt-tabs instantly
    else if (mode == WindowMode::Borderless)
    {
        video = sf::VideoMode::getDesktopMode();
        style = sf::Style::None;
    }

    // make the window (this reuses the same window object, so the Renderer still works)
    _window->create(video, _name, style);

    // if it is borderless, put it in the top left corner
    if (mode == WindowMode::Borderless)
        _window->setPosition({0, 0});

    // one KeyPressed per press, holding a key does not repeat it (menus, rebinding)
    _window->setKeyRepeatEnabled(false);

    // vsync on = no frame limit, vsync off = limit to 60 fps
    // never use both together (SFML docs)
    const bool vsync = Settings::get_vsync();
    _window->setVerticalSyncEnabled(vsync);
    _window->setFramerateLimit(vsync ? 0 : 60);

    _update_view();
}

// Draw the game at a fixed size and scale it into the window
void GameSystem::_update_view()
{
    sf::View view(sf::FloatRect(0.f, 0.f, _logical_size.x, _logical_size.y));
    view.setViewport(_calculate_viewport(_window->getSize(), _logical_size));
    _window->setView(view);
}

sf::FloatRect GameSystem::_calculate_viewport(const sf::Vector2u &screen, const sf::Vector2f &game)
{
    const float screen_aspect = static_cast<float>(screen.x) / static_cast<float>(std::max(1u, screen.y));
    const float game_aspect = game.x / game.y;
    float w = 1.f;
    float h = 1.f;
    if (screen_aspect > game_aspect)
    {
        w = game_aspect / screen_aspect; // window too wide: bars left and right
    }
    else
    {
        h = screen_aspect / game_aspect; // window too tall: bars top and bottom
    }
    return {(1.f - w) * 0.5f, (1.f - h) * 0.5f, w, h};
}
