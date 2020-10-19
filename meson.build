project('vkcl', ['c', 'cpp'], version : 'v0.10', meson_version : '>= 0.47')

cpu_family = target_machine.cpu_family()
platform = target_machine.system()

vkcl_compiler = meson.get_compiler('cpp')
vkcl_include_path = include_directories('./include')
vkcl_cpp_compiler_flags = '-std=c++14'
vkcl_compiler_flags = ''

if get_option('enable_debug')
	vkcl_compiler_flags = '-D=_DEBUG -ggdb -O2'
else
	vkcl_compiler_flags = '-O2'
endif

subdir('src')

src = util_src + vk_src

vkcl_lib = static_library('vkcl', src, cpp_args : [vkcl_cpp_compiler_flags, vkcl_compiler_flags], include_directories : [vkcl_include_path])
vkcl_dep = declare_dependency(link_with : [vkcl_lib], include_directories : [vkcl_include_path])

if get_option('enable_test')
	subdir('test')
endif
