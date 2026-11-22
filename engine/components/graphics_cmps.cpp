// graphics_cmps.cpp - see graphics_cmps.hpp
#include "graphics_cmps.hpp"
#include "resources.hpp"

// -------ShapeComponent------------
ShapeComponent::ShapeComponent(Entity *const p)
    : Component(p), _shape(std::make_shared<sf::CircleShape>()) {}

void ShapeComponent::update(const float &) {}

void ShapeComponent::render()
{
    // Sync to the entity when drawing, so paused entites still draw correctly
    _shape->setPosition(_parent->get_position());
    _shape->setRotation(_parent->get_rotation());
    Renderer::queue(_shape.get(), _layer);
}
sf::Shape &ShapeComponent::get_shape() const
{
    return *_shape;
}
void ShapeComponent::set_layer(int layer)
{
    _layer = layer;
}

// -----------SpriteComponent---------------
SpriteComponent::SpriteComponent(Entity *const p, const std::string &texture_path,
                                 const sf::IntRect &first_frame, int frame_count,
                                 float frame_time)
    : Component(p), _texture(Resources::get<sf::Texture>(texture_path)),
      _first_frame(first_frame), _frame_count(frame_count < 1 ? 1 : frame_count),
      _frame_time(frame_time)
{
    _sprite.setTexture(*_texture);
    _sprite.setTextureRect(_first_frame);
    // Center the origin so the position is in the middle of the sprite
    _sprite.setOrigin(first_frame.width / 2.f, first_frame.height / 2.f);
}

void SpriteComponent::update(const float &dt)
{
    if (_frame_count > 1)
    {
        _timer += dt;
        if (_timer >= _frame_time)
        {
            _timer -= _frame_time;
            _frame = (_frame + 1) % _frame_count;
            sf::IntRect r = _first_frame;
            r.left += _frame * _first_frame.width; // move the "cut" right (lab 3)
            _sprite.setTextureRect(r);
        }
    }
}

void SpriteComponent::render()
{
    _sprite.setPosition(_parent->get_position());
    _sprite.setRotation(_parent->get_rotation());
    Renderer::queue(&_sprite, _layer);
}
sf::Sprite &SpriteComponent::get_sprite()
{
    return _sprite;
}
void SpriteComponent::set_layer(int layer)
{
    _layer = layer;
}
void SpriteComponent::set_scale(float scale)
{
    _scale = scale;
    _sprite.setScale(_flip ? -_scale : _scale, _scale);
}
void SpriteComponent::set_flip_x(bool flip)
{
    _flip = flip;
    _sprite.setScale(_flip ? -_scale : _scale, _scale);
}
void SpriteComponent::set_frame(const sf::IntRect &rect)
{
    _first_frame = rect;
    _frame = 0;
    _sprite.setTextureRect(rect);
}
void SpriteComponent::set_row(int row)
{
    _first_frame.top = row * _first_frame.height;
    _frame = 0;
    _sprite.setTextureRect(_first_frame);
}

// --------TextComponent--------------------
TextComponent::TextComponent(Entity *const p, const std::string &font_path,
                             const std::string &str, unsigned int size)
    : Component(p), _font(Resources::get<sf::Font>(font_path))
{
    _text.setFont(*_font);
    _text.setCharacterSize(size);
    set_text(str);
}
void TextComponent::update(const float &) {}

void TextComponent::render()
{
    _text.setPosition(_parent->get_position() + _offset);
    Renderer::queue(&_text, _layer);
}
void TextComponent::set_text(const std::string &str)
{
    _text.setString(str);
    const sf::FloatRect b = _text.getLocalBounds();
    _text.setOrigin(b.left + b.width / 2.f, b.top + b.height / 2.f); // centered
}

sf::Text &TextComponent::get_text()
{
    return _text;
}
void TextComponent::set_offset(const sf::Vector2f &offset)
{
    _offset = offset;
}
void TextComponent::set_layer(int layer)
{
    _layer = layer;
}
