// engine.cpp - see engine.hpp
#include "engine.hpp"
#include "renderer.hpp"
#include <algorithm>

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
    Renderer::initialise(*_window);

    sf::Clock clock;
    while (_window->isOpen() && !_quit)
    {
        // 1. delta time - secounds since the last frame
        float dt = clock.restart().asSeconds();
        dt = std::min(dt, 0.1f);

        // 2. switch scene if one was requested last frame
        if (_next_scene)
        {
            if (_active_scene)
                _active_scene->on_exit();
            _active_scene = nullptr;
            if (!_active_scene->is_loaded())
            {
                _active_scene->load();
                _active_scene->_loaded = true;
            }
            _active_scene->on_enter();
        }

        // 3. events
        sf::Event event;
        while (_window->pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                _quit = true;
            if (event.type == sf::Event::Resized)
                _update_view();
        }

        // 4. Update, then render
        _window->clear(sf::Color::Black);
        _update(dt);
        _render();
        _window->display();
    }
    _clean();
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

sf::Vector2f GameSystem::get_mouse_position()
{
    if (!_window || !_window->isOpen())
        return {0.f, 0.f};
    return _window->mapPixelToCoords(sf::Mouse::getPosition(*_window));
}

void GameSystem::_create_window()
{
    _window->create(sf::VideoMode(1280, 720), _name,
                    sf::Style::Titlebar | sf::Style::Close | sf::Style::Resize);
    _window->setKeyRepeatEnabled(false);
    _window->setVerticalSyncEnabled(true);
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