cmake_minimum_required(VERSION 3.23...3.28 FATAL_ERROR)

project(notes LANGUAGES C)

set(CMAKE_BUILD_WITH_INSTALL_RPATH ON)

if (APPLE)
    list(APPEND CMAKE_INSTALL_RPATH @loader_path/../lib)
elseif(UNIX)
    list(APPEND CMAKE_INSTALL_RPATH $ORIGIN/../lib)
endif()

add_subdirectory(src/bank)
add_subdirectory(src/barrier)
add_subdirectory(src/hello)
add_subdirectory(src/pipeline)
add_subdirectory(src/warehouse)
