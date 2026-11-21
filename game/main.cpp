// #include <SFML/Graphics.hpp>

// int main() {
//   sf::RenderWindow window(sf::VideoMode(800, 600), "A Tale of Two Brothers");
//   sf::CircleShape shape(100.f);
//   shape.setFillColor(sf::Color::Green);

//   while (window.isOpen()) {
//     sf::Event event;
//     while (window.pollEvent(event)) {
//       if (event.type == sf::Event::Closed) {
//         window.close();
//       }
//     }
//     window.clear();
//     window.draw(shape);
//     window.display();
//   }
//   return 0;
// }

// Sandbox to test everything runs
// makes and entity with two components and runs three fake frames

// ch.2 - test ECM in the console
#include "ecm.hpp"
#include <iostream>

// component that counts how many times it has been updated
class CounterComponent : public Component
{
  public:
    explicit CounterComponent(Entity *p) : Component(p) {}
    void update(const float &dt) override
    {
      ++count;
      std::cout << "  counter update " << count << " (dt " << dt << ")\n";
    }
    void render() override {}
    int count = 0;
};

// A component that moves its entity to the right at 100 pixels per second
class MoveRightComponent : public Component
{
public:
  explicit MoveRightComponent(Entity *p) : Component(p) {}
  void update(const float &dt) override
  {
    _parent->set_position(_parent->get_position() + sf::Vector2f(100.f * dt, 0.f));
  }
  void render() override {}
};

int main()
{
  EntityManager em;
  auto wizard = std::make_shared<Entity>();
  wizard->add_tag("player");
  wizard->add_component<CounterComponent>();
  wizard->add_component<MoveRightComponent>();
  em.list.push_back(wizard);

  for (int frame = 0; frame < 3; ++frame)
  {
    std::cout << "frame " << frame << "\n";
    em.update(0.5f);
    std::cout << " wizard x = " << wizard->get_position().x << "\n";
  }

  std::cout << "counters found: " << wizard->get_components<CounterComponent>().size() << "\n";
  std::cout << "players found: " << em.find("player").size() << "\n";
  wizard->set_for_delete();
  em.update(0.f);
  std::cout << "entities left: " << em.list.size() << "\n";
  return 0;
}