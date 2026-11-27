// settings.hpp - user settings saved to disk as "key=value" lines
// added, not in the labs - coursework needs saved user preferences
// and graphics options (resolution & window mode)
// GDD: settings like controls and resolution are saved locally, game progress is not
// the file lives in the user's config folder, not next to the .exe
// (an installed game can't write inside Program Files)
//
// this file is not the settings themselves, it is the list of tools for using them
// the settings live in settings.ini on disk, and in _values while the game runs

#pragma once // only include this header once per build
#include <SFML/System/Vector2.hpp>
#include <map>
#include <string>
#include <vector>

// assigns an integer to each type of window
// the integer is what gets saved in the settings file (video.mode=1)
enum class WindowMode
{
    Windowed = 0,
    Fullscreen = 1,
    Borderless = 2
};

// groups all the settings functions under one name
// everything is static because the game only has one set of settings
class Settings
{
public:
    Settings() = delete; // static only - never create one

    // The File
    static void load();                 // read the file into memory
    static bool save();                 // write memory back to the file - true if it worked
    static std::string get_file_path(); // Finds the file path where settings were saved

    // Getters and setters
    // key = the name of a setting
    // fallback used if a setting does not exist
    // Input and Audio store their own settings with these too
    static int get_int(const std::string &key, int fallback);                           // "give me this setting as a whole number"
    static float get_float(const std::string &key, float fallback);                     // "give me this setting as a decimal"
    static std::string get_string(const std::string &key, const std::string &fallback); // "give me this setting as text"
    static void set_int(const std::string &key, int value);                             // "store this whole number under this name"
    static void set_float(const std::string &key, float value);                         // "store this decimal under this name"
    static void set_string(const std::string &key, const std::string &value);           // "store this text under this name"

    // Video
    // shortcuts so other code doesn't need to know the key names
    // used by GameSystem and the settings menu
    static sf::Vector2u get_resolution();                         // "tell me the width and height"
    static void set_resolution(const sf::Vector2u &res);          // "here is a width and height, store them"
    static WindowMode get_window_mode();                          // "is it windowed, fullscreen or borderless?"
    static void set_window_mode(WindowMode mode);                 // "store which window type to use"
    static bool get_vsync();                                      // "is vsync on?" - true or false
    static void set_vsync(bool on);                               // "turn vsync on or off"
    static std::vector<sf::Vector2u> get_supported_resolutions(); // "list the sizes that fit this monitor"
    static std::string window_mode_name(WindowMode mode);         // turns Fullscreen into the text "Fullscreen"

private:
    // the settings in memory
    // private, so it can only be changed through the functions above
    static std::map<std::string, std::string> _values;
};