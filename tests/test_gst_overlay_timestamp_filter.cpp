#include <gtest/gtest.h>
#include <gst/gst.h>

// GStreamerエレメントをテストするためのテストフィクスチャ
class GstOverlayTimestampFilterTest : public ::testing::Test {
protected:
    GstElement *pipeline;
    GstElement *overlay;

    // 各テストの開始前に呼ばれる
    void SetUp() override {
        // GStreamerを初期化
        gst_init(nullptr, nullptr);

        // テスト対象のエレメントを作成
        overlay = gst_element_factory_make("overlayts", "test_overlay");
        ASSERT_TRUE(overlay != nullptr) << "Failed to create 'overlayts' element. Check if the plugin is found.";

        // エレメントを保持するためのダミーパイプラインを作成
        pipeline = gst_pipeline_new("test-pipeline");
        gst_bin_add(GST_BIN(pipeline), overlay);
    }

    // 各テストの終了後に呼ばれる
    void TearDown() override {
        gst_object_unref(pipeline);
    }
};

// プロパティの設​​定と取得を検証するテストケース
TEST_F(GstOverlayTimestampFilterTest, PropertySetAndGet) {
    gchar *original_color, *retrieved_color;
    original_color = g_strdup("#123456");

    // "text-color" プロパティを設定
    g_object_set(G_OBJECT(overlay), "text-color", original_color, NULL);

    // "text-color" プロパティを再取得
    g_object_get(G_OBJECT(overlay), "text-color", &retrieved_color, NULL);

    // 設定した値と取得した値が一致することを確認
    EXPECT_STREQ(original_color, retrieved_color);

    g_free(original_color);
    g_free(retrieved_color);
}

// 状態遷移が正常に行えるかを検証するテストケース
TEST_F(GstOverlayTimestampFilterTest, StateChange) {
    ASSERT_EQ(gst_element_set_state(pipeline, GST_STATE_PLAYING), GST_STATE_CHANGE_SUCCESS);
    ASSERT_EQ(gst_element_set_state(pipeline, GST_STATE_NULL), GST_STATE_CHANGE_SUCCESS);
}
