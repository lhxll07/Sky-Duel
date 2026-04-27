cmake_minimum_required(VERSION 3.20)

project(SkyDuel LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT WIN32)
    message(FATAL_ERROR "Sky Duel currently uses bundled Windows GLFW binaries.")
endif()

find_package(OpenGL REQUIRED)

set(GLFW_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/glfw")
if(MSVC)
    set(GLFW_LIB_DIR "${GLFW_DIR}/lib-vc2022")
    set(GLFW_IMPLIB "${GLFW_LIB_DIR}/glfw3.lib")
elseif(MINGW)
    set(GLFW_LIB_DIR "${GLFW_DIR}/lib-mingw-w64")
    set(GLFW_IMPLIB "${GLFW_LIB_DIR}/libglfw3dll.a")
else()
    message(FATAL_ERROR "Unsupported compiler. Use MSVC or MinGW on Windows.")
endif()
set(GLFW_DLL "${GLFW_LIB_DIR}/glfw3.dll")

add_library(glfw3 SHARED IMPORTED GLOBAL)
set_target_properties(glfw3 PROPERTIES
    IMPORTED_IMPLIB "${GLFW_IMPLIB}"
    IMPORTED_LOCATION "${GLFW_DLL}"
    INTERFACE_INCLUDE_DIRECTORIES "${GLFW_DIR}/include"
)

set(IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/external/imgui")
add_library(imgui STATIC
    "${IMGUI_DIR}/imgui.cpp"
    "${IMGUI_DIR}/imgui_draw.cpp"
    "${IMGUI_DIR}/imgui_tables.cpp"
    "${IMGUI_DIR}/imgui_widgets.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_glfw.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp"
)
target_include_directories(imgui PUBLIC
    "${IMGUI_DIR}"
    "${IMGUI_DIR}/backends"
    "${GLFW_DIR}/include"
)
target_link_libraries(imgui PUBLIC glfw3 OpenGL::GL)

add_executable(sky_duel
    "src/main.cpp"
    "external/glad/src/glad.c"
)
set_target_properties(sky_duel PROPERTIES OUTPUT_NAME "sky-duel")
target_include_directories(sky_duel PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/external/glad/include"
)
target_link_libraries(sky_duel PRIVATE imgui glfw3 OpenGL::GL)

add_custom_command(TARGET sky_duel POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E copy_if_different
        "${GLFW_DLL}"
        "$<TARGET_FILE_DIR:sky_duel>"
    COMMAND "${CMAKE_COMMAND}" -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/pictures"
        "$<TARGET_FILE_DIR:sky_duel>/pictures"
)
