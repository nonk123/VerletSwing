cmake_minimum_required(VERSION 3.28.0 FATAL_ERROR)

project(VerletSwing
    VERSION 0.1.0
    DESCRIPTION "Verlet Swing"
    LANGUAGES C CXX)
set(PROJECT_BUNDLE "dev.nonk.games.verlet") # not a standard variable but whatever

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS ON)

set(BUILD_SHARED_LIBS OFF)

include(FetchContent)
set(FETCHCONTENT_QUIET FALSE)

set(SDL_EMSCRIPTEN_PERSISTENT_PATH /storage CACHE INTERNAL "")
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
    GIT_REPOSITORY https://github.com/Schwungus/S_tructures.git
    GIT_TAG 76e7bc5b6c0604149d3a4cad01af3a8f1c8bb777
    EXCLUDE_FROM_ALL)

FetchContent_MakeAvailable(S_tructures)
list(APPEND LIBS S_tructures)

FetchContent_Declare(NutBlast
    GIT_REPOSITORY https://github.com/Schwungus/NutBlast.git
    GIT_TAG 1e23b99bf55186e87f66d65f74f7c503678daf66
    EXCLUDE_FROM_ALL)

FetchContent_MakeAvailable(NutBlast)
list(APPEND LIBS NutBlast)

if(EMSCRIPTEN)
    set(CMAKE_EXECUTABLE_SUFFIX ".html")
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

set(DEBUG $<CONFIG:Debug>)
set(DESKTOP $<NOT:$<BOOL:${EMSCRIPTEN}>>)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    ASSETS=$<IF:$<AND:${DEBUG},${DESKTOP}>,"${CMAKE_SOURCE_DIR}/assets/","assets/">
    DEBUG=${DEBUG})

if(EMSCRIPTEN)
    target_link_options(${PROJECT_NAME} PRIVATE
        --preload-file ${CMAKE_SOURCE_DIR}/assets@assets
        -sEXPORTED_RUNTIME_METHODS=HEAPF32,HEAPU32,HEAPU8
        -sNO_DISABLE_EXCEPTION_CATCHING
        -sASYNCIFY)
endif()
