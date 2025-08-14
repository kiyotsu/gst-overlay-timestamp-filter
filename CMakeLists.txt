cmake_minimum_required(VERSION 3.10)

project(gst-overlay-timestamp-filter VERSION 0.0.1 LANGUAGES C)

set(CMAKE_SHARED_LIBRARY_SUFFIX ".so")

find_package(PkgConfig REQUIRED)

pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0 gstreamer-base-1.0 gstreamer-video-1.0 cairo)

set(PLUGIN_SOURCES
    src/gst_overlay_timestamp_filter.c
    src/gst_overlay_timestamp_utils.c
)

add_library(gstoverlayts SHARED ${PLUGIN_SOURCES})

target_include_directories(gstoverlayts PRIVATE ${GSTREAMER_INCLUDE_DIRS})

target_link_libraries(gstoverlayts PRIVATE ${GSTREAMER_LIBRARIES})

install(TARGETS gstoverlayts
        LIBRARY DESTINATION "${GSTREAMER_PLUGINS_INSTALL_DIR}")