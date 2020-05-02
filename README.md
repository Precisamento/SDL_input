# SDL_input

SDL_input is a general input manager for SDL applications. It has two forms: A general input manager that just keeps track of the various input method states (i.e. keyboard, mouse, etc), and an action manager that is used to categorize different inputs under a common category.

## Example

### input_manager

```c
#include <input_manager.h>

static InputManager* input;

void game_loop_update(void) {
    // The input manager needs to be passed controller events
    // (assuming you plan to have controller support)

    SDL_Event e;
    while(SDL_PollEvent(&e)) {
        switch(e.type) {
            case SDL_CONTROLLERDEVICEADDED:
            case SDL_CONTROLLERDEVICEREMOVED:
                input_manager_event(&e.cdevice);
                break;
            default:
                break;
        }
    }

    // After that, we can update the input manager.
    input_manager_update(input);

    // Then we can check for key/controller/etc states from the previous frame.

    if(input_key_check(input, SDL_SCANCODE_UP))
        puts("Pressing up on the keyboard");

    if(input_key_pressed(input, SDL_SCANCODE_A))
        puts("Just pressed 'A' on the keyboard");

    if(input_gamepad_check_index(input, SDL_CONTROLLER_BUTTON_DPAD_UP) ||
        input_gamepad_check_index(input, SDL_CONTROLLER_BUTTON_LEFTSTICKUP))
    {
        puts("Pressing up on a controller");
    }

    if(input_mouse_check(input, SDL_BUTTON_LEFT))
        puts("Left clicking on the mouse");
}

int main(void) {

    input = input_manager_create();

    // Assume this is the game loop
    while(1) {
        game_loop_update();
    }

    input_manager_free(input);
    return 0;
}
```

### action_manager

```c
#include <input_manager.h>
#include <action_manager.h>

typedef enum {
    ACTION_UP,
    ACTION_DOWN
    ACTION_SIZE
} Actions;

static InputManager* input;
static ActionManager* actions;

void game_loop_update(void) {
    // The input manager needs to be passed controller events
    // (assuming you plan to have controller support)

    SDL_Event e;
    while(SDL_PollEvent(&e)) {
        switch(e.type) {
            case SDL_CONTROLLERDEVICEADDED:
            case SDL_CONTROLLERDEVICEREMOVED:
                input_manager_event(&e.cdevice);
                break;
            default:
                break;
        }
    }

    // After that, we can update the input manager.
    input_manager_update(input);

    // The action manager uses an input manager to update
    action_manager_update(actions, input);

    // Then we can check if any state related to an action is active
    // (i.e. for ACTION_UP it checks if the up button on the keyboard is pressed,
    // or the up button on a controller dpad, or if the left stick on the controller
    // is up, all at once).

    if(action_check(actions, ACTION_UP))
        puts("Action Up is active");

    if(action_check_pressed(actions, ACTION_LEFT))
        puts("Action left has just become active");
}

int main(void) {
    input = input_manager_create();
    actions = action_manager_create(ACTION_SIZE);

    action_manager_add_key(actions, ACTION_UP, SDL_SCANCODE_UP);
    action_manager_add_gamepad_button(actions, ACTION_UP, SDL_CONTROLLER_BUTTON_DPAD_UP, -1);
    action_manager_add_gamepad_button(actions, ACTION_UP, SDL_CONTROLLER_BUTTON_LEFTSTICKUP, -1);

    action_manager_add_key(actions, ACTION_LEFT, SDL_SCANCODE_LEFT);
    action_manager_add_gamepad_button(actions, ACTION_LEFT, SDL_CONTROLLER_BUTTON_DPAD_LEFT, -1);
    action_manager_add_gamepad_button(actions, ACTION_LEFT, SDL_CONTROLLER_BUTTON_LEFTSTICKLEFT, -1);

    // Assume this is the game loop
    while(1) {
        game_loop_update();
    }

    input_manager_free(input);
    action_manager_free(actions);
    return 0;
}
```

## Including

This library is pretty small, so you can easily clone it directly into your projects, but it uses meson to make the process even easier.

In your `subprojects` folder add the following wrap file:

```
[wrap-git]

directory = SDL_input
url = https://github.com/Precisamento/SDL_input.git
revision = master
clone-recursive = true
```

Then you can reference the project in your meson file like so:

```meson
SDL_input = subproject('SDL_input')
SDL_input_dep = SDL_input.get_variable('sdl_input_dep')
```

then you can use `sdl_input_dep` just like any other dependency.

## Building

This library uses meson to build. On windows, you need to specify the location of SDL, but there are no other options, just buld like normal. On linux, it assumes that SDL is installed on your system, but the location can be manually specified if it's not.

To build on windows:

```cmd
/      mkdir build
/      cd build
build/ meson ..
build/ meson configure "-Dsdl_dir=path/to/sdl"
build/ ninja
```

The SDL location should have the following layout (this is the default when installing for windows):

```
/include
/lib
    /x86
    /x64
```