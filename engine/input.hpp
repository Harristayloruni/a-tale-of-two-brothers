// input.hpp - keyboard, mouse and controller input
// added (not in the labs) - the labs read keys directly with hard-coded keys
// (lab 2 controls[] array), which makes remapping impossible
// coursework needs remappable controls and controller support
// GDD controls table:
//   Move W/A/S/D (D-pad)     Interact/use item E (X / Square)
//   Menu Esc (Menu btn)      Switch item mouse wheel (LB / RB)
// so the game asks about actions (is Move Up held?), never about real keys
// which key belongs to which action is stored in Settings and saved to disk
//
// this file is the list of input tools, the code behind them is in input.cpp

// only include this header once per build
#pragma once
#include <SFML/Graphics.hpp>
#include <string>

// the things a player can do, each one gets a number starting from 0
// Count is not an action, it is last so its number equals how many actions there are
enum class Action
{
    Up = 0,
    Down,
    Left,
    Right,
    Interact,
    Menu,
    NextItem,
    PrevItem,
    Count
};

// groups all the input functions under one name
// everything is static because the game only has one input system,
// so you call Input::is_down() and never create an Input object
class Input
{
public:
    // static only, never create one
    Input() = delete;

    // Setup

    // set the default keys, then read any saved ones from Settings
    static void initialise();

    // Every frame
    // GameSystem calls these in this order, once per frame

    // a new frame is starting, forget last frame's presses
    static void begin_frame();

    // here is something that just happened (a key press etc.)
    static void handle_event(const sf::Event &e);

    // that is all the events, now work out what is held and pressed
    static void end_events();

    // the game window was clicked on or clicked away from
    static void set_focus(bool focused);

    // Asking questions
    // this is what the game code uses

    // is this action being held right now? (walking)
    static bool is_down(Action a);

    // did this action get pressed this frame? (menus, interact)
    static bool was_pressed(Action a);

    // which direction is the player pushing? keys, D-pad or stick
    static sf::Vector2f get_move_vector();

    // was the left mouse button clicked this frame?
    static bool mouse_clicked();

    // was anything at all pressed this frame?
    static bool any_pressed();

    // was the last input from a controller?
    static bool using_controller();

    // Remapping
    // controller = true means we are changing the controller button, false means the key

    // the next key or button pressed becomes this action
    static void begin_rebind(Action a, bool controller);

    // are we waiting for that key or button?
    static bool is_rebinding();

    // stop waiting, keep the old one
    static void cancel_rebind();

    // which key is this action on?
    static sf::Keyboard::Key get_key(Action a);

    // which controller button is this action on? -1 = none
    static int get_joy_button(Action a);

    // put every action back on its default key
    static void reset_defaults();

    // store the keys in Settings (call Settings::save() after)
    static void save_to_settings();

    // Names for the menu to show

    // Action::Up becomes Move Up
    static std::string action_name(Action a);

    // a key becomes text like E
    static std::string key_name(sf::Keyboard::Key k);

    // a button number becomes text like Pad X
    static std::string joy_button_name(int b);

    // key and button together, like W / D-pad/Stick
    static std::string binding_text(Action a);
};
