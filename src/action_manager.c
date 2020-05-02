#include <action_manager.h>

#include "std_definitions.h"

static SDL_bool action_map_check_resize(InputActionMap* map) {
    if(map->action_count == map->action_capacity) {
        unsigned int capacity = map->action_capacity == 0 ? 2 : map->action_capacity * 2;
        void* buffer = input_realloc(map->actions, capacity * sizeof(*map->actions));
        if(!buffer)
            return SDL_FALSE;

        map->action_capacity = capacity;
        map->actions = buffer;
    }

    return SDL_TRUE;
}

SDL_bool action_manager_add_key(ActionManager* action_manager, Uint32 action, SDL_Scancode key) {
    InputActionMap* map = &action_manager->actions[action];
    if(!action_map_check_resize(map))
        return SDL_FALSE;

    map->actions[map->action_count++] = (InputAction){ .type = INPUT_ACTION_KEYBOARD, .key = key };
    return SDL_TRUE;
}

SDL_bool action_manager_add_mouse_button(ActionManager* action_manager, Uint32 action, MouseButton button) {
    InputActionMap* map = &action_manager->actions[action];
    if(!action_map_check_resize(map))
        return SDL_FALSE;

    map->actions[map->action_count++] = (InputAction){ .type = INPUT_ACTION_KEYBOARD, .mouse = button };
    return SDL_TRUE;
}

SDL_bool action_manager_add_gamepad_button(ActionManager* action_manager, Uint32 action, GamepadButton button, int controller_index) {
    InputActionMap* map = &action_manager->actions[action];
    if(!action_map_check_resize(map))
        return SDL_FALSE;

    InputAction input_action;
    input_action.type = INPUT_ACTION_GAMEPAD;
    input_action.gamepad.button = button;
    input_action.gamepad.controller_index = controller_index;

    map->actions[map->action_count++] = input_action;
    return SDL_TRUE;
}

void action_manager_clear_action(ActionManager* action_manager, Uint32 action) {
    action_manager->actions[action].action_count = 0;
}

ActionManager* action_manager_create(Uint32 action_count) {
    ActionManager* action_manager = input_malloc(sizeof(*action_manager));
    if(!action_manager)
        return NULL;

    InputActionMap* actions = input_calloc(action_count, sizeof(*actions));
    if(!actions) {
        input_free(action_manager);
        return NULL;
    }

    return action_manager;
}

void action_manager_free(ActionManager* action_manager) {
    for(int i = 0; i < action_manager->action_count; i++) {
        input_free(action_manager->actions[i].actions);
    }

    input_free(action_manager->actions);
    input_free(action_manager);
}

static void input_update_action_map(InputActionMap* map, InputManager* input) {
    map->previous = map->current;

    for(int i = 0; i < map->action_count; i++) {
        InputAction* action = &map->actions[i];
        switch(action->type) {
            case INPUT_ACTION_KEYBOARD:
                map->current = input_key_check(input, action->key);
                break;
            case INPUT_ACTION_GAMEPAD:
                map->current = input_gamepad_check_index(input, action->gamepad.button, action->gamepad.controller_index);
                break;
            case INPUT_ACTION_MOUSE:
                map->current = input_mouse_check(input, action->mouse);
                break;
        }

        if(map->current)
            return;
    }
}

void action_manager_update(ActionManager* action_manager, InputManager* input) {
    for(int i = 0; i < action_manager->action_count; i++)
        input_update_action_map(&action_manager->actions[i], input);
}