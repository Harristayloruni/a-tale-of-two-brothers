// scene with a wizard you can move.
#include "engine.hpp"
#include "graphics_cmps.hpp"
#include "math.hpp"

// Temporary component - moves its entity with WASD 
class KeyboardMoveComponent : public Component {
public:
  KeyboardMoveComponent(Entity *p, float speed) : Component(p), _speed(speed) {}
  void update(const float &dt) override {
    sf::Vector2f dir(0.f, 0.f);
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) dir.x -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) dir.x += 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) dir.y -= 1.f;
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) dir.y += 1.f;
    _parent->set_position(_parent->get_position() + math::normalize(dir) * _speed * dt);
  }
  void render() override {}

private:
  float _speed;
};

class TestScene : public Scene {
public:
  void load() override {
    auto wizard = make_entity();
    wizard->set_position({960.f, 540.f});
    auto sprite = wizard->add_component<SpriteComponent>("img/tilemap_packed.png", sf::IntRect(0, 112, 16, 16));
    sprite->set_scale(4.f);
    wizard->add_component<KeyboardMoveComponent>(320.f);
    auto label = wizard->add_component<TextComponent>("fonts/RobotoMono-Regular.ttf", "Wizard", 22);
    label->set_offset({0.f, -50.f});
  }
};

int main() {
  GameSystem::set_active_scene(std::make_shared<TestScene>());
  GameSystem::start(1920, 1080, "A Tale of Two Brothers", false);
  return 0;
}