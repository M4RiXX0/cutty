cmake_minimum_required(VERSION 3.16)

project(Cutty
    VERSION 1.0.0
    DESCRIPTION "Simple FFmpeg wrapper"
    LANGUAGES C CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /utf-8)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

file(GLOB SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.c"
)

add_executable(cutty ${SOURCES})

target_include_directories(cutty PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_definitions(cutty PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()