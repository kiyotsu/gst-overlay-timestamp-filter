import sys
from gi.repository import Gst, GObject


def on_message(_, message):
    if message.type == Gst.MessageType.EOS:
        pipeline.seek_simple(
            Gst.Format.TIME,
            Gst.SeekFlags.FLUSH | Gst.SeekFlags.ACCURATE,
            0 * Gst.SECOND
        )
    return True


if len(sys.argv) < 2:
    raise TypeError("Required input file path parameter !!")


input_file_path = sys.argv[1]
gst_elements = [
    f"filesrc location={input_file_path}",
    "decodebin",
    "videoscale",
    "videoconvert",
    "overlayts text-color=\"#98FFF0\" format=\"%Y/%m/%d %H:%M:%S.%q\"",
    "videoconvert",
    "ximagesink",
]


Gst.init(None)
print(" ! ".join(gst_elements))
pipeline = Gst.parse_launch(" ! ".join(gst_elements))

bus = pipeline.get_bus()
bus.add_signal_watch()
bus.connect("message", on_message)

pipeline.set_state(Gst.State.PLAYING)

loop = GObject.MainLoop()
try:
    loop.run()
except KeyboardInterrupt:
    pass

pipeline.set_state(Gst.State.NULL)
