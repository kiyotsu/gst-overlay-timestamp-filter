#ifndef __GST_OVERLAY_TIMESTAMP_FILTER_H__
#define __GST_OVERLAY_TIMESTAMP_FILTER_H__

#include <gst/gst.h>
#include <gst/base/gstbasetransform.h>
#include "gst_overlay_timestamp_utils.h"

G_BEGIN_DECLS

#define GST_TYPE_OVERLAY_TIMESTAMP_FILTER (gst_overlay_timestamp_filter_get_type())
G_DECLARE_FINAL_TYPE(GstOverlayTimestampFilter, gst_overlay_timestamp_filter, GST, OVERLAY_TIMESTAMP_FILTER, GstBaseTransform)

#define TIMESTAMP_FORMAT_LEN 64

#define DEFAULT_TEXT_COLOR "#000000"
#define DEFAULT_OUTLINE_COLOR "#FFFFFF"
#define DEFAULT_TIMESTAMP_FORMAT "%Y-%m-%d %H:%M:%S.%q"
#define DEFAULT_TEXT_ALIGN TEXT_ALIGN_TOP_LEFT

enum
{
    PROP_0,
    PROP_TEXT_COLOR,
    PROP_OUTLINE_COLOR,
    PROP_TIMESTAMP_FORMAT,
    PROP_TEXT_ALIGN,
};

struct _GstOverlayTimestampFilter
{
    GstBaseTransform parent;

    struct RGB text_color;
    struct RGB outline_color;

    gchar timestamp_format[TIMESTAMP_FORMAT_LEN];
    TextAlignment text_align;

    gint x;
    gint y;

    gint width;
    gint height;
    gint stride;
    gdouble font_size;
};

G_END_DECLS

#endif
