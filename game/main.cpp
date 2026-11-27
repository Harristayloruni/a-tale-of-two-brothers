// chapter 5 checkpoint - a room loaded from a text file, drawn with the
// Kenney tileset, the wizard can't walk into walls, lab 5
#include "audio.hpp"
#include "engine.hpp"
#include "graphics_cmps.hpp"
#include "input.hpp"
#include "level_system.hpp"
#include "renderer.hpp"
#include "resources.hpp"
#include "settings.hpp"

// lets us write ls instead of LevelSystem
using ls = LevelSystem;

// turns a tile number into its square in the tileset image
// the image is 12 tiles wide and each tile is 16 by 16 pixels
sf::IntRect tile(int index)
{
    return {(index % 12) * 16, (index / 12) * 16, 16, 16};
}

// moves its entity with the Input system, but not into walls
class TileMoveComponent : public Component
{
public:
    TileMoveComponent(Entity *p, float speed) : Component(p), _speed(speed) {}

    void update(const float &dt) override
    {
        // next = where the wizard would be after moving
        const sf::Vector2f next = _parent->get_position() + Input::get_move_vector() * _speed * dt;

        // if that spot is not a wall, move there, lab 5
        if (ls::get_tile_at(next) != ls::WALL)
            _parent->set_position(next);

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
        // move the room down to leave space for a HUD at the top
        ls::set_offset({0.f, 120.f});

        // the image the tile pictures are cut from
        ls::set_texture(Resources::get<sf::Texture>("img/tilemap_packed.png").get());

        // every tile type you can walk on is drawn as floor
        // tile 48 is listed three times and 49 once, so plain sand is three times as likely
        for (auto t : {ls::EMPTY, ls::START, ls::ENEMY, ls::BOSS, ls::CHEST, ls::NPC, ls::WAYPOINT})
        {
            ls::set_tile_rects(t, {tile(48), tile(48), tile(48), tile(49)});
        }

        // tile 40 is the stone wall, tile 33 is the open door
        ls::set_tile_rects(ls::WALL, {tile(40)});
        ls::set_tile_rects(ls::END, {tile(33)});

        // load the room, each tile is 64 pixels wide
        ls::load_level("resources/levels/level1_room1.txt", 64.f);

        // make the wizard (tile 84) and put him on the start tile
        auto wizard = make_entity();
        wizard->set_position(ls::get_start_position());
        wizard->add_component<SpriteComponent>("img/tilemap_packed.png", tile(84))->set_scale(4.f);
        wizard->add_component<TileMoveComponent>(320.f);
    }

    void render() override
    {
        // draw the room first
        ls::render(Renderer::get_window());

        // then the entities on top
        Scene::render();
    }
};

// the order matters - Settings loads first because Input and Audio read from it
int main()
{
    Settings::load();
    Input::initialise();
    Audio::initialise();
    GameSystem::set_debug_font("fonts/RobotoMono-Regular.ttf"); // F3 overlay
    GameSystem::set_active_scene(std::make_shared<TestScene>());
    GameSystem::start(1920, 1080, "A Tale of Two Brothers", false);
    Settings::save();
    Audio::shutdown();
    Resources::clear();
    return 0;
}
