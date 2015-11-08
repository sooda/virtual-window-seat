#./stream | ffmpeg -f rawvideo -i - -s 1024x768 -r 60 -pix_fmt bgra -c:v libx264 -preset slow -crf 22 $1.mkv
#./stream | avconv -f rawvideo -re -i - -s 1024x768 -r 60 -pix_fmt bgra -an -map 0 -flags -global_header -c:v libx264 -preset slow -crf 22 $1.mkv
./stream | gst-launch-1.0 -e -v fdsrc fd=0 ! videoparse width=1024 height=768 framerate=60/1 format=12 ! autovideoconvert ! omxh264enc ! mp4mux ! filesink location=$1.mp4
