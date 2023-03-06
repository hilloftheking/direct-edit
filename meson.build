project('directedit', 'c', 
    default_options: 'default_library=static')

sdl2_dep = dependency('sdl2')

sources = ['src/main.c', 'src/buffer.c', 'src/program.c']
executable('dedit', sources, dependencies: sdl2_dep)