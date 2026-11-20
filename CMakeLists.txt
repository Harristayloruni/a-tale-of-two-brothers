cmake_minimum_required(VERSION 3.22)
project(a_a_tale_of_two_brothers VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Put every .exe in build/bin
SET(OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/")
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY})

# SFML built from submodule as a static library
SET(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
SET(SFML_BUILD_NETWORK OFF CACHE BOOL "" FORCE)
SET(SFML_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory("lib/SFML")

# MinGW: put the C++ runtime inside the .exe (no extra DLLs to ship)
if(MINGW)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static-libgcc -static-libstdc++ -static")
endif()

# Engine library
file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS engine/*.cpp engine/*.hpp)
add_library(engine STATIC ${ENGINE_SOURCES})
target_include_directories(engine PUBLIC engine engine/components engine/ai)
target_link_libraries(engine PUBLIC sfml-graphics sfml-window sfml-system)

# The Game
file(GLOB_RECURSE GAME_SOURCES CONFIGURE_DEPENDS game/*.cpp game/*.hpp)
add_executable(a_tale_of_two_brothers ${GAME_SOURCES})
target_include_directories(a_tale_of_two_brothers PRIVATE game)
target_link_libraries(a_tale_of_two_brothers PRIVATE engine)
if(WIN32)
  target_link_libraries(a_tale_of_two_brothers PRIVATE sfml-main)
endif()

# set(SFML_INCS "lib/SFML/include")
# link_directories("${CMAKE_BINARY_DIR}/lib/SFML/lib")

# add_executable(game main.cpp)
# target_include_directories(game PRIVATE ${SFML_INCS})
# target_link_libraries(game sfml-graphics)
