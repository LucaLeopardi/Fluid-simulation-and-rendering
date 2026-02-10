# Real-time fluid simulation and rendering
Video showcase:

[![Application showcase video](https://img.youtube.com/vi/EEDOQ55whBs/0.jpg)](https://www.youtube.com/watch?v=EEDOQ55whBs)

## How to compile
- Install CUDA Toolkit (v12.9 or above).
- Install VS Code.
- Download Visual Studio Installer and install "Visual Studio Build Tools 2022 - C++ desktop applications" workload with components:
  - MSVC
  - Windows SDK
  - CMake tools
  - vcpkg package manager
- Install vcpkg via git and run setup script. No need for other setup steps.
- Install CMake (v4.0.3 or above).
- Install project's VS Code extensions (`.vscode/.extensions.json`).
- Edit `CMakePresets.json`: `toolchainFile` must point to `[VCPKG_INSTALL_FOLDER]/scripts/buildsystems/vcpkg.cmake`.
  If CMake cannot find the Cuda Toolkit installation, manually set the `cuda` specifier in the `toolset` field to point to your Toolkit folder: `[...]/NVIDIA GPU Computing Toolkit/CUDA/[VERSION]`.
  Set the `CMAKE_CUDA_COMPILER` field to point to: `[...]/NVIDIA GPU Computing Toolkit/CUDA/[VERSION]/bin/nvcc.exe`.
- In VS Code, in CMake side panel:
  - Run Configure and select Visual Studio Build Tools 2022 Preset.
  - Run Build (Release or Debug preset). 
  - Launch debugger or .exe in chosen preset's output folder.
## Controls 
- **Right mouse:** Rotate camera
- **WASD:** Move camera horizontally
- **Q/E:** Move camera vertically
- **SHIFT:** Fast camera movement
- **ENTER:** Toggle UI
- **ESC:** Exit application
---
Final submission for courses GPU Computing and Real-time Graphics Programming (Università degli studi di Milano)
