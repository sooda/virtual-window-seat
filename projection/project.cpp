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

#define SZ 1024
#define SZ_F 1024.0f

struct camera {
	mat4 local_to_world;
	vec3 pos() {
		return -vec3(
				local_to_world(0, 3),
				local_to_world(1, 3),
				local_to_world(2, 3));
	}
	struct {
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
mat4 rotz(float a) {
	float c = cos(a);
	float s = sin(a);
	return mat4(   c,   -s, 0.0f, 0.0f,
	               s,    c, 0.0f, 0.0f,
	            0.0f, 0.0f, 1.0f, 0.0f,
	            0.0f, 0.0f, 0.0f, 1.0f);
}

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
	cout << "camplane to plane: pt " << pt_2d << endl;
	// should be normalized to f already
	vec4 local_3d(pt_2d[0], pt_2d[1], -1.0f, 1.0f); // homog, note z dir
	// on virtual image plane in global coord
	vec4 pt_3d_hom = c.local_to_world * local_3d;
	cout << "pt 3d hom, in world: " << pt_3d_hom << endl;
	vec3 pt_3d(ptfromhom(pt_3d_hom));
	vec3 v = unit(pt_3d - c.pos());
	cout << "v: " << v << endl;
	float t = intersect_vec_plane(c.pos(), v, p);
	if (t < 0) {
		cout << "negative ray ignored" << endl;
#if 0
		return vec2();
#endif
		throw dont_draw();
	}
	vec3 pt_on_plane = c.pos() + t * v;
	cout << "pt on plane: " << t << " units => " << pt_on_plane << endl;
	vec4 pt_on_plane_hom = hompt(pt_on_plane);

	vec4 pt_on_plane_as_2d = p.world_to_local * pt_on_plane_hom;
	cout << "as 2d: " << pt_on_plane_as_2d << endl;
	// div by w?
	vec2 ret(pt_on_plane_as_2d[0], pt_on_plane_as_2d[1]); // * scaling?
	cout << "ret: " << ret << endl;
	return ret;
}

// find where the corners of c's image plane project to, on plane p.
// image wid/hei given by scale, in units of focal len
// tex1 is "one" in tex pixel coords
Mat obtain_dest(camera c, plane p, vec2 scale, float tex1) {
	// lower left and counterclockwise
	pt2 src_pixels[] = {
		pt2(0.0f, 0.0f),
		pt2(tex1, 0.0f),
		pt2(tex1, tex1),
		pt2(0.0f, tex1)
	};
	vec2 v0 = camplane_to_plane(c, vec2(scale[0] * -1.0f, scale[1] * -1.0f), p);
	vec2 v1 = camplane_to_plane(c, vec2(scale[0] *  1.0f, scale[1] * -1.0f), p);
	vec2 v2 = camplane_to_plane(c, vec2(scale[0] *  1.0f, scale[1] *  1.0f), p);
	vec2 v3 = camplane_to_plane(c, vec2(scale[0] * -1.0f, scale[1] *  1.0f), p);
#if 0
	if (v0 == v1) {
		// all v0, v1, v2,v3 zeroes from camplane_to_plane since it's behind the cam
		cout << "flipping plane ignored" << endl;
	}
#endif
	pt2 dst_pixels[] = {v0, v1, v2, v3};
	cout << src_pixels[0] << endl;
	cout << src_pixels[1] << endl;
	cout << src_pixels[2] << endl;
	cout << src_pixels[3] << endl;
	cout << "==>" << endl;
	cout << dst_pixels[0] << endl;
	cout << dst_pixels[1] << endl;
	cout << dst_pixels[2] << endl;
	cout << dst_pixels[3] << endl;
	Mat m = getPerspectiveTransform(src_pixels, dst_pixels);
	return m;
}

// project 'src' tex taken from c onto plane p, return whole resulting tex
Mat project(camera c, plane p, Mat src) {
	vec2 scale(c.physical.w / c.physical.f, c.physical.h / c.physical.f);
	// XXX assume width==height
	Mat m;
	try {
		m = obtain_dest(c, p, scale, src.size().width);
	} catch (dont_draw&) {
		cout<<"flipping plane ignored "<<endl;
		return Mat(SZ, SZ, CV_8UC3);
	}
	Mat dst;
	cout<<"jee"<<endl;
	cout<<m<<endl;
	warpPerspective(src, dst, m, Size(SZ, SZ));
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

	cout << wtl << endl;
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

// le right side of teh wurld
void testb() {
	camdata cams[] = {
		{
			camera{
				// 90 rotated here means turn left to face on the front in world
				roty(90.0f*3.14159f/180.0f)*ones(),
				{
					0.50f, // w (all these three in same units)
					0.50f, // h
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
	mat4 rot = roty(-90.0f*3.1415926526f/180.0f);
	cout<<"rot"<<rot<<endl;
	// rot whatever there is first to front then translate similarly as for the front plane
	mat4 wtl = translate(2.0f, 2.0f, 2.0f)*rot;
	cout<<"tran"<<translate(2.0f,2.0f,2.0f)<<endl;
	// size of the whole plane is now 4x4, need to make it 1x1
	wtl = scale(1.0f/4.0f, 1.0f/4.0f, 1.0f) * wtl;
	// and then in pixel coords!
	wtl = scale(SZ_F, SZ_F, 1.0f) * wtl;
	// FIXME TODO XXX: topleft or bottomleft origin?

	cout << "wtl:"<<wtl << endl;
	// plane: n.p + d == 0
	box.zmin.p = plane{
		{1.0f, 0.0f, 0.0f}, // normal towards box center
		2.0f, // dist: plane normal 1, mul by coord -2, add 2 to get 0
		wtl // world_to_local
	};
	// first camera looking into zmin (front)
	// exactly at the middle
	Mat out = project(cams[0].c, box.zmin.p, cams[0].frame);
	imwrite("outb.png", out);
}

// one whole tex, sum images over
// TODO what to do with overlap? now just sum them
Mat projectwhole(camdata *cams, int ncams, plane p) {
	Mat full = project(cams[0].c, p, cams[0].frame);
	for (int i = 1; i < ncams; i++) {
		cout<<"CAM::::"<<cams[i].c.local_to_world<<endl;
		Mat next = project(cams[i].c, p, cams[i].frame);
		full += next;
	}
	return full;
}

void test2() {
	array<camdata,3> cams ={ {
		{
			camera{ // front

				rotx(00.0f*3.14159f/180.0f)*ones(), // local to world: camera sits at origin. positive rotation here tilts the cam down because local2world, not camera's rot
				{
					0.50f, // w (all these three in same units)
					0.50f, // h
					1.0f // f
				}
			},
			imread("camfront.png")
		},
		{
			camera{ // right

				translate(0.0f, 0.0f, 0.0f)*roty(-90.0f*3.14159f/180.0f)*ones(),
				{
					0.50f, // w (all these three in same units)
					0.50f, // h
					1.0f // f
				}
			},
			imread("camright.png")
		},
		{
			camera{ // left

				translate(0.0f, 0.0f, 0.0f)*roty(90.0f*3.14159f/180.0f)*ones(),
				{
					0.50f, // w (all these three in same units)
					0.50f, // h
					1.0f // f
				}
			},
			imread("camleft.png")
		}
	}};
	skybox box;
	// world point to plane: plane is backed off z axis, some left and down
	// so invert it. z -2 in world is z 0 on plane
	// size is twice as big as image plane, so 4 per dir
	// also shift the corner properly
	mat4 wtl;
	wtl = translate(2.0f, 2.0f, 2.0f);
	// size of the whole plane is now 4x4, need to make it 1x1
	wtl = scale(1.0f/4.0f, 1.0f/4.0f, 1.0f) * wtl;
	// and then in pixel coords!
	wtl = scale(SZ_F, SZ_F, 1.0f) * wtl;
	// FIXME TODO XXX: topleft or bottomleft origin?

	cout << wtl << endl;
	// normals pointing inside the box here, dunno if it matters
	// plane: n.p + d == 0
	box.zmin.p = plane{
		{0.0f, 0.0f, 1.0f},
		2.0f, // dist: plane normal 1, mul by coord -2, add 2 to get 0
		wtl // world_to_local
	};
	wtl = roty(90.0f/180.0f*3.14159f);
	wtl = translate(2.0f, 2.0f, 2.0f)*wtl;
	// size of the whole plane is now 4x4, need to make it 1x1
	wtl = scale(1.0f/4.0f, 1.0f/4.0f, 1.0f) * wtl;
	// and then in pixel coords!
	wtl = scale(SZ_F, SZ_F, 1.0f) * wtl;
	box.xmax.p = plane{
		{-1.0f, 0.0f, 0.0f},
		2.0f,
		wtl
	};
	wtl = roty(-90.0f/180.0f*3.14159f);
	wtl = translate(2.0f, 2.0f, 2.0f)*wtl;
	// size of the whole plane is now 4x4, need to make it 1x1
	wtl = scale(1.0f/4.0f, 1.0f/4.0f, 1.0f) * wtl;
	// and then in pixel coords!
	wtl = scale(SZ_F, SZ_F, 1.0f) * wtl;
	box.xmin.p = plane{
		{1.0f, 0.0f, 0.0f},
		2.0f,
		wtl
	};
	// first camera looking into zmin (front)
	// exactly at the middle
	//
	//      top
	// left front right back
	//      bottom

	// FIXME all planes properly
	box.ymax.tex = projectwhole(cams.data(), cams.size(), box.zmin.p); // top
	box.xmin.tex = projectwhole(cams.data(), cams.size(), box.xmin.p); // left
	box.zmin.tex = projectwhole(cams.data(), cams.size(), box.zmin.p); // front
	box.xmax.tex = projectwhole(cams.data(), cams.size(), box.xmax.p); // right
	box.zmax.tex = projectwhole(cams.data(), cams.size(), box.zmin.p); // back
	box.ymin.tex = projectwhole(cams.data(), cams.size(), box.zmin.p); // bottom
	Mat out(3*SZ, 4*SZ, CV_8UC3);
	box.ymax.tex.copyTo(out.rowRange(0, SZ).colRange(SZ, 2*SZ));
	box.xmin.tex.copyTo(out.rowRange(SZ, 2*SZ).colRange(0, SZ));
	box.zmin.tex.copyTo(out.rowRange(SZ, 2*SZ).colRange(SZ, 2*SZ));
	box.xmax.tex.copyTo(out.rowRange(SZ, 2*SZ).colRange(2*SZ, 3*SZ));
	box.zmax.tex.copyTo(out.rowRange(SZ, 2*SZ).colRange(3*SZ, 4*SZ));
	box.ymin.tex.copyTo(out.rowRange(2*SZ, 3*SZ).colRange(SZ, 2*SZ));
	imwrite("outfull.png", out);
}

int main() {
	test();
	testb();
	test2();
}
