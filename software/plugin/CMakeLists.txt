cmake_minimum_required(VERSION 3.16...3.21)

project(oss-plugin VERSION 1.0.1)
add_library(${CMAKE_PROJECT_NAME} MODULE main/plugin-main.cpp)


set(PLUGIN_AUTHOR "ZY4N")
set(MACOS_BUNDLEID "com.zy4n.${CMAKE_PROJECT_NAME}")
set(LINUX_MAINTAINER_EMAIL "me@zy4n.com")


target_sources(${CMAKE_PROJECT_NAME} PRIVATE main/plugin-main.cpp)
add_subdirectory(${CMAKE_SOURCE_DIR}/main)

set(COMMON_BINARY_DIR ${CMAKE_BINARY_DIR}/common)
add_subdirectory(${CMAKE_SOURCE_DIR}/../common ${COMMON_BINARY_DIR})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -Werror -std=c++23")


find_package(libobs REQUIRED)
include(cmake/ObsPluginHelpers.cmake)

if(NOT MSVC)
    add_definitions(-D_DEBUG)
endif()

# On non-Debug builds cmake automatically defines NDEBUG, so we explicitly undefine it:
if(NOT uppercase_CMAKE_BUILD_TYPE STREQUAL "DEBUG")
    # NOTE: use `add_compile_options` rather than `add_definitions` since
    # `add_definitions` does not support generator expressions.
    add_compile_options($<$<OR:$<COMPILE_LANGUAGE:C>,$<COMPILE_LANGUAGE:CXX>>:-UNDEBUG>)

    # Also remove /D NDEBUG to avoid MSVC warnings about conflicting defines.
    foreach (flags_var_to_scrub
            CMAKE_CXX_FLAGS_RELEASE
            CMAKE_CXX_FLAGS_RELWITHDEBINFO
            CMAKE_CXX_FLAGS_MINSIZEREL
            CMAKE_C_FLAGS_RELEASE
            CMAKE_C_FLAGS_RELWITHDEBINFO
            CMAKE_C_FLAGS_MINSIZEREL)
        string (REGEX REPLACE "(^| )[/-]D *NDEBUG($| )" " "
                "${flags_var_to_scrub}" "${${flags_var_to_scrub}}")
    endforeach()
endif()


target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE OBS::libobs)
target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE  crypto)
include_directories(${OPENSSL_INCLUDE_DIR})


# --- End of section ---

# --- Windows-specific build settings and tasks ---
if(OS_WINDOWS)
  configure_file(cmake/bundle/windows/installer-Windows.iss.in
                 ${CMAKE_BINARY_DIR}/installer-Windows.generated.iss)

  configure_file(cmake/bundle/windows/resource.rc.in ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.rc)
  target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}.rc)

  if(MSVC)
    target_compile_options(${CMAKE_PROJECT_NAME} PRIVATE /W4)
  endif()
  # --- End of section ---

  # -- macOS specific build settings and tasks --
if(OS_MACOS)
  configure_file(cmake/bundle/macos/installer-macos.pkgproj.in
                 ${CMAKE_BINARY_DIR}/installer-macos.generated.pkgproj)

  set(MACOSX_PLUGIN_GUI_IDENTIFIER "${MACOS_BUNDLEID}")
  set(MACOSX_PLUGIN_BUNDLE_VERSION "${CMAKE_PROJECT_VERSION}")
  set(MACOSX_PLUGIN_SHORT_VERSION_STRING "1")
endif()

endif()
# --- End of section ---

setup_plugin_target(${CMAKE_PROJECT_NAME})

