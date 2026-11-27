// engine.hpp - GameSystem (Window + game loop) and the scene base class
// labs 2 (loop), 3 (static GameSystem), 4 (scenes), 5 (EntityManager), 7 (make_entity)
// added for the coursework: window mode / resolution changes while the game runs,
// input handed to the Input system, and an F3 performance overlay

#pragma once
#include "ecm.hpp"
#include <SFML/Graphics.hpp>
#include <memory>
#include <string>

class Scene
{
public:
    Scene() = default;
    virtual ~Scene();

    virtual void load() = 0; // create this sceme's entities
    virtual void unload();   // destroy them
    virtual void update(const float &dt);
    virtual void render();
    virtual void on_enter() {} // called every time the scene becomes active
    virtual void on_exit() {}  // called when another scene takes over

    std::shared_ptr<Entity> make_entity(); // create + add to this scene (lab 7)
    EntityManager &get_entities();
    bool is_loaded() const;

protected:
    EntityManager _entities;

private:
    friend class GameSystem; // only GameSystem may set _loaded
    bool _loaded = false;
};

class GameSystem
{
public:
    GameSystem() = delete; // static only - never create one

    static void start(unsigned int logical_width, unsigned int logical_height,
                      const std::string &name, bool physics_enabled);
    static void set_active_scene(const std::shared_ptr<Scene> &scene);
    static std::shared_ptr<Scene> get_active_scene();
    static void quit();

    // rebuild the window from Settings (resolution, window mode, vsync)
    // the settings menu calls this after the player changes a video setting
    static void apply_video_settings();

    static sf::Vector2f get_logical_size();
    static sf::Vector2f get_mouse_position();

    // how many frames per second the game is running at
    static float get_fps();

    // which font the F3 overlay uses, it will not show until this is set
    static void set_debug_font(const std::string &path);

private:
    static void _init();
    static void _update(const float &dt);
    static void _render();
    static void _clean();
    static void _update_view();
    static void _create_window();
    static sf::FloatRect _calculate_viewport(const sf::Vector2u &screen, const sf::Vector2f &game);

    static std::shared_ptr<Scene> _active_scene;
    static std::shared_ptr<Scene> _next_scene;
    static std::unique_ptr<sf::RenderWindow> _window;
    static sf::Vector2f _logical_size;
    static std::string _name;
    static bool _physics_enabled;
    static bool _quit;

    // is the F3 overlay showing?
    static bool _show_debug;

    // the frames per second, worked out twice a second
    static float _fps;

    // the font file for the F3 overlay
    static std::string _debug_font_path;
};
