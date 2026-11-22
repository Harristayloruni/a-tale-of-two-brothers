// graphics_cmps.hpp - components that draw something
// labs 3 & 5

#pragma once
#include "ecm.hpp"
#include "renderer.hpp"
#include <SFML/Graphics.hpp>
#include <memory>
#include <string>

class ShapeComponent : public Component
{
public:
    ShapeComponent() = delete;
    explicit ShapeComponent(Entity *const p);
    void update(const float &dt) override;
    void render() override;
    sf::Shape &get_shape() const;
    void set_layer(int layer);

    // lab 5 - s->set_shape<sf::CircleShape>(12.f);
    template <typename T, typename... Targs>
    void set_shape(Targs... params)
    {
        _shape.reset(new T(params...));
    }

protected:
    std::shared_ptr<sf::Shape> _shape;
    int _layer = Renderer::ENTITIES;
};

// A sprite cut from a sprite sheet
// lab 3
class SpriteComponent : public Component
{
    public:
    SpriteComponent() = delete;
    // texture _path is relative to resouces
    SpriteComponent(Entity *const p, const std::string &texture_path,
                    const sf::IntRect &first_frame, int frame_count = 1,
                    float frame_time = 0.15f);

    void update(const float &dt) override;
    void render() override;
    sf::Sprite &get_sprite();
    void set_layer(int layer);
    void set_row(int row); // swith animation row in the sheet
    void set_scale(float scale); // pixel art is tiny (16px) - scale it up
    void set_flip_x(bool flip); // face left/right
    void set_frame(const sf::IntRect &rect); // show a different cut-out

    protected:
        std::shared_ptr<sf::Texture> _texture;
        sf::Sprite _sprite;
        sf::IntRect _first_frame;
        int _frame_count;
        int _frame = 0;
        float _frame_time;
        float _timer = 0.f;
        float _scale = 1.f;
        bool _flip = false;
        int _layer = Renderer::ENTITIES;
};

// A text label that follows its entity
class TextComponent : public Component
{
public:
    TextComponent() = delete;
    TextComponent(Entity *const p, const std::string &font_path, const std::string &str = "",
                  unsigned int size = 24);
    void update(const float &dt) override;
    void render() override;
    void set_text(const std::string &str);
    sf::Text &get_text();
    void set_offset(const sf::Vector2f &offset);
    void set_layer(int layer);

protected:
        std::shared_ptr<sf::Font> _font;
        sf::Text _text;
        sf::Vector2f _offset;
        int _layer = Renderer::UI;
};