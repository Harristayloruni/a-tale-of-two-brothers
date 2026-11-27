// chapter 4 checkpoint - actions instead of keys, saved settings, sound
#include "audio.hpp"
#include "engine.hpp"
#include "graphics_cmps.hpp"
#include "input.hpp"
#include "resources.hpp"
#include "settings.hpp"

// moves its entity using the Input system (keys, D-pad or stick)
class InputMoveComponent : public Component
{
public:
    InputMoveComponent(Entity *p, float speed) : Component(p), _speed(speed) {}

    void update(const float &dt) override
    {
        // move the wizard in the direction the player is pushing
        _parent->set_position(_parent->get_position() + Input::get_move_vector() * _speed * dt);

        // if Interact was pressed this frame, play the hit sound once
        if (Input::was_pressed(Action::Interact))
            Audio::play_sound("sounds/hit.wav");

        // if Menu was pressed this frame, close the game
        if (Input::was_pressed(Action::Menu))
            GameSystem::quit();
    }
    void render() override {}

private:
    float _speed;
};

class TestScene : public Scene
{
public:
    void load() override
    {
        auto wizard = make_entity();
        wizard->set_position({960.f, 540.f});
        auto sprite = wizard->add_component<SpriteComponent>("img/tilemap_packed.png", sf::IntRect(0, 112, 16, 16));
        sprite->set_scale(4.f);
        wizard->add_component<InputMoveComponent>(320.f);
    }

    // start the music every time this scene becomes active
    void on_enter() override
    {
        Audio::play_music("sounds/music.wav");
    }
};

// the order matters - Settings loads first because Input and Audio read from it
// saving happens after start() returns, which is when the player has quit
int main()
{
    Settings::load();
    Input::initialise();
    Audio::initialise();
    GameSystem::set_debug_font("fonts/RobotoMono-Regular.ttf"); // F3 overlay
    GameSystem::set_active_scene(std::make_shared<TestScene>());
    GameSystem::start(1920, 1080, "A Tale of Two Brothers", false);
    Input::save_to_settings();
    Settings::save();
    Audio::shutdown();
    Resources::clear();
    return 0;
}
