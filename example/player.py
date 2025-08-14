import sys
import argparse
from gi.repository import Gst, GObject

parser = argparse.ArgumentParser(description="video player of gst-overlay-timestamp-filter")
parser.add_argument("input_file", help="input video file path")
parser.add_argument("--text-color", help="timestamp text color (e.g., '#98FFF0')")
parser.add_argument("--outline-color", help="timestamp outline color (e.g., '#000000')")
parser.add_argument("--text-align", help="alignment of timestamp text (e.g., 'top-left', 'center', 'bottom-right')")
parser.add_argument("--format", help="show timestamp format (e.g., '%Y/%m/%d %H:%M:%S.%q')")
args = parser.parse_args()


def on_message(_, message):
    if message.type == Gst.MessageType.EOS:
        pipeline.seek_simple(
            Gst.Format.TIME,
            Gst.SeekFlags.FLUSH | Gst.SeekFlags.ACCURATE,
            0 * Gst.SECOND
        )
    return True


gst_elements = [
    f"filesrc location={args.input_file}",
    "decodebin",
    "videoscale",
    "videoconvert",
]

plugin_name = "overlayts"

if args.text_color:
    plugin_name += f' text-color="{args.text_color}"'

if args.outline_color:
    plugin_name += f' outline-color="{args.outline_color}"'

if args.text_align:
    plugin_name += f' text-align="{args.text_align}"'

if args.format:
    plugin_name += f' format="{args.format}"'

gst_elements.extend([
    plugin_name,
    "videoconvert",
    "ximagesink",
])


Gst.init(None)
launch_cmd = " ! ".join(gst_elements)
print(launch_cmd)
pipeline = Gst.parse_launch(launch_cmd)

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
