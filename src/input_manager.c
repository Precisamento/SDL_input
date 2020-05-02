#include <input_manager.h>

#include "std_definitions.h"

InputManager* input_manager_create(void) {
    if(!SDL_WasInit(SDL_INIT_GAMECONTROLLER)) {
        if(SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) != 0)
            return NULL;
    }

    InputManager* input = input_malloc(sizeof(*input));
    if(!input)
        return NULL;

    input->deadzone = (Uint16)(SDL_MAX_SINT16 * .15f);
    input->mouse_current = SDL_GetMouseState(&input->mouse_position_current.x, &input->mouse_position_current.y);
    input->keyboard_current = SDL_GetKeyboardState(NULL);

    input->touch_previous = NULL;
    input->touch_previous_count = 0;
    input->touch_previous_capacity = 0;
    input->touch_current = NULL;
    input->touch_current_count = 0;
    input->touch_current_capacity = 0;
    input->touch_event_poll_count = 0;

    return input;
}

void input_manager_free(InputManager* input) {
    input_free(input->touch_previous);
    input_free(input->touch_current);
    input_free(input);
}

static void input_gamepad_update(InputManager* input, InputGamepad* gamepad) {
    gamepad->button_previous = gamepad->button_current;
    gamepad->button_current = 0;

    for(int i = SDL_CONTROLLER_BUTTON_A; i < SDL_CONTROLLER_BUTTON_MAX; i++) {
        if(SDL_GameControllerGetButton(gamepad->controller, i))
            gamepad->button_current |= ___INPUT_GAMEPAD_BUTTON(i);
    }

    for(int i = SDL_CONTROLLER_AXIS_LEFTX; i < SDL_CONTROLLER_AXIS_MAX; i++) {
        Sint16 axis = SDL_GameControllerGetAxis(gamepad->controller, i);
        if(axis < -input->deadzone || axis > input->deadzone) {
            int index;
            switch(i) {
                case SDL_CONTROLLER_AXIS_LEFTX:
                    index = axis < 0 ? SDL_CONTROLLER_BUTTON_LEFTSTICKLEFT : SDL_CONTROLLER_BUTTON_LEFTSTICKRIGHT;
                    break;
                case SDL_CONTROLLER_AXIS_LEFTY:
                    index = axis < 0 ? SDL_CONTROLLER_BUTTON_LEFTSTICKUP : SDL_CONTROLLER_BUTTON_LEFTSTICKDOWN;
                    break;
                case SDL_CONTROLLER_AXIS_RIGHTX:
                    index = axis < 0 ? SDL_CONTROLLER_BUTTON_RIGHTSTICKLEFT : SDL_CONTROLLER_BUTTON_RIGHTSTICKRIGHT;
                    break;
                case SDL_CONTROLLER_AXIS_RIGHTY:
                    index = axis < 0 ? SDL_CONTROLLER_BUTTON_RIGHTSTICKUP : SDL_CONTROLLER_BUTTON_RIGHTSTICKDOWN;
                    break;
                case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
                    index = SDL_CONTROLLER_BUTTON_LEFTTRIGGER;
                    break;
                case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
                    index = SDL_CONTROLLER_BUTTON_RIGHTTRIGGER;
                    break;
            }
            gamepad->button_current |= ___INPUT_GAMEPAD_BUTTON(index);
        }
    }
}

void input_manager_update(InputManager* input) {
    input->mouse_previous = input->mouse_current;
    input->mouse_position_previous = input->mouse_position_current;
    input->keyboard_previous = input->keyboard_current;

    input->mouse_wheel_x = input->mouse_poll_scroll_x;
    input->mouse_wheel_y = input->mouse_poll_scroll_y;
    input->mouse_poll_scroll_x = input->mouse_poll_scroll_y = 0;
    input->mouse_current = SDL_GetMouseState(&input->mouse_position_current.x, &input->mouse_position_current.y);

    if(input->mouse_wheel_x < 0)
        input->mouse_current |= SDL_BUTTON(SDL_MouseScrollLeft);
    else if(input->mouse_wheel_x > 0)
        input->mouse_current |= SDL_BUTTON(SDL_MouseScrollRight);

    if(input->mouse_wheel_y < 0)
        input->mouse_current |= SDL_BUTTON(SDL_MouseScrollDown);
    else if(input->mouse_wheel_y > 0)
        input->mouse_current |= SDL_BUTTON(SDL_MouseScrollUp);


    input->keyboard_current = SDL_GetKeyboardState(NULL);

    for(int i = 0; i < input->controller_count; i++)
        input_gamepad_update(input, &input->gamepads[input->controllers[i]]);

    // During the pre-update phase where the caller should have polled for events,
    // input->touch_previous was overwritten with the newest events.
    // Here, we need to swap the values for current and previous
    // to finish updating the touch state.

    SDL_TouchFingerEvent* temp = input->touch_current;
    unsigned int temp_capacity = input->touch_current_capacity;
    unsigned int temp_count = input->touch_current_count;

    input->touch_current = input->touch_previous;
    input->touch_current_count = input->touch_previous_count;
    input->touch_current_capacity = input->touch_previous_capacity;

    input->touch_previous = temp;
    input->touch_previous_count = temp_count;
    input->touch_previous_capacity = temp_capacity;
}

static void input_gamepad_open(InputManager* input, int index) {
    SDL_GameController* controller = SDL_GameControllerOpen(index);

    if(!controller)
        return;

    InputGamepad* gp = input->gamepads + index;
    gp->controller = controller;
    gp->active = SDL_TRUE;
    input_gamepad_update(input, gp);
    input->controllers[input->controller_count++] = index;
}

void input_manager_controller_event(InputManager* input, SDL_ControllerDeviceEvent* event) {
    switch(event->type) {
        case SDL_CONTROLLERDEVICEADDED:
            input_gamepad_open(input, event->which);
            break;
        case SDL_CONTROLLERDEVICEREMOVED:
            for(int i = 0; i < INPUT_MAX_GAMEPADS; i++) {
                if(input->controllers[i] == event->which) {
                    input->gamepads[i].active = SDL_FALSE;
                    if(i != input->controller_count - 1) {
                        input_memmove(input->controllers + i, 
                                      input->controllers + i + 1, 
                                      (--input->controller_count - i) * sizeof(*input->controllers));
                        input->controllers[input->controller_count] = -1;
                        break;
                    }
                }
            }
            break;
    }
}

void input_manager_mouse_wheel_event(InputManager* input, SDL_MouseWheelEvent* event) {
    if(event->x != 0) {
        int x = event->x;
        if(event->direction == SDL_MOUSEWHEEL_FLIPPED)
            x *= -1;

        input->mouse_poll_scroll_x += x;
    } else {
        int y = event->y;
        if(event->direction == SDL_MOUSEWHEEL_FLIPPED)
            y *= -1;

        input->mouse_poll_scroll_y += y;
    }
}

void input_manager_touch_event(InputManager* input, SDL_TouchFingerEvent* event) {
    if(input->touch_event_poll_count == input->touch_previous_capacity) {
        unsigned int capacity = input->touch_previous_capacity == 0 ? 4 : input->touch_previous_capacity * 2;
        void* buffer = input_realloc(input->touch_previous, capacity * sizeof(*input->touch_previous));
        if(!buffer)
            return;
        input->touch_previous_capacity = capacity;
        input->touch_previous = buffer;
    }

    input->touch_previous[input->touch_event_poll_count++] = *event;
}