# vkcl
A library written in C++ that is focused on making compute with Vulkan a lot less painless with performance in mind.

## How to Build
### Library Requirements:
- [Meson](http://mesonbuild.com/) Version 0.47 or higher
- Any C++ Compiler

### Test Requirements:
- [glslang](https://github.com/KhronosGroup/glslang) for compiling SPIR-V Shaders

### Manual Compilation:
After cloning vkcl, run this inside the vkcl directory:
```
meson build
cd build
ninja
```

### Build Options:
The following build options are available:
* enable_debug - Enables debugging symbols and Vulkan validation layers.
* enable_test - Compiles the test program, which can then be run with `meson test`.

These build options are off by default, but can be enabled like such:
```
meson configure build -Denable_debug=true -Denable_test=true
```

## License
[Simplified BSD License](LICENSE)
