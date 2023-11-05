project('Azu - 2D graphics library', 'cpp',
  default_options : ['warning_level=3', 'cpp_std=c++17'])

subdir('src')

inc = include_directories('include')

deps = [
  dependency('sdl2')
]

executable('azu',
  sources: src,
  include_directories: inc,
  dependencies: deps
)
