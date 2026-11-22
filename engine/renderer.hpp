// renderer.hpp - things are queued during render() and drawn all at once
// lab 5
// layers provide the draw order (HUD is always drawn on top o the world)

#pragma once
#include <SFML/Graphics.hpp>

namespace Renderer
{
    enum Layer 
    {
        BACKGROUND = 0, WORLD, ENTITIES, EFFECTS, UI, OVERLAY, LAYER_COUNT
    };

    void initialise(sf::RenderWindow &window);
    sf::RenderWindow &get_window();
    void shutdown();
    void update(const float &dt);
    void queue(const sf::Drawable *d, int layer = ENTITIES);
    void render();
    unsigned int get_last_draw_count(); // draw calls last frame (performance evidence)

} // namespace Renderer

