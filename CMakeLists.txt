cmake_minimum_required(VERSION 3.27.0 FATAL_ERROR)

project(VerletSwing VERSION 0.1.0
    LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

set(BUILD_SHARED_LIBS OFF)

# bypass "emcc: error: sdl3 port is still experimental [-Wexperimental] [-Werror]"
set(CMAKE_COMPILE_WARNING_AS_ERROR $<NOT:$<BOOL:${EMSCRIPTEN}>>)

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)

set(SDL_SHARED OFF CACHE INTERNAL "")
set(SDL_STATIC ON CACHE INTERNAL "")
set(SDL_TEST_LIBRARY OFF CACHE INTERNAL "")
set(SDL_RENDER ON CACHE INTERNAL "")
set(SDL_GPU OFF CACHE INTERNAL "")
set(SDL_AUDIO ON CACHE INTERNAL "")
set(SDL_CAMERA OFF CACHE INTERNAL "")
set(SDL_JOYSTICK OFF CACHE INTERNAL "")
set(SDL_HAPTIC OFF CACHE INTERNAL "")

FetchContent_Declare(SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG release-3.4.8
    EXCLUDE_FROM_ALL)

FetchContent_MakeAvailable(SDL3)
list(APPEND LIBS SDL3::SDL3-static)

FetchContent_Declare(S_tructures
    GIT_REPOSITORY https://github.com/Schwungus/S_tructures
    GIT_TAG b867668c4b991bb2482d34a98416a3a106447544
    EXCLUDE_FROM_ALL)

FetchContent_MakeAvailable(S_tructures)
list(APPEND LIBS S_tructures)

if(EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
else()
    set(DEBUG_ASSETS_DIR ${CMAKE_SOURCE_DIR}/assets)
endif()

file(GLOB_RECURSE SOURCES ${CMAKE_SOURCE_DIR}/src/*.c)
file(GLOB_RECURSE HEADERS ${CMAKE_SOURCE_DIR}/src/*.h)

set(CONFIGURE_FILE ${CMAKE_BINARY_DIR}/generated/cmake.h)
configure_file(${CMAKE_SOURCE_DIR}/src/cmake.h.in ${CONFIGURE_FILE} @ONLY)
list(APPEND HEADERS ${CONFIGURE_FILE})

add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${HEADERS})
target_include_directories(${PROJECT_NAME}
    PRIVATE ${CMAKE_SOURCE_DIR}/include ${CMAKE_BINARY_DIR}/generated)
target_link_libraries(${PROJECT_NAME} PRIVATE ${LIBS})

if(EMSCRIPTEN)
    target_link_options(${PROJECT_NAME} PRIVATE --preload-file ${CMAKE_SOURCE_DIR}/assets@assets)
endif()
