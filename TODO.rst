TODO
====

We'd like to make this really a working prototype since the prototype we built during the hackathon only had four cameras (box.jpg). Maybe not in an airplane (except a drone) but as a cheapo 360-degree VR camera, like a poor man's Nokia OZO. Future work might happen sometimes later when we get excited again; this repo is some sort of conclusion only.

hardware
--------

More cameras would be nice. And/or a larger field of view; just replace the lenses.

An airplane moves relatively slowly compared to the terrain or the sunset or the moon or whatever (read: synchronization or camera bandwidth wouldn't be real issues), and its cameras would be too far away from the host streamer for USB to work anyway, so HW details aren't really relevant for that use case.

But what if the surroundings required a high framerate? Need proper high resolution cameras, but USB 2 can't really handle the bandwidth for that many. Can this be done in a shitty budget?

Read first: `Jetson/Cameras on elinux.org`_

.. _Jetson/Cameras on elinux.org: http://elinux.org/Jetson/Cameras

* Compression

  * This is just a neat entertainment tool, so we don't need raw data from the cameras; use a camera that can output compressed video to save the BW

* USB 3

  * not really any (affordable) webcams available
  * machine vision expensive for a toy project like this
  * how about a custom hub/proxy for talking USB 2 to the cameras and USB 3 to the host?

    * "hub" isn't really a correct term for this, as a normal USB hub `doesn't work that way`_
    * either one proxy for each camera combined to an USB 3 hub on the host, or one big proxy with N USB 2 receptables to work as a hub
    * look like an USB 3 camera (or a hub and several cams) to the host, actually drive the camera(s) when host requests data (possibly store framebuffers too)

* MIPI

  * Just two camera connections on Jetson TK1?
  * Multiplex the connection for more, like `this guy with a Pi`_? Probably not enough BW though

* Ethernet

  * Again BW might not be enough, but GigE cameras are standard at least in the expensive machine vision industry

* Tegra TX1

  * Claimed to handle 12 cameras simultaneously
  * Not sure about Jetson TX1, but get a Drive PX and hack that

* Just Throw Some Money At It

  * With a ~1keur budget, we could try something decent; at least the e-con cameras linked from elinux.org look somewhat feasible.

.. _doesn't work that way: http://superuser.com/questions/278670/using-usb-3-0-to-speed-up-transfer-between-usb-2-0-devices
.. _this guy with a Pi: http://hackaday.com/2014/12/19/multiplexing-pi-cameras/

software
--------

The code is ugly and hacky, which is completely expected and okay since it's hacked together in a hackathon.

* Texture projection

  * Get rid of whatever buffers we have in the OpenCV path, read v4l directly
  * Try to draw the projections "the other way around", i.e., for each pixel in a face, check where it comes from in the camera, on a GPU
  * Remove some generalities since they aren't applicable; e.g., all cameras should sit at the origin, so no position (just rotation) needed

* Streaming

  * Get rid of that ugly gstreamer plugin that requires an opencv package that conflicts with opencv4tegra
  * or fix opencv4tegra to virtually provide that, or fix the gstreamer thing to depend on either one
  * Actually understand what all the ffmpeg and gstreamer shit do, and how to perform that without lag (without buffers in between) maybe natively in C++

* Playback

  * Wait for webgl texture streaming to work correctly on any mobile device
  * Or hax the native player so that it could playback our stream properly
  * Calibrate the gyro sensor output to rotate the view exactly as the phone rotates; now it's scaled wrong

VR clarifications
-----------------

Lack of sleep got us into thinking wrongly some things and solving unsolvable problems.

* The client obviously renders everything in a cube (whose size is arbitrary) centered at the viewer
* The physical cameras should also sit at the cube center, or otherwise the user would see things wrongly
* Consequently, The physical cameras should also be infinitely close to each other

  * This assumption holds in an airplane that is practically infinitely far away from the surroundings and can be considered a single point
  * But wouldn't work for close-to-ground full-coverage cameras (there's a reason they are small spheres)
  * Maybe study if there are magic lenses for moving the aperture kinda behind the sensor

* Proper 3D isn't necessary in an airplane, but would be nice on the ground

  * general 3D reconstruction is nearly infinitely more difficult problem, so maybe allow 3D on only fixed camera pairs
