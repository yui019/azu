project('Azu - 2D graphics library', 'cpp',
  default_options : ['warning_level=3', 'cpp_std=c++20'])

subdir('src')

inc = include_directories('include')

cmake = import('cmake')

vk_bootstrap_proj = cmake.subproject('vk-bootstrap')
vk_bootstrap_dep = vk_bootstrap_proj.dependency('vk-bootstrap')

vma_proj = cmake.subproject('VulkanMemoryAllocator')
vma_dep = vma_proj.dependency('VulkanMemoryAllocator')

deps = [
  dependency('sdl2'),
  dependency('vulkan'),
  vk_bootstrap_dep,
  vma_dep
]


executable('azu',
  sources: src,
  include_directories: inc,
  dependencies: deps
)
