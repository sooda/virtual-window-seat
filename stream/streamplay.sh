#./stream | ffmpeg -f rawvideo -i - -s 906x720 -pix_fmt bgr24 
./stream | gst-launch-1.0 -v fdsrc fd=0 ! videoparse width=1024 height=768 framerate=60/1 format=12 ! autovideoconvert ! omxh264enc ! rtph264pay pt=96 config-interval=2 ! udpsink host=0.0.0.0 port=5000 clients=localhost:5000,192.168.1.2:5000,172.16.11.203:5000,172.16.9.78:5000 # the best so far
#./stream | gst-launch-1.0 -v fdsrc fd=0 ! videoparse width=640 height=480 framerate=60/1 format=12 ! autovideoconvert ! x264enc ! rtph264pay pt=96 config-interval=10 ! udpsink host=0.0.0.0 port=5000 # this works, BGRA, there's a minute of lag in the stream
