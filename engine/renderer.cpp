// renderer.cpp - see renderer.hpp

#include "renderer.hpp"
#include <array>
#include <stdexcept>
#include <vector>

namespace
{
    // keeps capacity between frames, so there are no per-frame allocations
    std::array<std::vector<const sf::Drawable *>, Renderer::LAYER_COUNT> layers;
    sf::RenderWindow *rw = nullptr;
    unsigned int last_draw_count = 0;
} // namespace

void Renderer::initialise(sf::RenderWindow &r) 
{
    rw = &r;
}

sf::RenderWindow &Renderer::get_window()
{
    if (rw == nullptr)
    {
        throw std::runtime_error("No render window set");
    }
    return *rw;
}

void Renderer::shutdown()
{
    for (auto &len :layers)
    {
        len.clear();
    }
    rw = nullptr;
}

void Renderer::render()
{
    if (rw == nullptr)
    {
        throw std::runtime_error("No render window set");
    }
    last_draw_count = 0;
    for (auto &len : layers)
    {
        for (const sf::Drawable *d : len)
        {
            rw->draw(*d);
            ++last_draw_count;
        }
        len.clear();
    }
}

unsigned int Renderer::get_last_draw_count()
{
    return last_draw_count;
}