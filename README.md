# Vulkan Ze

Vulkan Ze is a experimental, unfinished, 3D engine based on Vulkan 1.3 made for **learning Vulkan**. 

**Current state**
- Scene tree & nodes using classical OO approch
- Depth pre-pass
- glTF scene loading (diffuse, specular & normal maps)
- Directionals, omnidirectionals & spots lights
- Simple shadows for directionals & spots ligths
- Skybox using cubemap
- HDR tone mapping
- Jolt Physics for Static & Rigid bodies

**Vulkan extensions and third parties dependencies**
- Dynamic rendering (VK_KHR_dynamic_rendering)
- Shader object (VK_EXT_shader_object)
- volk https://github.com/zeux/volk
- VulkanMemoryAllocator https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
- GLFW https://www.glfw.org/
- GLM https://github.com/g-truc/glm
- stb https://github.com/nothings/stb
- fastgltf https://github.com/spnda/fastgltf
- Dear Imgui https://www.dearimgui.com/
- Jolt Physics https://github.com/jrouwe/JoltPhysics

**Building tools needed**
- GCC/MINGW 11+ (C++ 23)
- CMake 3.22+
- Vulkan SDK 1.3+ 
- Git

**Building**
- On Microsoft Windows create a `.env.cmake` file with `set(VULKAN_SDK_PATH=c:\\path\\to\\vulkan\\version)` (for example `C:\\VulkanSDK\\1.3.280.0`)
- `cmake -B build -D CMAKE_BUILD_TYPE=Release` (add `-D GLFW_BUILD_WAYLAND=false` on Linux)
- `cmake --build build`

Released under the [MIT license](https://raw.githubusercontent.com/HenriMichelon/zero_zero/main/LICENSE.txt).
