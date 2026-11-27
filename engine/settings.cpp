// settings.cpp - see settings.hpp
// this is the code behind each tool listed in the header

#include "settings.hpp"
#include <SFML/Window/VideoMode.hpp>
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace fs = std::filesystem; // lets us write fs instead of std::filesystem

// this creates the list of settings in memory
// each one is a name and a value, both as text
std::map<std::string, std::string> Settings::_values;

// anything in here can only be used inside this file
namespace
{
    // the name of our settings folder
    const char *const app_folder = "TaleOfTwoBrothers";

    // cuts the spaces off both ends of some text
    // " 1920 " becomes "1920"
    std::string trim(const std::string &s)
    {
        // b = where the first real character is
        const auto b = s.find_first_not_of(" \t\r\n");
        // if b was not found, the text is only spaces, so give back nothing
        if (b == std::string::npos)
            return "";
        // e = where the last real character is
        const auto e = s.find_last_not_of(" \t\r\n");
        // give back only the part from b to e
        return s.substr(b, e - b + 1);
    }
} // namespace

// File
// works out where the settings file is on this computer
std::string Settings::get_file_path()
{
    // dir = the folder the file will go in, empty for now
    fs::path dir;
#if defined(_WIN32)
    // if this is Windows, use the AppData folder
    if (const char *appdata = std::getenv("APPDATA"))
        dir = fs::path(appdata) / app_folder;
#elif defined(__APPLE__)
    // if this is a Mac, use home / Library / Application Support
    if (const char *home = std::getenv("HOME"))
        dir = fs::path(home) / "Library" / "Application Support" / app_folder;
#else
    // otherwise it is Linux, use the .config folder
    if (const char *xdg = std::getenv("XDG_CONFIG_HOME"))
        dir = fs::path(xdg) / app_folder;
    else if (const char *home = std::getenv("HOME"))
        dir = fs::path(home) / ".config" / app_folder;
#endif
    // if no folder was found, just put the file next to the game
    if (dir.empty())
        return "settings.ini";
    // give back the folder with settings.ini on the end
    return (dir / "settings.ini").string();
}

// reads the settings file into memory
void Settings::load()
{
    // open the file
    std::ifstream f(get_file_path());
    // if the file did not open, there is no file yet, so stop here
    if (!f.good())
    {
        std::cout << "No settings file yet, using defaults (" << get_file_path() << ")\n";
        return;
    }
    std::string line;
    // keep reading one line at a time until the file ends
    while (std::getline(f, line))
    {
        // cut the spaces off the line
        line = trim(line);
        // if the line is empty or starts with #, skip it
        if (line.empty() || line[0] == '#')
            continue;
        // eq = where the = sign is
        const auto eq = line.find('=');
        // if there is no = sign, skip the line
        if (eq == std::string::npos)
            continue;
        // the part before the = is the name, the part after is the value
        // store them together in the list
        _values[trim(line.substr(0, eq))] = trim(line.substr(eq + 1));
    }
    std::cout << "Settings loaded from " << get_file_path() << "\n";
}

// writes the settings in memory back to the file
// gives back true if it worked, false if it did not
bool Settings::save()
{
    // path = where the file is
    const fs::path path(get_file_path());
    std::error_code ec;
    // if the folder does not exist yet, make it
    if (path.has_parent_path())
        fs::create_directories(path.parent_path(), ec);
    // open the file so we can write to it
    std::ofstream f(path);
    // if the file did not open, say so and give back false
    if (!f.good())
    {
        std::cerr << "Could not save settings to " << path.string() << "\n";
        return false;
    }
    // write a title line at the top
    f << "# A Tale of Two Brothers - user settings\n";
    // for every setting in the list, write a line like name=value
    for (const auto &kv : _values)
    {
        f << kv.first << '=' << kv.second << '\n';
    }
    // it worked
    return true;
}

// Getters and setters
// looks up a setting and gives it back as a whole number
// key = the name of the setting, fallback = the answer if something goes wrong
int Settings::get_int(const std::string &key, int fallback)
{
    // look for the name in the list
    auto it = _values.find(key);
    // if the name was not found, give back the fallback
    if (it == _values.end())
        return fallback;
    // try to turn the text into a whole number
    try
    {
        return std::stoi(it->second);
    }
    // if it could not be turned into a number (e.g. "abc"), give back the fallback
    catch (...)
    {
        return fallback;
    }
}

// same as get_int, but gives back a decimal number
float Settings::get_float(const std::string &key, float fallback)
{
    // look for the name in the list
    auto it = _values.find(key);
    // if the name was not found, give back the fallback
    if (it == _values.end())
        return fallback;
    // try to turn the text into a decimal number
    try
    {
        return std::stof(it->second);
    }
    // if it could not be turned into a number, give back the fallback
    catch (...)
    {
        return fallback;
    }
}

// looks up a setting and gives it back as text
std::string Settings::get_string(const std::string &key, const std::string &fallback)
{
    // look for the name in the list
    auto it = _values.find(key);
    // if it was not found give back the fallback, otherwise give back the value
    return it == _values.end() ? fallback : it->second;
}

// stores a whole number under a name (turned into text first)
void Settings::set_int(const std::string &key, int value)
{
    _values[key] = std::to_string(value);
}
// stores a decimal number under a name (turned into text first)
void Settings::set_float(const std::string &key, float value)
{
    _values[key] = std::to_string(value);
}
// stores text under a name
void Settings::set_string(const std::string &key, const std::string &value)
{
    _values[key] = value;
}

// Video
// gives back the width and height the window should be
sf::Vector2u Settings::get_resolution()
{
    // desk = the size of the player's monitor
    const auto desk = sf::VideoMode::getDesktopMode();

    // big = true if the monitor is at least 1920 by 1080
    const bool big = desk.width >= 1920 && desk.height >= 1080;

    // w = the saved width, or if none is saved, 1920 on a big monitor and 1280 on a small one
    const int w = get_int("video.width", big ? 1920 : 1280);

    // h = the saved height, or if none is saved, 1080 on a big monitor and 720 on a small one
    const int h = get_int("video.height", big ? 1080 : 720);

    // give back w and h, but never smaller than 320 by 240
    return {static_cast<unsigned>(std::max(320, w)), static_cast<unsigned>(std::max(240, h))};
}

// stores a width and height
void Settings::set_resolution(const sf::Vector2u &res)
{
    set_int("video.width", static_cast<int>(res.x));
    set_int("video.height", static_cast<int>(res.y));
}

// gives back windowed, fullscreen or borderless
WindowMode Settings::get_window_mode()
{
    // m = the saved number, or 0 if none is saved
    const int m = get_int("video.mode", 0);

    // if m is 0, 1 or 2 give back that window type, otherwise give back windowed
    return (m >= 0 && m <= 2) ? static_cast<WindowMode>(m) : WindowMode::Windowed;
}

// stores the window type as its number
void Settings::set_window_mode(WindowMode mode)
{
    set_int("video.mode", static_cast<int>(mode));
}
// gives back true if vsync is on (saved as 1), false if it is off (saved as 0)
bool Settings::get_vsync()
{
    return get_int("video.vsync", 1) != 0;
}
// stores vsync as 1 if on, 0 if off
void Settings::set_vsync(bool on)
{
    set_int("video.vsync", on ? 1 : 0);
}

// gives back the list of window sizes that fit on this monitor
std::vector<sf::Vector2u> Settings::get_supported_resolutions()
{
    // candidates = the common sizes we could offer
    const std::vector<sf::Vector2u> candidates = {
        {1280, 720}, {1366, 768}, {1600, 900}, {1920, 1080}, {2560, 1440}, {3840, 2160}};

    // desk = the size of the player's monitor
    const auto desk = sf::VideoMode::getDesktopMode();

    // out = the list we will give back, empty for now
    std::vector<sf::Vector2u> out;
    // for every candidate, if it fits on the monitor, add it to the list
    for (const auto &c : candidates)
    {
        if (c.x <= desk.width && c.y <= desk.height)
            out.push_back(c);
    }
    // if nothing fitted, add 1280 by 720 so the list is never empty
    if (out.empty())
        out.push_back({1280, 720});
    // current = the size being used right now
    const auto current = get_resolution();
    // if the current size is not in the list, add it
    if (std::find(out.begin(), out.end(), current) == out.end())
        out.push_back(current);
    return out;
}

// turns a window type into text for the menu to show
std::string Settings::window_mode_name(WindowMode mode)
{
    switch (mode)
    {
    // if it is fullscreen, give back "Fullscreen"
    case WindowMode::Fullscreen:
        return "Fullscreen";
    // if it is borderless, give back "Borderless"
    case WindowMode::Borderless:
        return "Borderless";
    // anything else, give back "Windowed"
    default:
        return "Windowed";
    }
}
