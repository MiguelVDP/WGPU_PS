cmake_minimum_required(VERSION 3.0...3.24)
project(WGPU_PS)

set(CMAKE_CXX_STANDARD 17)

include_directories(
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)
add_executable(WGPU_PS
        #Header files
        include/pipelineData.h
        #Source files
        main.cpp
        src/pipelineData.cpp
        src/implementations.cpp)

target_link_libraries(WGPU_PS PRIVATE glfw webgpu glfw3webgpu)

set_target_properties(WGPU_PS PROPERTIES
        CXX_STANDARD 17
        COMPILE_WARNING_AS_ERROR ON
        )

if (MSVC)
    target_compile_options(WGPU_PS PRIVATE /W4)
else()
    target_compile_options(WGPU_PS PRIVATE -Wall -Wextra -pedantic)
endif()

if(XCODE)
    set_target_properties(WGPU_PS PROPERTIES
            XCODE_GENERATE_SCHEME ON
            XCODE_SCHEME_ENABLE_GPU_FRAME_CAPTURE_MODE "Metal")
endif()

add_subdirectory(glfw)
add_subdirectory(WebGPU-distribution-wgpu)
add_subdirectory(glfw3webgpu)
target_include_directories(WGPU_PS PRIVATE .)

# The application's binary must find wgpu.dll or libwgpu.so at runtime,
# so we automatically copy it (it's called WGPU_RUNTIME_LIB in general)
# next to the binary.
target_copy_webgpu_binaries(WGPU_PS)

# We add an option to enable different settings when developing the app than
# when distributing it.
option(DEV_MODE "Set up development helper settings" ON)

if(DEV_MODE)
    # In dev mode, we load resources from the source tree, so that when we
    # dynamically edit resources (like shaders), these are correctly
    # versionned.
    target_compile_definitions(WGPU_PS PRIVATE
            RESOURCE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/resources"
    )
else()
    # In release mode, we just load resources relatively to wherever the
    # executable is launched from, so that the binary is portable
    target_compile_definitions(WGPU_PS PRIVATE
            RESOURCE_DIR="./resources"
    )
endif()