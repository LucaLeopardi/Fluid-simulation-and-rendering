cmake_minimum_required(VERSION 4.0.3)

project(GPUCRTGP LANGUAGES 
	C 
	CXX 
#	CUDA
)

#find_package(CUDAToolkit REQUIRED)
find_package(glad REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(imgui REQUIRED)
find_package(Stb REQUIRED)

set(SOURCES
	src/main.cpp
	src/utils/stb_image.cpp
	src/utils/Mesh.cpp
	src/utils/Texture.cpp
	src/utils/Cubemap.cpp
	src/Shader.cpp
	src/Camera.cpp
	src/Model.cpp
	src/UIRenderer.cpp
	src/Renderer.cpp
	src/MPM/MPMSimulation.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE 
	include
	${Stb_INCLUDE_DIR}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
#	CUDA::cudart
	glad::glad 
	glfw 
	glm::glm 
	imgui::imgui
)

# Only for architectures of current machine - great for sharing source code, might need to specify arch to share built app.
# Only supported in CMake 3.24+
#set_property(TARGET ${PROJECT_NAME} PROPERTY CUDA_ARCHITECTURES native)

# Enable OpenGL Debugging Context in Debug build
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:ENABLE_GL_DEBUG_CONTEXT>
)

add_compile_options(    
	# C++ compiler optimization in Release
    $<$<CONFIG:Release>:$<$<COMPILE_LANGUAGE:CXX>:-O3 -march=native>>
	# CUDA compiler optimization in Release
#  	$<$<CONFIG:Release>:$<$<COMPILE_LANGUAGE:CUDA>:-Xcompiler=-O3;-Xcompiler=-march=native>>
	# Suppress warning
#	$<$<COMPILE_LANGUAGE:CUDA>:-Wno-deprecated-gpu-targets >
)

# Copy shader files to the build directory
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
)