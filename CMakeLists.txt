cmake_minimum_required(VERSION 3.10)
project(MasterThesis)

set(CMAKE_CXX_STANDARD 17)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# build google test
if(NOT NO_GTEST)
    configure_file(CMakeLists.txt.in googletest-download/CMakeLists.txt)
    execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
            RESULT_VARIABLE result
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download)
    if(result)
        message(FATAL_ERROR "CMake step for googletest failed: ${result}")
    endif()
    execute_process(COMMAND ${CMAKE_COMMAND} --build .
            RESULT_VARIABLE result
            WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download)
    if(result)
        message(FATAL_ERROR "Build step for googletest failed: ${result}")
    endif()

    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

    add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/googletest-src
            ${CMAKE_CURRENT_BINARY_DIR}/googletest-build
            EXCLUDE_FROM_ALL)
endif()

include_directories(src)
include_directories(include)

if(CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 8.0)
    set(THREADS_PREFER_PTHREAD_FLAG ON)
endif()

find_package(Threads REQUIRED)

file(GLOB_RECURSE HEADER_FILES CONFIGURE_DEPENDS src/**/*.hpp)
file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS src/**/*.cpp)

add_executable(MasterThesis src/main.cpp ${SOURCE_FILES} ${HEADER_FILES})
add_executable(QueryGenerator src/queryGenerator.cpp ${SOURCE_FILES} ${HEADER_FILES})
add_executable(LCRQueryGenerator src/lcrQueryGenerator.cpp ${SOURCE_FILES} ${HEADER_FILES})
add_executable(GraphUtilities src/graphUtilities.cpp ${SOURCE_FILES} ${HEADER_FILES})

if(NOT NO_GTEST)
    file(GLOB_RECURSE TEST_FILES CONFIGURE_DEPENDS test/**/*.cpp test/*.cpp)
    add_executable(Tests ${TEST_FILES} ${SOURCE_FILES})
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
        message("Enabling Clang LTO for release build")

        set_target_properties(MasterThesis PROPERTIES COMPILE_FLAGS "-flto")
        set_target_properties(QueryGenerator PROPERTIES COMPILE_FLAGS "-flto")
        set_target_properties(LCRQueryGenerator PROPERTIES COMPILE_FLAGS "-flto")
        set_target_properties(GraphUtilities PROPERTIES COMPILE_FLAGS "-flto")

        if(NOT NO_GTEST)
            set_target_properties(Tests PROPERTIES COMPILE_FLAGS "-flto")
        endif()
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
        message("Enabling G++ LTO for release build")

        set_target_properties(MasterThesis PROPERTIES COMPILE_FLAGS "-flto")
        set_target_properties(QueryGenerator PROPERTIES COMPILE_FLAGS "-flto")
        set_target_properties(LCRQueryGenerator PROPERTIES COMPILE_FLAGS "-flto")
        set_target_properties(GraphUtilities PROPERTIES COMPILE_FLAGS "-flto")

        if(NOT NO_GTEST)
            set_target_properties(Tests PROPERTIES COMPILE_FLAGS "-flto")
        endif()
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
        message("Enabling MSVC-style LTO for release build")

        set_target_properties(MasterThesis PROPERTIES COMPILE_FLAGS "/GL")
        set_target_properties(MasterThesis PROPERTIES LINK_FLAGS "/LTCG")

        set_target_properties(QueryGenerator PROPERTIES COMPILE_FLAGS "/GL")
        set_target_properties(QueryGenerator PROPERTIES LINK_FLAGS "/LTCG")

        set_target_properties(LCRQueryGenerator PROPERTIES COMPILE_FLAGS "/GL")
        set_target_properties(LCRQueryGenerator PROPERTIES LINK_FLAGS "/LTCG")

        set_target_properties(GraphUtilities PROPERTIES COMPILE_FLAGS "/GL")
        set_target_properties(GraphUtilities PROPERTIES LINK_FLAGS "/LTCG")

        if(NOT NO_GTEST)
            set_target_properties(Tests PROPERTIES COMPILE_FLAGS "/GL")
            set_target_properties(Tests PROPERTIES LINK_FLAGS "/LTCG")
        endif()
    endif()
endif()

if(NOT NO_PRECOMPILED_HEADER)
    target_precompile_headers(MasterThesis PUBLIC src/precompiled.hpp)

    target_precompile_headers(QueryGenerator REUSE_FROM MasterThesis)
    target_precompile_headers(LCRQueryGenerator REUSE_FROM MasterThesis)
    target_precompile_headers(GraphUtilities REUSE_FROM MasterThesis)
else()
    if(MSVC)
        add_definitions(/FI"precompiled.hpp")
    else()
        # GCC or Clang
        add_definitions(-include precompiled.hpp)
    endif()
endif()

if(NOT NO_GTEST)
    if(NOT NO_PRECOMPILED_HEADER)
    else()
        target_precompile_headers(Tests REUSE_FROM MasterThesis)
    endif()

    target_link_libraries(Tests PRIVATE gtest gtest_main Threads::Threads)
endif()

target_link_libraries(MasterThesis PRIVATE  Threads::Threads)
target_link_libraries(QueryGenerator PRIVATE  Threads::Threads)
target_link_libraries(LCRQueryGenerator PRIVATE  Threads::Threads)
target_link_libraries(GraphUtilities PRIVATE  Threads::Threads)