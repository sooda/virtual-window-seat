#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/gpu/gpu.hpp>

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

struct camera {
	mat4 m;
	mat3 r;
	vec3 t;

	vec3 pos;
	mat4 local_to_world;
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

// project a point pt_2d in camera's image plane (local coords) on 3d plane p
vec2 camplane_to_plane(camera c, vec2 pt_2d, plane p) {
	cout << "camplane to plane: pt " << pt_2d << endl;
	// should be normalized to f already
	vec4 local_3d(pt_2d[0], pt_2d[1], -1.0f, 1.0f); // homog, note z dir
	// on virtual image plane in global coord
	vec4 pt_3d_hom = c.local_to_world * local_3d;
	cout << "pt 3d hom: " << pt_3d_hom << endl;
	vec3 pt_3d(ptfromhom(pt_3d_hom));
	vec3 v = unit(pt_3d - c.pos);
	cout << "v: " << v << endl;
	float t = intersect_vec_plane(c.pos, v, p);
	vec3 pt_on_plane = c.pos + t * v;
	cout << "pt on plane: " << pt_on_plane << endl;
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
	Mat m = obtain_dest(c, p, scale, src.size().width);
	Mat dst;
	cout<<"jee"<<endl;
	cout<<m<<endl;
	warpPerspective(src, dst, m, Size(1024, 1024));
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
				{}, // m
				{}, // r
				{}, // t

				{}, // pos
				ones(), // local to world: camera sits at origin
				{
					0.50f, // w (all these three in same units)
					0.25f, // h
					1.0f // f
				}
			},
			imread("cam0.png")
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
	wtl = scale(1024.0f, 1024.0f, 1.0f) * wtl;
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

int main() {
	test();
}
