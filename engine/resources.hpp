// resources.hpp - load each texture/font/sound once, then share it.
// prevents copies in memory causeing slow disk reads
// loads each file once and shares it
// Paths are relative to the resources/ folder next to the .exe

#pragma once
#include <SFML/Audio/SoundBuffer.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/Graphics/Texture.hpp>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>

class Resources
{
public:
    Resources() = delete;
    static constexpr const char *root = "resources/";

    // auto font = Resources::get<sf::Font>("fonts/RobotoMonoRegular.ttf");
    template <typename T>
    static std::shared_ptr<T> get(const std::string &path)
    {
        auto &cache = _cache<T>();
        auto it = chache.find(path);
        if (it != cache.end())
        {
            return it->second;
        }
        auto res = std::make_shared<T>();
        if (!res->loadFromFile(std::string(root) + path))
        {
            // missing file will throw error instead of drawing a white square
            throw std::runtime_error("Failed to load resource: " + std::string(root) + path);
        }
        cache[path] = res;
        return res;
    }
    // Free everything (call on shutdown)
    static void clear()
    {
        _cache<sf::Texture>().clear();
        _cache<sf::Font>().clear();
        _cache<sf::SoundBuffer>().clear();
    }

private:
    template <typename T>
    static std::map<std::string, std::shared_ptr<T>> &_cache()
    {
        static std::map<std::string, std::shared_ptr<T>> cache;
        return cache;
    }
};