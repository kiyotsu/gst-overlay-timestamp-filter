#include <gtest/gtest.h>
#include <stdlib.h>

extern "C" {
#include "gst_overlay_timestamp_utils.h"
}

// color_hex_string_to_rgb のテスト
TEST(ColorUtilsTest, HexToRgbValid) {
    struct RGB color;
    ASSERT_TRUE(color_hex_string_to_rgb(&color, "#FFFFFF"));
    EXPECT_EQ(color.red, 255);
    EXPECT_EQ(color.green, 255);
    EXPECT_EQ(color.blue, 255);

    ASSERT_TRUE(color_hex_string_to_rgb(&color, "#000000"));
    EXPECT_EQ(color.red, 0);
    EXPECT_EQ(color.green, 0);
    EXPECT_EQ(color.blue, 0);

    ASSERT_TRUE(color_hex_string_to_rgb(&color, "#FF5733"));
    EXPECT_EQ(color.red, 0xFF);
    EXPECT_EQ(color.green, 0x57);
    EXPECT_EQ(color.blue, 0x33);
}

TEST(ColorUtilsTest, HexToRgbInvalid) {
    struct RGB color;
    EXPECT_FALSE(color_hex_string_to_rgb(NULL, "#FFFFFF"));
    EXPECT_FALSE(color_hex_string_to_rgb(&color, NULL));
    EXPECT_FALSE(color_hex_string_to_rgb(&color, "FFFFFF"));   // Missing #
    EXPECT_FALSE(color_hex_string_to_rgb(&color, "#FFFFF"));    // Too short
    EXPECT_FALSE(color_hex_string_to_rgb(&color, "#FFFFFFF"));  // Too long
    EXPECT_FALSE(color_hex_string_to_rgb(&color, "#GGGGGG"));   // Invalid hex chars
}

// rgb_to_color_hex_string のテスト
TEST(ColorUtilsTest, RgbToHexValid) {
    struct RGB color = {255, 87, 51};
    char hex_buffer[8];
    ASSERT_TRUE(rgb_to_color_hex_string(&color, hex_buffer, sizeof(hex_buffer)));
    EXPECT_STREQ(hex_buffer, "#FF5733");
}

TEST(ColorUtilsTest, RgbToHexInvalid) {
    struct RGB color = {255, 255, 255};
    char hex_buffer[7];  // Too small buffer
    EXPECT_FALSE(rgb_to_color_hex_string(&color, hex_buffer, sizeof(hex_buffer)));
    EXPECT_FALSE(rgb_to_color_hex_string(&color, NULL, 8));
}

// strfmttime のテスト
TEST(TimeUtilsTest, Strfmttime) {
    // テストの再現性を確保するため、タイムゾーンをUTCに固定
    setenv("TZ", "UTC", 1);
    tzset();

    char buf[64];
    struct timeval tv;

    // 2023-10-27 09:30:50.123 UTC (Epoch: 1698399050)
    tv.tv_sec = 1698399050;
    tv.tv_usec = 123000;

    int len = strfmttime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S.%q", &tv);
    EXPECT_EQ(len, 23);
    EXPECT_STREQ(buf, "2023-10-27 09:30:50.123");

    len = strfmttime(buf, sizeof(buf), "%%Y", &tv);
    EXPECT_EQ(len, 2);
    EXPECT_STREQ(buf, "%Y");
}

TEST(TimeUtilsTest, StrfmttimeBufferTooSmall) {
    // テストの再現性を確保するため、タイムゾーンをUTCに固定
    setenv("TZ", "UTC", 1);
    tzset();

    char buf[10];
    struct timeval tv;
    tv.tv_sec = 1698399050;
    tv.tv_usec = 123000;

    int len = strfmttime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S.%q", &tv);
    EXPECT_EQ(len, 9);
    EXPECT_STREQ(buf, "2023-10-2");
}

// calculate_aligned_position のテスト
TEST(AlignUtilsTest, CalculateAlignedPosition) {
    int x, y;
    unsigned int container_w = 1920, container_h = 1080;
    unsigned int object_w = 200, object_h = 50;
    int margin = 10;

    calculate_aligned_position(TEXT_ALIGN_TOP_LEFT, container_w, container_h, object_w, object_h,
                               margin, &x, &y);
    EXPECT_EQ(x, margin);
    EXPECT_EQ(y, margin + object_h);

    calculate_aligned_position(TEXT_ALIGN_TOP_CENTER, container_w, container_h, object_w,
                               object_h, margin, &x, &y);
    EXPECT_EQ(x, (container_w - object_w) / 2);
    EXPECT_EQ(y, margin + object_h);

    calculate_aligned_position(TEXT_ALIGN_CENTER, container_w, container_h, object_w, object_h,
                               margin, &x, &y);
    EXPECT_EQ(x, (container_w - object_w) / 2);
    EXPECT_EQ(y, container_h / 2);

    calculate_aligned_position(TEXT_ALIGN_BOTTOM_RIGHT, container_w, container_h, object_w,
                               object_h, margin, &x, &y);
    EXPECT_EQ(x, container_w - object_w - margin);
    EXPECT_EQ(y, container_h - margin);
}