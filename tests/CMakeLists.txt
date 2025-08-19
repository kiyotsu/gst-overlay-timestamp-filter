cmake_minimum_required(VERSION 3.14)

project(gst-overlay-timestamp-filter-tests LANGUAGES CXX C)

find_package(PkgConfig REQUIRED)
pkg_check_modules(GSTREAMER REQUIRED gstreamer-1.0 gstreamer-base-1.0)

# GoogleTestの取得
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
FetchContent_MakeAvailable(googletest)

# テスト対象のソースコードをインクルード
include_directories(${GSTREAMER_INCLUDE_DIRS})

# テスト実行ファイルの作成
add_executable(
  run_tests
  test_gst_overlay_timestamp_utils.cpp
  test_gst_overlay_timestamp_filter.cpp
)

# 親ディレクトリのターゲットgstoverlaytsを参照可能にする
add_dependencies(run_tests gstoverlayts)

# テスト対象のソースコードをインクルード
target_include_directories(run_tests PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/../src
  ${GSTREAMER_INCLUDE_DIRS}
)

# テスト対象のソースファイルをリンク
target_link_libraries(
  run_tests
  PRIVATE gstoverlayts gtest_main ${GSTREAMER_LIBRARIES})

# テストの追加
include(GoogleTest)
gtest_discover_tests(run_tests)