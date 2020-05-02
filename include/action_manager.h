#ifndef SDL_INPUT_ACTION_MANAGER_H
#define SDL_INPUT_ACTION_MANAGER_H

#include <SDL.h>
#include "input_manager.h"

typedef enum InputActionType {
    INPUT_ACTION_KEYBOARD,
    INPUT_ACTION_GAMEPAD,
    INPUT_ACTION_MOUSE
} InputActionType;

typedef struct InputAction {
    InputActionType type;
    union {
        SDL_Scancode key;
        MouseButton mouse;
        struct {
            GamepadButton button;
            int controller_index;
        } gamepad;
    };
} InputAction;

typedef struct InputActionMap {
    InputAction* actions;
    int action_count;
    int action_capacity;
    SDL_bool previous;
    SDL_bool current;
} InputActionMap;

typedef struct ActionManager {
    InputActionMap* actions;
    int action_count;
} ActionManager;

static inline SDL_bool action_check(ActionManager* action_manager, Uint32 action) {
    return action_manager->actions[action].current;
}

static inline SDL_bool action_pressed(ActionManager* action_manager, Uint32 action) {
    return action_manager->actions[action].current && 
           !action_manager->actions[action].previous;
}

static inline SDL_bool action_released(ActionManager* action_manager, Uint32 action) {
    return !action_manager->actions[action].current && 
           action_manager->actions[action].previous;
}

SDL_bool action_manager_add_key(ActionManager* action_manager, Uint32 action, SDL_Scancode key);
SDL_bool action_manager_add_mouse_button(ActionManager* action_manager, Uint32 action, MouseButton button);
SDL_bool action_manager_add_gamepad_button(ActionManager* action_manager, Uint32 action, GamepadButton gamepad, int controller_index);
void action_manager_clear_action(ActionManager* action_manager, Uint32 action);

ActionManager* action_manager_create(Uint32 action_count);
void action_manager_free(ActionManager* action_manager);
void action_manager_update(ActionManager* action_manager, InputManager* input);

#endif