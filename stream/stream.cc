extern "C" {
#include <stdio.h>
#include <unistd.h>
}
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <iostream>

using namespace cv;
using namespace std;

static void write_img(VideoCapture &cap_a, VideoCapture &cap_b, VideoCapture &cap_c, VideoCapture &cap_d)
{
	Mat out_img;
	Mat img(480, 640, CV_8UC3), img0, img1, img2, img3;
	while (1) {
		cap_a >> img0;
		cap_b >> img1;
		cap_c >> img2;
		cap_d >> img3;
		//img = (img0 + img1) / 2.0f;
		//resize(img, img, Size(640, 480));
		img0.copyTo(img.rowRange(0, 240).colRange(0, 320));
		img1.copyTo(img.rowRange(0, 240).colRange(320, 640));
		img2.copyTo(img.rowRange(240, 480).colRange(0, 320));
		img3.copyTo(img.rowRange(240, 480).colRange(320, 640));
		cvtColor(img, out_img, CV_BGR2BGRA);
		const auto size = out_img.elemSize() * out_img.total();
		write(STDOUT_FILENO, out_img.data, size);
	}
}

int main( int argc, char** argv )
{
	Mat image;
	VideoCapture cap_a(0);
	VideoCapture cap_b(1);
	VideoCapture cap_c(2);
	VideoCapture cap_d(3);

	cap_a.set(CV_CAP_PROP_FRAME_WIDTH, 320);
	cap_a.set(CV_CAP_PROP_FRAME_HEIGHT, 240);
	cap_b.set(CV_CAP_PROP_FRAME_WIDTH, 320);
	cap_b.set(CV_CAP_PROP_FRAME_HEIGHT, 240);
	cap_c.set(CV_CAP_PROP_FRAME_WIDTH, 320);
	cap_c.set(CV_CAP_PROP_FRAME_HEIGHT, 240);
	cap_d.set(CV_CAP_PROP_FRAME_WIDTH, 320);
	cap_d.set(CV_CAP_PROP_FRAME_HEIGHT, 240);

	write_img(cap_a, cap_b, cap_c, cap_d);
	return 0;
}
