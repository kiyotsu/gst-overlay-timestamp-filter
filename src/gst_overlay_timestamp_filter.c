#include "gst_overlay_timestamp_filter.h"

#include <cairo/cairo.h>
#include <gst/video/video.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>

#ifndef PACKAGE
#define PACKAGE "overlayts"
#endif

#define GST_CAT_DEFAULT gst_overlay_timestamp_filter_debug
GST_DEBUG_CATEGORY_STATIC(GST_CAT_DEFAULT);

G_DEFINE_TYPE(GstOverlayTimestampFilter, gst_overlay_timestamp_filter, GST_TYPE_BASE_TRANSFORM)

static void gst_overlay_timestamp_filter_class_init(GstOverlayTimestampFilterClass *klass);
static void gst_overlay_timestamp_filter_init(GstOverlayTimestampFilter *filter);
static void gst_overlay_timestamp_filter_set_property(GObject *object, guint prop_id,
                                                      const GValue *value, GParamSpec *pspec);
static void gst_overlay_timestamp_filter_get_property(GObject *object, guint prop_id, GValue *value,
                                                      GParamSpec *pspec);
static gboolean gst_overlay_timestamp_filter_start(GstBaseTransform *trans);
static gboolean gst_overlay_timestamp_filter_set_caps(GstBaseTransform *trans, GstCaps *incaps,
                                                      GstCaps *outcaps);
static GstFlowReturn gst_overlay_timestamp_filter_transform_ip(GstBaseTransform *trans,
                                                               GstBuffer *buf);
static gboolean overlay_timestamp_plugin_init(GstPlugin *plugin);

static GType gst_text_alignment_get_type(void) {
    static GType alignment_type = 0;
    static const GEnumValue alignment_values[] = {
        {TEXT_ALIGN_TOP_LEFT, "Top-Left (左上)", "top-left"},
        {TEXT_ALIGN_TOP_CENTER, "Top-Center (中央上)", "top-center"},
        {TEXT_ALIGN_TOP_RIGHT, "Top-Right (右上)", "top-right"},
        {TEXT_ALIGN_CENTER_LEFT, "Center-Left (中央左)", "center-left"},
        {TEXT_ALIGN_CENTER, "Center (中央)", "center"},
        {TEXT_ALIGN_CENTER_RIGHT, "Center-Right (中央右)", "center-right"},
        {TEXT_ALIGN_BOTTOM_LEFT, "Bottom-Left (左下)", "bottom-left"},
        {TEXT_ALIGN_BOTTOM_CENTER, "Bottom-Center (中央下)", "bottom-center"},
        {TEXT_ALIGN_BOTTOM_RIGHT, "Bottom-Right (右下)", "bottom-right"},
        {0, NULL, NULL}};

    if (!alignment_type) {
        alignment_type = g_enum_register_static("TextAlignment", alignment_values);
    }
    return alignment_type;
}

static void gst_overlay_timestamp_filter_class_init(GstOverlayTimestampFilterClass *klass) {
    GObjectClass *gobject_class = G_OBJECT_CLASS(klass);
    GstElementClass *element_class = GST_ELEMENT_CLASS(klass);
    GstBaseTransformClass *base_transform_class = GST_BASE_TRANSFORM_CLASS(klass);

    gst_element_class_set_static_metadata(element_class, "OverlayTimestampFilter", "Filter",
                                          "Overlay timestamp filter", "t_yone");

    gobject_class->set_property = GST_DEBUG_FUNCPTR(gst_overlay_timestamp_filter_set_property);
    gobject_class->get_property = GST_DEBUG_FUNCPTR(gst_overlay_timestamp_filter_get_property);

    g_object_class_install_property(
        gobject_class, PROP_TEXT_COLOR,
        g_param_spec_string("text-color", "Text Color",
                            "Text overlay color in hex format (#RRGGBB)", DEFAULT_TEXT_COLOR,
                            G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

    g_object_class_install_property(
        gobject_class, PROP_OUTLINE_COLOR,
        g_param_spec_string("outline-color", "Outline Color",
                            "Text overlay outline color in hex format (#RRGGBB)",
                            DEFAULT_OUTLINE_COLOR, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

    g_object_class_install_property(
        gobject_class, PROP_TIMESTAMP_FORMAT,
        g_param_spec_string("format", "Timestamp Format", "Text overlay timestamp format",
                            DEFAULT_TIMESTAMP_FORMAT, G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
    g_object_class_install_property(
        gobject_class, PROP_TEXT_ALIGN,
        g_param_spec_enum("text-align", "Text Alignment", "Position of the text overlay",
                          gst_text_alignment_get_type(), DEFAULT_TEXT_ALIGN,
                          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

    GST_DEBUG_CATEGORY_INIT(GST_CAT_DEFAULT, "overlayts", 0, "Overlay timestamp filter debug");

    GstCaps *caps = gst_caps_from_string("video/x-raw, format=(string){BGRA}");
    GstPadTemplate *templ;

    templ = gst_pad_template_new("sink", GST_PAD_SINK, GST_PAD_ALWAYS, caps);
    gst_element_class_add_pad_template(element_class, templ);

    templ = gst_pad_template_new("src", GST_PAD_SRC, GST_PAD_ALWAYS, caps);
    gst_element_class_add_pad_template(element_class, templ);
    gst_caps_unref(caps);

    base_transform_class->start = GST_DEBUG_FUNCPTR(gst_overlay_timestamp_filter_start);
    base_transform_class->transform_ip =
        GST_DEBUG_FUNCPTR(gst_overlay_timestamp_filter_transform_ip);
    base_transform_class->set_caps = GST_DEBUG_FUNCPTR(gst_overlay_timestamp_filter_set_caps);
    base_transform_class->passthrough_on_same_caps = TRUE;
}

static void gst_overlay_timestamp_filter_init(GstOverlayTimestampFilter *filter) {
    GST_DEBUG_OBJECT(filter, "gst_overlay_timestamp_filter_init()");
    filter->width = 1280;
    filter->height = 720;
    filter->stride = 0;
    filter->font_size = 24;

    color_hex_string_to_rgb(&filter->text_color, "#000000");
    color_hex_string_to_rgb(&filter->outline_color, "#FFFFFF");

    snprintf(filter->timestamp_format, sizeof(filter->timestamp_format), "%s",
             DEFAULT_TIMESTAMP_FORMAT);
}

static void gst_overlay_timestamp_filter_set_property(GObject *object, guint prop_id,
                                                      const GValue *value, GParamSpec *pspec) {
    GstOverlayTimestampFilter *filter = GST_OVERLAY_TIMESTAMP_FILTER(object);
    gchar *color_hex = NULL;
    gchar *timestamp_format = NULL;

    switch (prop_id) {
        case PROP_TEXT_COLOR:
            color_hex = g_value_dup_string(value);
            color_hex_string_to_rgb(&filter->text_color, color_hex);
            GST_DEBUG_OBJECT(filter, "Text color RGB:#%02X%02X%02X", filter->text_color.red,
                             filter->text_color.green, filter->text_color.blue);
            break;
        case PROP_OUTLINE_COLOR:
            color_hex = g_value_dup_string(value);
            color_hex_string_to_rgb(&filter->outline_color, color_hex);
            GST_DEBUG_OBJECT(filter, "Outline color RGB:#%02X%02X%02X", filter->outline_color.red,
                             filter->outline_color.green, filter->outline_color.blue);
            break;
        case PROP_TIMESTAMP_FORMAT:
            timestamp_format = g_value_dup_string(value);
            snprintf(filter->timestamp_format, sizeof(filter->timestamp_format), "%s",
                     timestamp_format);
            g_free(timestamp_format);
            break;
        case PROP_TEXT_ALIGN:
            filter->text_align = g_value_get_enum(value);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }

    if (color_hex) {
        g_free(color_hex);
    }
}

static void gst_overlay_timestamp_filter_get_property(GObject *object, guint prop_id, GValue *value,
                                                      GParamSpec *pspec) {
    GstOverlayTimestampFilter *filter = GST_OVERLAY_TIMESTAMP_FILTER(object);
    gchar color_hex[8] = {0};

    switch (prop_id) {
        case PROP_TEXT_COLOR:
            rgb_to_color_hex_string(&filter->text_color, color_hex, sizeof(color_hex));
            g_value_set_string(value, color_hex);
            break;
        case PROP_OUTLINE_COLOR:
            rgb_to_color_hex_string(&filter->outline_color, color_hex, sizeof(color_hex));
            g_value_set_string(value, color_hex);
            break;
        case PROP_TIMESTAMP_FORMAT:
            g_value_set_string(value, filter->timestamp_format);
            break;
        case PROP_TEXT_ALIGN:
            g_value_set_enum(value, filter->text_align);
            break;
        default:
            G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
            break;
    }
}

static gboolean gst_overlay_timestamp_filter_start(GstBaseTransform *trans) {
    GST_DEBUG_OBJECT(trans, "gst_overlay_timestamp_filter_start()");
    return TRUE;
}

static gboolean gst_overlay_timestamp_filter_set_caps(GstBaseTransform *trans, GstCaps *incaps,
                                                      GstCaps *outcaps) {
    GstOverlayTimestampFilter *filter = GST_OVERLAY_TIMESTAMP_FILTER(trans);
    GstVideoInfo info;

    if (!gst_video_info_from_caps(&info, incaps)) {
        return FALSE;
    }

    filter->width = GST_VIDEO_INFO_WIDTH(&info);
    filter->height = GST_VIDEO_INFO_HEIGHT(&info);
    filter->stride = GST_VIDEO_INFO_PLANE_STRIDE(&info, 0);

    filter->font_size = filter->width / 20.0;
    if (filter->font_size < 10) {
        filter->font_size = 10;
    }

    if (filter->font_size > 128) {
        filter->font_size = 128;
    }

    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_A8, 0, 0);
    cairo_t *cr = cairo_create(surface);
    cairo_text_extents_t extents;
    char timestamp[64];
    current_strfmttime(timestamp, sizeof(timestamp), filter->timestamp_format);

    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, filter->font_size);
    cairo_text_extents(cr, timestamp, &extents);
    cairo_destroy(cr);
    cairo_surface_destroy(surface);

    calculate_aligned_position(filter->text_align, filter->width, filter->height, extents.width,
                               extents.height, 10, &filter->x, &filter->y);

    GST_INFO_OBJECT(filter, "Caps are set: width=%d, height=%d, x=%d, y=%d format=%s",
                    filter->width, filter->height, filter->x, filter->y,
                    gst_video_format_to_string(GST_VIDEO_INFO_FORMAT(&info)));

    return TRUE;
}

static GstFlowReturn gst_overlay_timestamp_filter_transform_ip(GstBaseTransform *trans,
                                                               GstBuffer *buf) {
    GstOverlayTimestampFilter *filter = GST_OVERLAY_TIMESTAMP_FILTER(trans);
    GstMapInfo map;

    if (!gst_buffer_map(buf, &map, GST_MAP_WRITE)) {
        return GST_FLOW_ERROR;
    }

    // システム時刻取得
    char timestamp[64];
    current_strfmttime(timestamp, sizeof(timestamp), filter->timestamp_format);

    cairo_surface_t *buffer_surface = cairo_image_surface_create_for_data(
        map.data, CAIRO_FORMAT_ARGB32, filter->width, filter->height, filter->stride);

    cairo_t *cr = cairo_create(buffer_surface);

    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, filter->font_size);
    cairo_move_to(cr, filter->x, filter->y);

    cairo_text_path(cr, timestamp);

    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_set_line_width(cr, 2.0);
    cairo_stroke_preserve(cr);

    cairo_set_source_rgb(cr, filter->text_color.red / 255.0, filter->text_color.green / 255.0,
                         filter->text_color.blue / 255.0);
    cairo_fill(cr);

    cairo_destroy(cr);
    cairo_surface_destroy(buffer_surface);
    gst_buffer_unmap(buf, &map);

    return GST_FLOW_OK;
}

static gboolean overlay_timestamp_plugin_init(GstPlugin *plugin) {
    return gst_element_register(plugin, "overlayts", GST_RANK_NONE,
                                GST_TYPE_OVERLAY_TIMESTAMP_FILTER);
}

GST_PLUGIN_DEFINE(GST_VERSION_MAJOR, GST_VERSION_MINOR, overlayts,
                  "My custom filter that overlays timestamp", overlay_timestamp_plugin_init, "1.0",
                  "LGPL", "GStreamer", "https://gstreamer.freedesktop.org/")
