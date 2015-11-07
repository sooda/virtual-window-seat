#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/gpu/gpu.hpp>
#include <array>

using namespace std;
using namespace cv;

typedef Matx44f mat4;
typedef Matx33f mat3;
// Vec or Point?
typedef Vec4f vec4;
typedef Vec3f vec3;
typedef Vec2f vec2;
typedef Point2f pt2;
typedef Point3f pt3;

#define SZ 256
#define SZ_F 256.0f

Mat rot180(Mat m) {
	transpose(m, m);
	flip(m, m, 0);
	return m;
}

vec3 invpos(mat4 m) {
	Mat n(m);
	mat3 r = n.rowRange(0, 3).colRange(0, 3);
	vec3 t = n.rowRange(0, 3).col(3);
	return -r.t() * t;
}

struct camera {
	mat4 local_to_world;
	vec3 pos() {
		// in global coords
#if 0
		return -vec3(
				local_to_world(0, 3),
				local_to_world(1, 3),
				local_to_world(2, 3));
#else
		return invpos(local_to_world);
#endif
	}
	struct intrinsic {
		float w;
		float h;
		float f;
	} physical;
};

struct plane {
	// infinite plane
	vec3 n;
	float d;
	// to the bottom-left corner origin
	mat4 world_to_local;
};

float deg2rad(float deg) {
	return deg / 180.0f * M_PI;
}

vec3 unit(vec3 v) {
	return v * (1.0f / norm(v));
}

vec4 hompt(vec3 v) {
	return vec4(v[0], v[1], v[2], 1.0f);
}

vec3 ptfromhom(vec4 v) {
	v *= 1.0f / v[3];
	return vec3(v[0], v[1], v[2]);
}

mat4 ones() {
	// note: m(y,x)=1.0f;
	mat4 m;
	m=mat4(1.0f);
	m = m.diag(vec4(1.0f, 1.0f, 1.0f, 1.0f));
	return m;
}

mat4 scale(float x, float y, float z) {
	mat4 m(ones());
	m(0, 0) = x;
	m(1, 1) = y;
	m(2, 2) = z;
	return m;
}

mat4 rotx(float a) {
	float c = cos(a);
	float s = sin(a);
	return mat4(1.0f, 0.0f, 0.0f, 0.0f,
	            0.0f,    c,   -s, 0.0f,
	            0.0f,    s,    c, 0.0f,
	            0.0f, 0.0f, 0.0f, 1.0f);
}
mat4 roty(float a) {
	float c = cos(a);
	float s = sin(a);
	return mat4(   c, 0.0f,    s, 0.0f,
	            0.0f, 1.0f, 0.0f, 0.0f,
	              -s, 0.0f,    c, 0.0f,
	            0.0f, 0.0f, 0.0f, 1.0f);
}
#if 0
mat4 rotz(float a) {
	float c = cos(a);
	float s = sin(a);
	return mat4(   c,   -s, 0.0f, 0.0f,
	               s,    c, 0.0f, 0.0f,
	            0.0f, 0.0f, 1.0f, 0.0f,
	            0.0f, 0.0f, 0.0f, 1.0f);
}
#endif

mat4 translate(float x, float y, float z) {
	mat4 m(ones());
	m(0,3)=x;
	m(1,3)=y;
	m(2,3)=z;
	return m;
}

float intersect_vec_plane(vec3 origin, vec3 dir, plane p) {
	// vec: origin + t * dir
	// plane: n . p + d == 0
	// what's t?
	// n . (origin + t * dir) + d == 0
	// n . origin + n . (t * dir) + d == 0
	// n . (t * dir) == -d - n . origin
	// t * n . dir == -d - n . origin
	// t == -(d + n . origin) / (n . dir)
	return -(p.d + p.n.dot(origin)) / (p.n.dot(dir));
}

class dont_draw {};

// project a point pt_2d in camera's image plane (local coords) on 3d plane p
vec2 camplane_to_plane(camera c, vec2 pt_2d, plane p) {
	cerr << "camplane to plane: pt " << pt_2d << endl;
	// should be normalized to f already
	vec4 local_3d(pt_2d[0], pt_2d[1], -1.0f, 1.0f); // homog, note z dir
	// on virtual image plane in global coord
	// local screen is always at z=-1.0f and straight along x,y axes
	vec4 pt_3d_hom = c.local_to_world * local_3d;
	cerr << "pt 3d hom, in world: " << pt_3d_hom << endl;
	vec3 pt_3d(ptfromhom(pt_3d_hom));
	vec3 v = unit(pt_3d - c.pos());
	cerr << "v: " << v << endl;
	float t = intersect_vec_plane(c.pos(), v, p);
	if (t < 0) {
		// FIXME: other side may be negative nicely if the other is not, then it just wraps infinitely
		cerr << "negative ray ignored " << t << endl;
#if 0
		return vec2();
#endif
		throw dont_draw();
	}
	vec3 pt_on_plane = c.pos() + t * v;
	cerr << "pt on plane: " << t << " units => " << pt_on_plane << endl;
	vec4 pt_on_plane_hom = hompt(pt_on_plane);

	vec4 pt_on_plane_as_2d = p.world_to_local * pt_on_plane_hom;
	cerr << "as 2d: " << pt_on_plane_as_2d << endl;
	// div by w?
	vec2 ret(pt_on_plane_as_2d[0], pt_on_plane_as_2d[1]); // * scaling?
	cerr << "ret: " << ret << endl;
	return ret;
}

#if 0
// given a vec from c to pt, where (in 2d) does it hit the camera image?
vec2 pt_to_camplane(camera c, vec3 pt_3d) {
	// v = (pt - c).u
	// c + t*v = i
	// transform i from world into camera local
	// i /= i.z (image plane is at 1)
	vec3 v = unit(pt_3d - c.pos());
	// m * c + m * t + v = m * i [m * i.z == 1]
}
#endif

// find where the corners of c's image plane project to, on plane p.
// image wid/hei given by scale, in units of focal len
// tex1 is "one" in tex pixel coords
Mat obtain_dest(camera c, plane p, vec2 scale, float texw, float texh) {
	// lower left and counterclockwise
	pt2 src_pixels[] = {
		pt2(0.0f, 0.0f),
		pt2(texw, 0.0f),
		pt2(texw, texh),
		pt2(0.0f, texh)
	};
	vec2 v0 = camplane_to_plane(c, vec2(scale[0] * -1.0f, scale[1] * -1.0f), p);
	vec2 v1 = camplane_to_plane(c, vec2(scale[0] *  1.0f, scale[1] * -1.0f), p);
	vec2 v2 = camplane_to_plane(c, vec2(scale[0] *  1.0f, scale[1] *  1.0f), p);
	vec2 v3 = camplane_to_plane(c, vec2(scale[0] * -1.0f, scale[1] *  1.0f), p);
#if 0
	if (v0 == v1) {
		// all v0, v1, v2,v3 zeroes from camplane_to_plane since it's behind the cam
		cerr << "flipping plane ignored" << endl;
	}
#endif
	pt2 dst_pixels[] = {v0, v1, v2, v3};
	cerr << src_pixels[0] << endl;
	cerr << src_pixels[1] << endl;
	cerr << src_pixels[2] << endl;
	cerr << src_pixels[3] << endl;
	cerr << "==>" << endl;
	cerr << dst_pixels[0] << endl;
	cerr << dst_pixels[1] << endl;
	cerr << dst_pixels[2] << endl;
	cerr << dst_pixels[3] << endl;
	Mat m = getPerspectiveTransform(src_pixels, dst_pixels);
	return m;
}

// project 'src' tex taken from c onto plane p, return whole resulting tex
Mat project(camera c, plane p, Mat src) {
	vec2 scale(0.5f * c.physical.w / c.physical.f, 0.5f * c.physical.h / c.physical.f);
	Mat m;
	try {
		m = obtain_dest(c, p, scale, src.size().width, src.size().height);
	} catch (dont_draw&) {
		cerr<<"flipping plane ignored "<<endl;
		return Mat(SZ, SZ, CV_8UC3, Scalar(0));
	}
	Mat dst;
	cerr<<"jee"<<endl;
	cerr<<m<<endl;
	warpPerspective(src, dst, m, Size(SZ, SZ));
	dst.rowRange(0, 10) = Mat::ones(10, SZ, CV_8UC3)*0xff;
	dst.colRange(0, 10) = Mat::ones(SZ, 10, CV_8UC3)*0xff;
	return dst;
}

struct camdata {
	camera c;
	Mat frame;
};

struct boxside {
	plane p;
	Mat tex;
};

struct skybox {
	boxside xmin, xmax;
	boxside ymin, ymax;
	boxside zmin, zmax;
};

void test() {
	camdata cams[] = {
		{
			camera{

				rotx(30.0f*3.14159f/180.0f)*ones(), // local to world: camera sits at origin. positive rotation here tilts the cam down because local2world, not camera's rot
				{
					0.50f, // w (all these three in same units)
					0.25f, // h
					1.0f // f
				}
			},
			imread("camfront.png")
		},
	};
	skybox box;
	// world point to plane: plane is backed off z axis, some left and down
	// so invert it. z -2 in world is z 0 on plane
	// size is twice as big as image plane, so 4 per dir
	// also shift the corner properly
	mat4 wtl = translate(2.0f, 2.0f, 2.0f);
	// size of the whole plane is now 4x4, need to make it 1x1
	wtl = scale(1.0f/4.0f, 1.0f/4.0f, 1.0f) * wtl;
	// and then in pixel coords!
	wtl = scale(SZ_F, SZ_F, 1.0f) * wtl;
	// FIXME TODO XXX: topleft or bottomleft origin?

	cerr << wtl << endl;
	// plane: n.p + d == 0
	box.zmin.p = plane{
		{0.0f, 0.0f, 1.0f}, // normal towards box center
		2.0f, // dist: plane normal 1, mul by coord -2, add 2 to get 0
		wtl // world_to_local
	};
	// first camera looking into zmin (front)
	// exactly at the middle
	Mat out = project(cams[0].c, box.zmin.p, cams[0].frame);
	imwrite("out.png", out);
}

// FIXME this gets scaled ughh, real scaling in camera size. tweak wtl below to just center it, the eqs have boxdim already for proper scaling
static float boxdim = 1.0f; // dist from cam, half box
// "local" = in 2d coords and units already here
mat4 frontbox_world_to_local() {
	mat4 wtl;
	// position the corner properly, and get away from cam
	wtl = translate(boxdim, boxdim, boxdim);
	// size of the whole plane is now 2dim x 2dim, need to make it 1x1 (z isn't important at this point anymore)
	wtl = scale(1.0f/(2.0f*boxdim), 1.0f/(2.0f*boxdim), 1.0f) * wtl;
	// and then in pixel coords!
	wtl = scale(SZ_F, SZ_F, 1.0f) * wtl;
	return wtl;
}

void testb() {
	camdata cams[] = {
		{
			camera{
				rotx(deg2rad(30.0f)),
				{ 0.50f, 0.50f, 1.0f }
			},
			imread("camup.png")
		},
	};
	skybox box;
	box.zmin.p = plane{
		{0.0f, -1.0f, 0.0f},
		boxdim,
		frontbox_world_to_local() * rotx(deg2rad(-90.0f))
	};
	Mat out = project(cams[0].c, box.zmin.p, cams[0].frame);
	imwrite("outb.png", out);
}

// one whole tex, sum images over
// TODO what to do with overlap? now just sum them
Mat projectwhole(camdata *cams, int ncams, plane p) {
	cerr<<"IDX:0"<<endl;
	Mat full = project(cams[0].c, p, cams[0].frame);
	for (int i = 1; i < ncams; i++) {
		cerr<<"IDX:"<<i<<endl;
		cerr<<"CAM::::"<<cams[i].c.local_to_world<<endl;
		Mat next = project(cams[i].c, p, cams[i].frame);
		full += next;
	}
	return full;
}

#define TEST 0
//#define CAMS

void test2() {
#ifdef CAMS
	static VideoCapture cap1(0);
	static VideoCapture cap2(1);
	static int initd;

	if (!initd) {
		initd=1;
		cap1.set(CV_CAP_PROP_FRAME_WIDTH, 320);
		cap1.set(CV_CAP_PROP_FRAME_HEIGHT, 240);
		cap2.set(CV_CAP_PROP_FRAME_WIDTH, 320);
		cap2.set(CV_CAP_PROP_FRAME_HEIGHT, 240);
	}
	Mat a, b;
	cap1 >> a;
	cap2 >> b;
#endif
#if !TEST
	static VideoCapture cap[4];
	static int initd;

	if (!initd) {
		initd=1;
		for (int i = 0; i < 4; i++) {
			cerr << "open " << i << endl;
			if (!cap[i].open(i))
				throw "nope";
			cap[i].set(CV_CAP_PROP_FRAME_WIDTH, 320);
			cap[i].set(CV_CAP_PROP_FRAME_HEIGHT, 240);
		}
	}
	Mat camfront, camright, camleft, camdown;
	cap[0] >> camfront;
	cap[1] >> camright;
	cap[2] >> camleft;
	cap[3] >> camdown;
	flip(camright, camright, -1);
	flip(camleft, camleft, -1);
	cerr << "hox:" << endl;
	cerr << camfront.size() << endl;
	cerr << camright.size() << endl;
	cerr << camleft.size() << endl;
	cerr << camdown.size() << endl;
#endif
	float w = 1.53f; // 2*tan(75deg/2)
	float h = w/640.0*480.0; // ~1.15
	//w=1.0f;h=1.0f; // should be half of the screen with these

	static float ang;
	ang += deg2rad(3);
#if TEST
	array<camdata, 8> cams ={ {
#else
	array<camdata, 4> cams ={ {
#endif

	
#if TEST
		{ camera{
				ones(),
				{ w, h, 1.0f }
			},
#ifndef CAMS
			imread("camfront.png")
#else
			a
#endif
		},
#if 1
		{ camera{
				roty(ang)*rotx(deg2rad(5.0f)),
				{ w, h, 1.0f }
			},
			imread("blank.png")
		},
#endif
		{ camera{
				rotx(ang),
				{ w, h, 1.0f }
			},
			imread("blank.png")
		},
		{ camera{
				roty(deg2rad(-90.0f)),
				{ w, h, 1.0f }
			},
#ifndef CAMS
			imread("camright.png")
#else
			b
#endif
		},
		{ camera{
				roty(deg2rad(90.0f)),
				{ w, h, 1.0f }
			},
			imread("camleft.png")
		},
		{ camera{
				rotx(deg2rad(90.0f)),
				{ w, h, 1.0f }
			},
			imread("camup.png")
		},
		{ camera{
				rotx(deg2rad(-90.0f)),
				{ w, h, 1.0f }
			},
			imread("camdown.png")
		},
		{ camera{
				roty(deg2rad(180.0f)),
				{ w, h, 1.0f }
			},
			imread("camback.png")
		}
#else
		{ camera{
				ones(),
				{ w, h, 1.0f }
			},
			camfront,
		},
		{ camera{
				roty(deg2rad(-90.0f))*rotx(deg2rad(20.0f)),
				{ w, h, 1.0f }
			},
			camright,
		},
		{ camera{
				roty(deg2rad(90.0f))*rotx(deg2rad(20.0f)),
				{ w, h, 1.0f }
			},
			camleft,
		},
		{ camera{
				rotx(deg2rad(-90.0f)),
				{ w, h, 1.0f }
			},
			camdown
		},
#endif
	}};
	skybox box;
	cerr << frontbox_world_to_local() << endl;
	// normals pointing inside the box here, dunno if it matters
	// plane: n.p + d == 0
	
	// front
	box.zmin.p = plane{
		{0.0f, 0.0f, 1.0f},
		boxdim,
		frontbox_world_to_local()
	};
	// right
	box.xmax.p = plane{
		{-1.0f, 0.0f, 0.0f},
		boxdim,
		frontbox_world_to_local() * roty(deg2rad(90.0f))
	};
	// left
	box.xmin.p = plane{
		{1.0f, 0.0f, 0.0f},
		boxdim,
		frontbox_world_to_local() * roty(deg2rad(-90.0f))
	};
	// up
	box.ymax.p = plane{
		{0.0f, -1.0f, 0.0f},
		boxdim,
		frontbox_world_to_local() * rotx(deg2rad(-90.0f))
	};
	// down
	box.ymin.p = plane{
		{0.0f, 1.0f, 0.0f},
		boxdim,
		frontbox_world_to_local() * rotx(deg2rad(90.0f))
	};
	// back
	box.zmax.p = plane{
		{0.0f, 0.0f, -1.0f},
		boxdim,
		frontbox_world_to_local() * roty(deg2rad(180.0f))
	};
	// first camera looking into zmin (front)
	// exactly at the middle
	//
	//      top
	// left front right back
	//      bottom

	cerr<<"top"<<endl;
	box.ymax.tex = projectwhole(cams.data(), cams.size(), box.ymax.p); // top
	cerr<<"left"<<endl;
	box.xmin.tex = projectwhole(cams.data(), cams.size(), box.xmin.p); // left
	cerr<<"front"<<endl;
	box.zmin.tex = projectwhole(cams.data(), cams.size(), box.zmin.p); // front
	cerr<<"right"<<endl;
	box.xmax.tex = projectwhole(cams.data(), cams.size(), box.xmax.p); // right
	cerr<<"back"<<endl;
	box.zmax.tex = projectwhole(cams.data(), cams.size(), box.zmax.p); // back
	cerr<<"bottom"<<endl;
	box.ymin.tex = projectwhole(cams.data(), cams.size(), box.ymin.p); // bottom
	cerr<<endl<<endl<<endl<<endl;
	Mat out(3*SZ, 4*SZ, CV_8UC3, Scalar(0));
	box.ymax.tex.copyTo(out.rowRange(0, SZ).colRange(SZ, 2*SZ));
	box.xmin.tex.copyTo(out.rowRange(SZ, 2*SZ).colRange(0, SZ));
	box.zmin.tex.copyTo(out.rowRange(SZ, 2*SZ).colRange(SZ, 2*SZ));
	box.xmax.tex.copyTo(out.rowRange(SZ, 2*SZ).colRange(2*SZ, 3*SZ));
	box.zmax.tex.copyTo(out.rowRange(SZ, 2*SZ).colRange(3*SZ, 4*SZ));
	box.ymin.tex.copyTo(out.rowRange(2*SZ, 3*SZ).colRange(SZ, 2*SZ));
	imwrite("outfull.png", out);
#if 0
	Size s=out.size();
	resize(out,out,Size(s.width/2,s.height/2));
#endif
	namedWindow("outfull.png", 1);
	imshow("outfull.png", out);
}

int main() {
#if 0
	cerr << rotx(0) << endl;
	cerr << roty(0) << endl;
	cerr << rotz(0) << endl;
	cerr << "--" << endl;
	cerr << rotx(deg2rad(90)) << endl;
	cerr << roty(deg2rad(90)) << endl;
	cerr << rotz(deg2rad(90)) << endl;
	return 0;
#endif
#if 0
	testb();
	return 0;
#endif
	for (;;) {
		test2();
		waitKey(1);
	}
}
