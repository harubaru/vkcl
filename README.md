# vkcl
A library written in C++ that is focused on making compute with Vulkan a lot less painless with performance in mind.

## How to Build
### Requirements:
- [Meson](http://mesonbuild.com/) Version 0.47 or higher
- [glslang](https://github.com/KhronosGroup/glslang) for compiling SPIR-V Shaders
- Any C++ Compiler

### Manual Compilation:
After cloning vkcl, run this inside the vkcl directory:
```
meson build
cd build
ninja
```

## License
[Simplified BSD License](LICENSE)
