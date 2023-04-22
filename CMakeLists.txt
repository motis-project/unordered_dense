cmake_minimum_required(VERSION 3.12)
project("unordered_dense"
    VERSION 4.0.0
    DESCRIPTION "A fast & densely stored hashmap and hashset based on robin-hood backward shift deletion"
    HOMEPAGE_URL "https://github.com/martinus/unordered_dense")

include(GNUInstallDirs)

include(cmake/pkg.cmake)

# determine whether this is a standalone project or included by other projects
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
    set(_unordered_dense_is_toplevel_project TRUE)
else()
    set(_unordered_dense_is_toplevel_project FALSE)
endif()

add_library(unordered_dense INTERFACE)
add_library(unordered_dense::unordered_dense ALIAS unordered_dense)

target_include_directories(
    unordered_dense
    SYSTEM INTERFACE
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>)
target_link_libraries(unordered_dense INTERFACE cista)
target_compile_features(unordered_dense INTERFACE cxx_std_17)

add_executable(unordered_dense_example example/main.cpp)
target_link_libraries(unordered_dense_example PRIVATE unordered_dense)