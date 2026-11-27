// input.cpp - see input.hpp
// this is the code behind each tool listed in the header

#include "input.hpp"
#include "settings.hpp"
#include <array>
#include <cctype>
#include <cmath>

// anything in here can only be used inside this file
namespace
{
    // how many actions there are (Count is the last one in the list, so its number is the total)
    constexpr size_t action_count = static_cast<size_t>(Action::Count);

    // a stick resting in the middle is rarely exactly 0
    // SFML sticks go from -100 to 100, anything under 35 counts as "not pushed"
    constexpr float stick_dead_zone = 35.f;

    // SFML reports D-pad up differently per system (checked in SFML 2.6.2 source)
    // Windows gives +100 for up, Linux gives -100 for up
#if defined(_WIN32)
    constexpr float pov_y_up_sign = 1.f;
#else
    constexpr float pov_y_up_sign = -1.f;
#endif

    // one action's controls: a keyboard key and a controller button
    struct Binding
    {
        sf::Keyboard::Key key;
        int joy_button; // -1 = no button
    };

    // the default controls, from the GDD controls table, in the same order as the Action list
    // button numbers follow the usual XInput layout on Windows:
    // 0=A 1=B 2=X 3=Y 4=LB 5=RB 6=Back 7=Start
    const std::array<Binding, action_count> default_bindings = {{
        {sf::Keyboard::W, -1},      // Up (D-pad and stick are handled separately)
        {sf::Keyboard::S, -1},      // Down
        {sf::Keyboard::A, -1},      // Left
        {sf::Keyboard::D, -1},      // Right
        {sf::Keyboard::E, 2},       // Interact - X / Square
        {sf::Keyboard::Escape, 7},  // Menu - Start / Menu button
        {sf::Keyboard::Unknown, 5}, // NextItem - RB (and mouse wheel)
        {sf::Keyboard::Unknown, 4}, // PrevItem - LB (and mouse wheel)
    }};

    // bindings = the controls being used right now (starts as the defaults)
    std::array<Binding, action_count> bindings = default_bindings;

    // down = for each action, is it being held right now?
    std::array<bool, action_count> down{};

    // event_pressed = for each action, did it get pressed this frame?
    std::array<bool, action_count> event_pressed{};

    // pending_presses = for each action, how many presses are waiting to be handed out
    // a counter, not true/false, so two quick presses in one slow frame are not merged
    // the second one is handed out next frame instead of being lost
    std::array<int, action_count> pending_presses{};

    // D-pad and stick have no "pressed" event, so we compare this frame with last frame
    // axis_down = is the direction pushed now, prev_axis_down = was it pushed last frame
    // keys and buttons use events only, checking them both ways can count one press twice
    std::array<bool, action_count> axis_down{};
    std::array<bool, action_count> prev_axis_down{};

    // mouse_down = is the left button held now, mouse_prev = was it held last frame
    bool mouse_down = false;
    bool mouse_prev = false;

    // has_focus = is the game window the one being used?
    bool has_focus = true;

    // last_was_controller = did the last input come from a controller?
    bool last_was_controller = false;

    // swallow_frame = ignore held keys for one frame after a rebind
    bool swallow_frame = false;

    // wheel_delta = how far the mouse wheel moved this frame
    float wheel_delta = 0.f;

    // rebinding = are we waiting for the player to press a new key or button?
    bool rebinding = false;

    // rebinding_controller = are we waiting for a controller button (true) or a key (false)?
    bool rebinding_controller = false;

    // rebind_action = which action is getting the new key or button
    Action rebind_action = Action::Up;

    // turns an action into its number, so it can be used as a position in the lists above
    size_t idx(Action a)
    {
        return static_cast<size_t>(a);
    }

    // the name each action is saved under in the settings file, e.g. bind.up.key
    const char *action_keys[action_count] = {"up", "down", "left", "right",
                                             "interact", "menu", "next_item", "prev_item"};

    // gives back the number of the first controller plugged in, or -1 if there is none
    int first_joystick()
    {
        // for every controller slot, if one is connected, give back its number
        for (unsigned int j = 0; j < sf::Joystick::Count; ++j)
        {
            if (sf::Joystick::isConnected(j))
                return static_cast<int>(j);
        }
        return -1;
    }

    // is this direction being pushed on the D-pad or the left stick?
    bool joy_direction(Action a)
    {
        // j = the first controller, if there is none the answer is no
        const int j = first_joystick();
        if (j < 0)
            return false;
        const auto ju = static_cast<unsigned int>(j);

        // x and y = where the left stick is, from -100 to 100
        const float x = sf::Joystick::getAxisPosition(ju, sf::Joystick::X);
        const float y = sf::Joystick::getAxisPosition(ju, sf::Joystick::Y);

        // px and py = where the D-pad is, or 0 if this controller has no D-pad
        const float px = sf::Joystick::hasAxis(ju, sf::Joystick::PovX)
                             ? sf::Joystick::getAxisPosition(ju, sf::Joystick::PovX)
                             : 0.f;
        const float py = sf::Joystick::hasAxis(ju, sf::Joystick::PovY)
                             ? sf::Joystick::getAxisPosition(ju, sf::Joystick::PovY) * pov_y_up_sign
                             : 0.f;

        // the direction counts if the stick or the D-pad is pushed past the dead zone
        switch (a)
        {
        case Action::Up:
            return y < -stick_dead_zone || py > stick_dead_zone;
        case Action::Down:
            return y > stick_dead_zone || py < -stick_dead_zone;
        case Action::Left:
            return x < -stick_dead_zone || px < -stick_dead_zone;
        case Action::Right:
            return x > stick_dead_zone || px > stick_dead_zone;

        // any other action is not a direction
        default:
            return false;
        }
    }
} // namespace

// Setup

// sets the controls, called once when the game starts
void Input::initialise()
{
    // check which controllers are plugged in
    sf::Joystick::update();

    // for every action
    for (size_t i = 0; i < action_count; ++i)
    {
        // base = the start of its name in the settings file, e.g. "bind.up"
        const std::string base = std::string("bind.") + action_keys[i];

        // key = the saved key, or the default key if none is saved
        const int key = Settings::get_int(base + ".key", static_cast<int>(default_bindings[i].key));

        // joy = the saved controller button, or the default button if none is saved
        const int joy = Settings::get_int(base + ".joy", default_bindings[i].joy_button);

        // if the saved key is a real key use it, otherwise use the default
        bindings[i].key = (key >= -1 && key < sf::Keyboard::KeyCount)
                              ? static_cast<sf::Keyboard::Key>(key)
                              : default_bindings[i].key;

        // if the saved button is a real button use it, otherwise use the default
        bindings[i].joy_button = (joy >= -1 && joy < static_cast<int>(sf::Joystick::ButtonCount))
                                     ? joy
                                     : default_bindings[i].joy_button;
    }
}

// Every frame

// a new frame is starting, forget last frame's presses and wheel movement
void Input::begin_frame()
{
    event_pressed.fill(false);
    wheel_delta = 0.f;
}

// GameSystem hands every event (key press, button press, wheel move) to this
void Input::handle_event(const sf::Event &e)
{
    // Rebinding - if we are waiting for a new key or button, this press becomes it
    if (rebinding)
    {
        // if we are waiting for a key and a key was pressed
        if (!rebinding_controller && e.type == sf::Event::KeyPressed)
        {
            // if the key is not Escape, or the action being changed is Menu, use the new key
            // (Escape on any other action cancels and keeps the old key)
            if (e.key.code != sf::Keyboard::Escape || rebind_action == Action::Menu)
            {
                bindings[idx(rebind_action)].key = e.key.code;
            }

            // stop waiting
            rebinding = false;

            // ignore this press so it doesn't also do something in the menu
            swallow_frame = true;
            return;
        }

        // if we are waiting for a controller button and one was pressed, use it
        if (rebinding_controller && e.type == sf::Event::JoystickButtonPressed)
        {
            bindings[idx(rebind_action)].joy_button = static_cast<int>(e.joystickButton.button);
            rebinding = false;
            swallow_frame = true;
            return;
        }

        // if we are waiting for a controller button and Escape was pressed, cancel
        if (rebinding_controller && e.type == sf::Event::KeyPressed &&
            e.key.code == sf::Keyboard::Escape)
        {
            rebinding = false;
            swallow_frame = true;
            return;
        }

        // anything else while waiting is ignored
        return;
    }

    // Normal play - work out what kind of event this is
    switch (e.type)
    {
    // the mouse wheel moved
    case sf::Event::MouseWheelScrolled:

        // if it was the up/down wheel, add how far it moved
        if (e.mouseWheelScroll.wheel == sf::Mouse::VerticalWheel)
        {
            wheel_delta += e.mouseWheelScroll.delta;
            last_was_controller = false;
        }
        break;

    // a keyboard key was pressed
    case sf::Event::KeyPressed:
        last_was_controller = false;

        // for every action, if this key is the one it is on, add one waiting press
        for (size_t i = 0; i < action_count; ++i)
        {
            if (bindings[i].key == e.key.code && e.key.code != sf::Keyboard::Unknown)
            {
                ++pending_presses[i];
            }
        }

        // arrow keys and Enter always work in menus, even if the player remapped things
        // (the second half of each line stops a press being counted twice)
        if (e.key.code == sf::Keyboard::Up && bindings[idx(Action::Up)].key != e.key.code)
            ++pending_presses[idx(Action::Up)];
        if (e.key.code == sf::Keyboard::Down && bindings[idx(Action::Down)].key != e.key.code)
            ++pending_presses[idx(Action::Down)];
        if (e.key.code == sf::Keyboard::Left && bindings[idx(Action::Left)].key != e.key.code)
            ++pending_presses[idx(Action::Left)];
        if (e.key.code == sf::Keyboard::Right && bindings[idx(Action::Right)].key != e.key.code)
            ++pending_presses[idx(Action::Right)];
        if (e.key.code == sf::Keyboard::Enter && bindings[idx(Action::Interact)].key != e.key.code)
            ++pending_presses[idx(Action::Interact)];
        break;

    // a controller button was pressed
    case sf::Event::JoystickButtonPressed:
        last_was_controller = true;

        // for every action, if this button is the one it is on, add one waiting press
        for (size_t i = 0; i < action_count; ++i)
        {
            if (bindings[i].joy_button == static_cast<int>(e.joystickButton.button))
            {
                ++pending_presses[i];
            }
        }
        break;

    // a stick moved - if it moved past the dead zone, the player is using a controller
    case sf::Event::JoystickMoved:
        if (std::abs(e.joystickMove.position) > stick_dead_zone)
            last_was_controller = true;
        break;

    // any other kind of event is ignored
    default:
        break;
    }
}

// all the events are in, now work out what is held and what was pressed
void Input::end_events()
{
    // remember what the stick and mouse were doing last frame
    prev_axis_down = axis_down;
    mouse_prev = mouse_down;

    // the mouse is down if our window is in use and the left button is held
    mouse_down = has_focus && sf::Mouse::isButtonPressed(sf::Mouse::Left);

    // j = the first controller, or -1 if there is none
    const int j = first_joystick();

    // for every action, work out if it is being held
    for (size_t i = 0; i < action_count; ++i)
    {
        const auto a = static_cast<Action>(i);

        // d = is this action held? start with no
        bool d = false;

        // if our window is in use and the action has a key, is that key held?
        if (has_focus && bindings[i].key != sf::Keyboard::Unknown)
        {
            d = sf::Keyboard::isKeyPressed(bindings[i].key);
        }

        // if there is a controller and the action has a button, is that button held?
        if (j >= 0 && bindings[i].joy_button >= 0)
        {
            d = d || sf::Joystick::isButtonPressed(static_cast<unsigned int>(j),
                                                   static_cast<unsigned int>(bindings[i].joy_button));
        }
        axis_down[i] = false;

        // if this is one of the four directions (Up, Down, Left, Right)
        if (i <= idx(Action::Right))
        {
            // is the stick or D-pad pushed that way?
            axis_down[i] = joy_direction(a);
            d = d || axis_down[i];

            // the arrow keys also count as directions (menus and accessibility)
            if (has_focus)
            {
                if (a == Action::Up)
                    d = d || sf::Keyboard::isKeyPressed(sf::Keyboard::Up);
                if (a == Action::Down)
                    d = d || sf::Keyboard::isKeyPressed(sf::Keyboard::Down);
                if (a == Action::Left)
                    d = d || sf::Keyboard::isKeyPressed(sf::Keyboard::Left);
                if (a == Action::Right)
                    d = d || sf::Keyboard::isKeyPressed(sf::Keyboard::Right);
            }
        }

        // store the answer
        down[i] = d;
    }

    // if a rebind just happened, throw away any presses from this frame
    if (swallow_frame)
    {
        prev_axis_down = axis_down;
        mouse_prev = mouse_down;
        pending_presses.fill(0);
        swallow_frame = false;
    }

    // the mouse wheel switches items (GDD controls table)
    // if the wheel moved down add a NextItem press, if it moved up add a PrevItem press
    if (wheel_delta < 0.f)
        ++pending_presses[idx(Action::NextItem)];
    if (wheel_delta > 0.f)
        ++pending_presses[idx(Action::PrevItem)];

    // hand out at most one waiting press per action this frame
    for (size_t i = 0; i < action_count; ++i)
    {
        // the action counts as pressed this frame if at least one press is waiting
        event_pressed[i] = pending_presses[i] > 0;

        // if one was waiting, take it off the count
        if (pending_presses[i] > 0)
            --pending_presses[i];

        // never let more than 3 presses queue up
        if (pending_presses[i] > 3)
            pending_presses[i] = 3;
    }
}

// remembers whether the game window is the one being used
// when the player alt-tabs away, keys pressed in other programs are ignored
void Input::set_focus(bool focused)
{
    has_focus = focused;
}

// Asking questions

// is this action being held right now?
bool Input::is_down(Action a)
{
    return down[idx(a)];
}

// did this action get pressed this frame?
bool Input::was_pressed(Action a)
{
    // if we are waiting for a rebind, nothing counts as pressed
    if (rebinding)
        return false;
    const size_t i = idx(a);

    // yes if a key or button press came in,
    // or if the stick is pushed now and was not pushed last frame
    return event_pressed[i] || (axis_down[i] && !prev_axis_down[i]);
}

// which direction is the player pushing?
sf::Vector2f Input::get_move_vector()
{
    // v = the direction, start with not moving
    sf::Vector2f v(0.f, 0.f);

    // if we are waiting for a rebind, don't move
    if (rebinding)
        return v;

    // left and right change x, up and down change y (up is minus because y goes down the screen)
    if (is_down(Action::Left))
        v.x -= 1.f;
    if (is_down(Action::Right))
        v.x += 1.f;
    if (is_down(Action::Up))
        v.y -= 1.f;
    if (is_down(Action::Down))
        v.y += 1.f;

    // l = the length of the direction
    const float l = std::sqrt(v.x * v.x + v.y * v.y);

    // if moving, shrink the length to 1 so diagonal is not faster (lab 4)
    return l > 0.f ? v / l : v;
}

// was the left mouse button clicked this frame?
// yes if it is down now and was not down last frame
bool Input::mouse_clicked()
{
    return !rebinding && mouse_down && !mouse_prev;
}

// was anything at all pressed this frame?
bool Input::any_pressed()
{
    // for every action, if it was pressed, the answer is yes
    for (size_t i = 0; i < action_count; ++i)
    {
        if (was_pressed(static_cast<Action>(i)))
            return true;
    }

    // otherwise it depends on the mouse
    return mouse_clicked();
}

// was the last input from a controller?
bool Input::using_controller()
{
    return last_was_controller;
}

// Remapping

// start waiting for the player to press a new key or button for this action
void Input::begin_rebind(Action a, bool controller)
{
    rebinding = true;
    rebinding_controller = controller;
    rebind_action = a;
}

// are we waiting for a new key or button?
bool Input::is_rebinding()
{
    return rebinding;
}

// stop waiting, keep the old one
void Input::cancel_rebind()
{
    rebinding = false;
}

// which key is this action on?
sf::Keyboard::Key Input::get_key(Action a)
{
    return bindings[idx(a)].key;
}

// which controller button is this action on?
int Input::get_joy_button(Action a)
{
    return bindings[idx(a)].joy_button;
}

// put every action back on its default key and button
void Input::reset_defaults()
{
    bindings = default_bindings;
}

// stores the controls in Settings so they get saved
void Input::save_to_settings()
{
    // for every action, store its key and its button, e.g. bind.up.key and bind.up.joy
    for (size_t i = 0; i < action_count; ++i)
    {
        const std::string base = std::string("bind.") + action_keys[i];
        Settings::set_int(base + ".key", static_cast<int>(bindings[i].key));
        Settings::set_int(base + ".joy", bindings[i].joy_button);
    }
}

// Names for the menu to show

// turns an action into text
std::string Input::action_name(Action a)
{
    switch (a)
    {
    case Action::Up:
        return "Move Up";
    case Action::Down:
        return "Move Down";
    case Action::Left:
        return "Move Left";
    case Action::Right:
        return "Move Right";
    case Action::Interact:
        return "Interact / Use";
    case Action::Menu:
        return "Menu";
    case Action::NextItem:
        return "Next Item";
    case Action::PrevItem:
        return "Previous Item";
    default:
        return "?";
    }
}

// turns a key into text, like "E"
std::string Input::key_name(sf::Keyboard::Key k)
{
    // if there is no key, show a dash
    if (k == sf::Keyboard::Unknown)
        return "-";

    // ask SFML for the key's name in the player's own keyboard layout (SFML 2.6)
    std::string s = sf::Keyboard::getDescription(sf::Keyboard::delocalize(k)).toAnsiString();

    // if the name is one letter, make it a capital
    if (s.size() == 1)
        s[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(s[0])));

    // if SFML gave no name, show "Key" and its number, otherwise show the name
    return s.empty() ? "Key " + std::to_string(static_cast<int>(k)) : s;
}

// turns a controller button number into text, like "Pad X"
std::string Input::joy_button_name(int b)
{
    static const char *names[] = {"A", "B", "X", "Y", "LB", "RB", "Back", "Start", "L3", "R3"};

    // if there is no button, show a dash
    if (b < 0)
        return "-";

    // if it is one of the 10 named buttons, show its name
    if (b < 10)
        return std::string("Pad ") + names[b];

    // otherwise show its number
    return "Pad " + std::to_string(b);
}

// the key and button for an action together, for the controls menu
std::string Input::binding_text(Action a)
{
    // pad = the controller button's name
    std::string pad = joy_button_name(get_joy_button(a));

    // if the action is a direction, the controller part is the D-pad or stick
    if (a <= Action::Right)
        pad = "D-pad/Stick";

    // if the action switches items, the mouse wheel does it too
    if (a == Action::NextItem || a == Action::PrevItem)
    {
        return "Wheel / " + key_name(get_key(a)) + " / " + pad;
    }
    return key_name(get_key(a)) + " / " + pad;
}
