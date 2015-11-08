Virtual window seat for airplanes
=================================

A quick hack for Junction_ 2015.

.. _Junction: http://hackjunction.com

Details here: http://devpost.com/software/virtual-window-seat

TL;DR: what if airplanes had video cameras mounted all around them for full 360° realtime video coverage of the outside view? What if we could stream that onto a mobile device for anyone onboard, and render it as a simple skybox, taking into account the device's pose based on its gyro?

This is just a project log of some ugly hacks during a weekend with too little sleep. Don't use this for anything. We have some future plans to make this neater and suitable for also other things than airplanes. (Our prototype "airplane" is a cardboard box.)

We couldn't get rtsp streaming to work to the android phone we tried, and webgl streams almost worked but not quite, too. The android client plays a static image or a pre-recorded video. The live stream is viewable with a normal computer though.

Special hardware: Nvidia Jetson TK1, and four cheap webcams. Could use any Linux box though, Jetson is just a very neat mobile supercomputer. We observed a cpu usage of about 10 % and perf wasn't a priority. The resolution is just 320x240 though to fit four streams in the usb bandwidth and in our timeframe.
