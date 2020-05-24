test_vkcl_deps = [vkcl_dep]

shader_src = files([
	'test_mul.comp'
])

shader_out = [
	'test_mul.spv'
]

spirvcomp = find_program('glslangValidator')

custom_target(
	'Compute Shaders', 
	build_by_default : true,
	build_always_stale : true, 
	output : shader_out, 
	input : shader_src, 
	command : [spirvcomp, '@INPUT@', '-V', '-o', '@OUTPUT@']
)

vk_test = executable('vk_test', files('vk_test.cpp'), cpp_args : [vkcl_cpp_compiler_flags], dependencies: test_vkcl_deps)
test('VKCL Test Program', vk_test, workdir : meson.build_root())
