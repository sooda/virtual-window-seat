# SENSOR: ov7220/ov7221, linux driver ov534
# image area 3894 x 2952 um
# pixels 640x480 active (total 656x488)
# pixel size 6.0 x 6.0 um (6.225 x 6.15 um from img area??)
# 6um*640x480 = 3.840x2.880 mm
# max rate 60 fps for vga
# output fmt yuv/ycbcr 4:2:2, rgb565/555/444, grb 4:2.2, raw rgb
# specs (pixel size etc) could be wrong too, but approx in the right range

# LENS: zoom
# horizontal fov 56 .. 75 deg
# that's a focal len of 3.61 .. 2.502 mm (can this be right?)
# blue is the more wide one, 56 with 3.61

# CALIBRATION: 9x6 pattern.png from opencv, print with outline
# principal point not at the center?

# nvprof --query-metrics

# A[R T]p_world = (u, v, 1)*z
# C_0 in world origin
# C_1 extrinsics [R T] where T position of world origin in this reference
# C_1 in C_0: -R^-1 T (rotate world origin diff to world coord then translate back)
#intr:
#
#f*mx, 0, u,
#0, f*my, v,
#0,   0,  1

import cv2
import numpy as np
from matplotlib import pyplot as plt
import time


img = cv2.imread('square.png')
rows,cols,ch = img.shape

# (first undistort etc.)



# note: intrinsic matrix contains: focal len combined with pixel scale, and principal point (at center, usually)

# "3d" point range (=corners) of this virtual image (sensor plane but in front of pinhole) is obtained easily from focal len in pixels, and known image (sensor) size; plane is one focal length away from camera

# let's say that one of the cameras always points directly to one face
# then another has an extrinsic matrix of something-else-than-I

# then:
# camera center C
# corner point p on virtual camera plane, f away from camera into its z axis and (w/2,h/2) offset on plane. normalized to f we get (w/2, h/2, 1) note w,h not in pixels but from calibration mtx, multipliers for f (e.g. 2 mm f, 3 mm sensor width -> 1.5)
# projected to 3d skybox plane P
# plane eq: (x-x0).n = 0

# ray R = C + d*v
# v obtained from p and camera calibration
# d = (x0 - C) . n / (v . n)

# top left, then clockwise
pts1 = np.float32([[0, 0],[cols-1, 0],[cols-1, rows-1],[0, rows-1]])
# looking "downwards so top edge longer", size arbitrary
pts2 = np.float32([[100, 100],[1000, 100],[400, 1000],[200, 1000]])
#pts2 = np.float32([[10, 10],[cols-1, 0],[cols-1, rows-1],[0, rows-1]])
# findHomography would be for automatically detected >4 pts
# but these are known exactly
a=time.time()
M = cv2.getPerspectiveTransform(pts1,pts2)
b=time.time()

# fyi, for warping individual points, use perspectiveTransform(in,out,m)
# see also gpu::warpPerspective
# http://docs.opencv.org/2.4/modules/gpu/doc/image_processing.html
# gpu stream things? async
# what about buildwarpperspectivemaps? maybe faster if warpperspective does it already, run this and ptu the xmap and ymap to gpu::remap()
dst = cv2.warpPerspective(img,M,(2000,2000))
c=time.time()
print b-a,c-b
# XXX in loop: waitKey(1000.0/fps) for yielding for windowing system

# initundistortrectifymap and friends for camera stuff in realtime

plt.subplot(121),plt.imshow(img),plt.title('Input')
plt.subplot(122),plt.imshow(dst),plt.title('Output')
plt.show()

