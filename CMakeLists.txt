cmake_minimum_required(VERSION 3.10)

project(gst-overlay-timestamp-filter VERSION 0.0.1 LANGUAGES C)

option(BUILD_TEST "Build the unit tests" OFF)
option(ENABLE_COVERAGE "Enable coverage reporting (for GCC/Clang)" OFF)

set(CMAKE_SHARED_LIBRARY_SUFFIX ".so")

find_package(PkgConfig REQUIRED)

pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0 gstreamer-base-1.0 gstreamer-video-1.0 cairo)

if(ENABLE_COVERAGE)
  if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    message(STATUS "Coverage enabled. Adding --coverage flag.")
    add_compile_options(--coverage)
    add_link_options(--coverage)
  else()
    message(WARNING "Coverage reporting is only supported for GCC and Clang. Disabling.")
  endif()
endif()

set(PLUGIN_SOURCES
    src/gst_overlay_timestamp_filter.c
    src/gst_overlay_timestamp_utils.c
)

add_library(gstoverlayts SHARED ${PLUGIN_SOURCES})

target_include_directories(gstoverlayts PRIVATE ${GSTREAMER_INCLUDE_DIRS})

target_link_libraries(gstoverlayts PRIVATE ${GSTREAMER_LIBRARIES})

install(TARGETS gstoverlayts
        LIBRARY DESTINATION "${GSTREAMER_PLUGINS_INSTALL_DIR}")


if(BUILD_TEST)
  enable_testing()
  add_subdirectory(tests)
endif()