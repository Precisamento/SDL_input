#ifndef SDL_INPUT_INPUT_MANAGER_H
#define SDL_INPUT_INPUT_MANAGER_H

#include <SDL.h>

// When compiling, can be used to change the amount of
// gamepads that can be detected by the gamepad manager.
// Rarely does this need to be > 4, but if you want a massive
// local co-op game like 16 player Halo LAN games, it can be increased
// accordingly.
#ifndef INPUT_MAX_GAMEPADS
#define INPUT_MAX_GAMEPADS 4
#endif

/*
    Defines some extra values to allow specific stick directions to be
    used like other controller buttons by the input manager.
*/
typedef enum SDL_GameControllerButtonExtension {
    SDL_CONTROLLER_BUTTON_EXTENSION_INVALID = -1,
    SDL_CONTROLLER_BUTTON_LEFTSTICKUP = SDL_CONTROLLER_BUTTON_MAX,
    SDL_CONTROLLER_BUTTON_LEFTSTICKLEFT,
    SDL_CONTROLLER_BUTTON_LEFTSTICKDOWN,
    SDL_CONTROLLER_BUTTON_LEFTSTICKRIGHT,
    SDL_CONTROLLER_BUTTON_RIGHTSTICKUP,
    SDL_CONTROLLER_BUTTON_RIGHTSTICKLEFT,
    SDL_CONTROLLER_BUTTON_RIGHTSTICKDOWN,
    SDL_CONTROLLER_BUTTON_RIGHTSTICKRIGHT,
    SDL_CONTROLLER_BUTTON_LEFTTRIGGER,
    SDL_CONTROLLER_BUTTON_RIGHTTRIGGER,
    SDL_CONTROLLER_BUTTON_EXTENSION_MAX
} SDL_GameControllerButtonExtension;

/*
    Defines some extra values to allow mouse scroll directions to
    be used like other mouse buttons by the input manager.
*/
typedef enum SDL_MouseButtonExtension {
    SDL_MouseScrollLeft = SDL_BUTTON_X2 + 1,
    SDL_MouseScrollRight,
    SDL_MouseScrollUp,
    SDL_MouseScrollDown
} SDL_MouseButtonExtension;

/*
    Represents a mouse button.
*/
typedef Uint32 MouseButton;

/*
    Represents a gamepad button.
*/
typedef Uint32 GamepadButton;

/*
    Represents a gamepad stick or trigger value.
*/
typedef Uint16 GamepadAxis;

typedef struct InputGamepad {
    SDL_GameController* controller;
    GamepadButton button_current;
    GamepadButton button_previous;
    SDL_bool active;
} InputGamepad;

typedef struct InputManager {
    MouseButton mouse_current;
    MouseButton mouse_previous;
    SDL_Point mouse_position_current;
    SDL_Point mouse_position_previous;
    const Uint8* keyboard_current;
    const Uint8* keyboard_previous;
    InputGamepad gamepads[INPUT_MAX_GAMEPADS];
    unsigned int controllers[INPUT_MAX_GAMEPADS];
    unsigned int controller_count;
    GamepadAxis deadzone;
    SDL_TouchFingerEvent* touch_previous;
    SDL_TouchFingerEvent* touch_current;
    unsigned int touch_current_count;
    unsigned int touch_current_capacity;
    unsigned int touch_previous_count;
    unsigned int touch_previous_capacity;
    unsigned int touch_event_poll_count;
    int mouse_wheel_x;
    int mouse_wheel_y;
    int mouse_poll_scroll_x;
    int mouse_poll_scroll_y;
} InputManager;

#define ___INPUT_GAMEPAD_BUTTON(x) (1 << (x))

// Checks if the specified key is currently down.
static inline SDL_bool input_key_check(InputManager* input, SDL_Scancode key) {
    return input->keyboard_current[key] == 1;
}

// Checks if the specified key was just pressed during the last update.
static inline SDL_bool input_key_pressed(InputManager* input, SDL_Scancode key) {
    return input->keyboard_current[key] == 1 && input->keyboard_previous[key] == 0;
}

// Checks if the specified key was just released during the last update.
static inline SDL_bool input_key_released(InputManager* input, SDL_Scancode key) {
    return input->keyboard_current[key] == 0 && input->keyboard_previous[key] == 1;
}

// Checks if the specified button is currently down.
static inline SDL_bool input_mouse_check(InputManager* input, MouseButton button) {
    return (input->mouse_current & button) == button;
}


// Checks if the specified button was just pressed during the last update.
static inline SDL_bool input_mouse_pressed(InputManager* input, MouseButton button) {
    return ((input->mouse_current & button) == button) && ((input->mouse_previous & button) != button);
}

// Checks if the specified button was just released during the last update.
static inline SDL_bool input_mouse_released(InputManager* input, MouseButton button) {
    return ((input->mouse_current & button) != button) && ((input->mouse_previous & button) == button);
}

// Determines if the mouse moved at all during the last update.
static inline SDL_bool input_mouse_moved(InputManager* input) {
    return input->mouse_position_current.x != input->mouse_position_previous.x ||
           input->mouse_position_current.y != input->mouse_position_previous.y;
}

// Gets the current mouse position relative to the application window.
static inline SDL_Point input_mouse_position(InputManager* input) {
    return input->mouse_position_current;
}

/*
   Checks if the specified button is currently down.
   @index The gamepad index retrieved from the SDL_ControllerDeviceEvent,
          or -1 to get the first controller plugged in.
*/
static inline SDL_bool input_gamepad_check_index(InputManager* input, GamepadButton button, int index) {
    if(index == -1)
        index = input->controllers[0];

    if(index < 0 || index >= INPUT_MAX_GAMEPADS || !input->gamepads[index].active)
        return SDL_FALSE;

    return (input->gamepads[index].button_current & ___INPUT_GAMEPAD_BUTTON(button)) != 0;
}


/*
   Checks if the specified button was just pressed during the last update.
   @index The gamepad index retrieved from the SDL_ControllerDeviceEvent,
          or -1 to get the first controller plugged in.
*/
static inline SDL_bool input_gamepad_pressed_index(InputManager* input, GamepadButton button, int index) {
    if(index == -1)
        index = input->controllers[0];

    if(index < 0 || index >= INPUT_MAX_GAMEPADS || !input->gamepads[index].active)
        return SDL_FALSE;

    return (input->gamepads[index].button_current & ___INPUT_GAMEPAD_BUTTON(button)) != 0 &&
           (input->gamepads[index].button_previous & ___INPUT_GAMEPAD_BUTTON(button)) == 0;
}



/*
   Checks if the specified button was just released during the last update.
   @index The gamepad index retrieved from the SDL_ControllerDeviceEvent,
          or -1 to get the first controller plugged in.
*/
static inline SDL_bool input_gamepad_released_index(InputManager* input, GamepadButton button, int index) {
    if(index == -1)
        index = input->controllers[0];

    if(index < 0 || index >= INPUT_MAX_GAMEPADS || !input->gamepads[index].active)
        return SDL_FALSE;

    return (input->gamepads[index].button_current & ___INPUT_GAMEPAD_BUTTON(button)) == 0 &&
           (input->gamepads[index].button_previous & ___INPUT_GAMEPAD_BUTTON(button)) != 0;
}


/*
   Gets the specified axis value.
   @index The gamepad index retrieved from the SDL_ControllerDeviceEvent,
          or -1 to get the first controller plugged in.
*/
static inline Uint16 input_gamepad_axis_value_index(InputManager* input, SDL_GameControllerAxis axis, int index) {
    if(index == -1)
        index = input->controllers[0];

    if(index < 0 || index >= INPUT_MAX_GAMEPADS || !input->gamepads[index].active)
        return 0;

    return SDL_GameControllerGetAxis(input->gamepads[index].controller, axis);
}

// Sets the deadzone for an axis to be considered active.
static inline void input_gamepad_set_deadzone(InputManager* input, Uint16 value) {
    input->deadzone = value;
}

// Gets the deadzone for an axis to be considered active.
static inline Uint16 input_gamepad_get_deadzone(InputManager* input) {
    return input->deadzone;
}

// Gets the touch events for the previous previous update.
static inline SDL_TouchFingerEvent* input_touch_previous(InputManager* input, int* out_count) {
    *out_count = input->touch_previous_count;
    return input->touch_previous;
}

// Gets the touch events for the current update.
static inline SDL_TouchFingerEvent* input_touch_current(InputManager* input, int* out_count) {
    *out_count = input->touch_current_count;
    return input->touch_current;
}

// Allocates and initializes a new <InputManager>.
InputManager* input_manager_create(void);

// Frees an <InputManager>.
void input_manager_free(InputManager* input);

// Updates an <InputManager>. Only call this after polling for input events.
void input_manager_update(InputManager* input);

// Updates controller state based off of the event. Should be called before <input_manager_update>.
void input_manager_controller_event(InputManager* input, SDL_ControllerDeviceEvent* event);

// Updates mouse wheel state based off of the event. Should be called before <input_manager_update>.
void input_manager_mouse_wheel_event(InputManager* input, SDL_MouseWheelEvent* event);

// Updates touch state based off of the event. Should be called before <input_manager_update>.
void input_manager_touch_event(InputManager* input, SDL_TouchFingerEvent* event);

#endif