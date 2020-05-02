project('SDL_Input', 'c')

c_comp = meson.get_compiler('c')

include_files = ['include']

if(c_comp.has_header('SDL.h'))
    sdl = c_comp.find_library('SDL2',
        required: true)
else
    sdl_dir = get_option('sdl_dir')
    if sdl_dir == ''
        sdl = disabler()
    else
        include_files += sdl_dir + '/include'

        cpus = {
            'aarch64' : 'x64',
            'arc' : 'x86',
            'arm' : 'x86',
            'ia64' : 'x64',
            'mips' : 'x86',
            'mips64' : 'x64',
            'ppc' : 'x86',
            'ppc64' : 'x64',
            'riscv32' : 'x86',
            'riscv64' : 'x64',
            'rx' : 'x86',
            'sparc' : 'x86',
            'sparc64' : 'x64',
            'wasm32' : 'x86',
            'wasm64' : 'x64',
            'x86' : 'x86',
            'x86_64' : 'x64'
        }

        cpu = cpus.get(target_machine.cpu_family())

        sdl = c_comp.find_library('SDL2',
            required: true,
            dirs: sdl_dir + '/lib/' + cpu)
    endif
endif

deps = [sdl]

inc = include_directories(include_files)

sources = [
    './src/action_manager.c',
    './src/input_manager.c'
]

sdl_input = static_library(
    'SDL_Input',
    sources,
    include_directories: inc,
    dependencies: deps,
    install: true,
    name_suffix: 'lib',
    name_prefix: ''
)

sdl_input_shared = shared_library(
    'SDL_Input',
    sources,
    include_directories: inc,
    dependencies: deps,
    install: true
)

sdl_input_dep = declare_dependency(
    include_directories: inc,
    link_with: sdl_input_shared,
    dependencies: deps
)