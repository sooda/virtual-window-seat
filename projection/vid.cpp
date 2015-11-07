#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/gpu/gpu.hpp>
#include <array>

using namespace std;
using namespace cv;

int main() {
	VideoCapture cap1(0);
	VideoCapture cap2(1);

	cap1.set(CV_CAP_PROP_FRAME_WIDTH, 320);
	cap1.set(CV_CAP_PROP_FRAME_HEIGHT, 240);
	cap2.set(CV_CAP_PROP_FRAME_WIDTH, 320);
	cap2.set(CV_CAP_PROP_FRAME_HEIGHT, 240);

	for (;;) {
		Mat a, b;
		cap1 >> a;
		cap2 >> b;

		Mat out(240, 640, CV_8UC3);
		a.copyTo(out.rowRange(0, 240).colRange(0, 320));
		b.copyTo(out.rowRange(0, 240).colRange(320, 640));

		namedWindow("outfull.png", 1);
		imshow("outfull.png", out);
		waitKey(1);
	}
}
