cmake_minimum_required(VERSION 4.0.2 FATAL_ERROR)
project(LearnGL)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/include)
link_directories(${CMAKE_SOURCE_DIR}/lib)

file(GLOB_RECURSE SOURCES
    ${CMAKE_SOURCE_DIR}/src/*.cpp
    ${CMAKE_SOURCE_DIR}/src/*.h
    ${CMAKE_SOURCE_DIR}/src/*.hpp
)

set(SHADERS
    ${CMAKE_SOURCE_DIR}/src/fragShader.glsl
    ${CMAKE_SOURCE_DIR}/src/vertShader.glsl
)

add_executable(LearnGL ${SOURCES})

set_property(TARGET LearnGL PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$(OutDir)")

source_group("Shaders" FILES ${SHADERS})

target_sources(LearnGL PRIVATE ${SHADERS})

target_link_libraries(LearnGL
    glew32
    glfw3
    OpenAL32
    opengl32
)

get_target_property(LEARNGL_OUTPUT_DIR LearnGL RUNTIME_OUTPUT_DIRECTORY)
if(NOT LEARNGL_OUTPUT_DIR)

    set(LEARNGL_OUTPUT_DIR "$<TARGET_FILE_DIR:LearnGL>")
endif()

message(STATUS "LearnGL executable output directory: ${LEARNGL_OUTPUT_DIR}")

add_custom_command(TARGET LearnGL POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Copying DLLs and Shaders to ${LEARNGL_OUTPUT_DIR}..."
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/glew32.dll"
        "${CMAKE_SOURCE_DIR}/glfw3.dll"
        "${CMAKE_SOURCE_DIR}/OpenAL32.dll"
        "${CMAKE_SOURCE_DIR}/src/fragShader.glsl"
        "${CMAKE_SOURCE_DIR}/src/vertShader.glsl" 
        "${LEARNGL_OUTPUT_DIR}" 
    COMMAND ${CMAKE_COMMAND} -E echo "DLLs and Shaders copied."
    COMMAND ${CMAKE_COMMAND} -E remove_directory "${LEARNGL_OUTPUT_DIR}/Assets"
    COMMAND ${CMAKE_COMMAND} -E echo "Removing old Assets directory in ${LEARNGL_OUTPUT_DIR}/Assets"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/src/Assets"
        "${LEARNGL_OUTPUT_DIR}/Assets"
    COMMAND ${CMAKE_COMMAND} -E echo "Assets copied to ${LEARNGL_OUTPUT_DIR}/Assets"
    VERBATIM
)