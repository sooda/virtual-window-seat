using namespace std;
using namespace cv;

typedef Matx33f mat3;
// or Vec2f?
typedef Point3f vec3;
typedef Point2f vec2;

struct camera {
	mat4 m;
	mat3 r;
	vec3 t;

	vec3 pos;
	mat4 origin_to_local;
	struct physical {
		float w;
		float h;
		float f;
	};
};

struct plane {
	vec3 n;
	float d;
	vec3 x0; // or d: n . x + d == 0
};

vec3 unit(vec3 v) {
	return v * (1.0f / norm(v));
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
	return -(plane.d + plane.n.dot(origin)) / (plane.n.dot(dir));
}

// c.pos in global
vec2 camplane_to_plane(camera c, float f, float w, float h, vec2 pt_2d, plane p) {
	// normalized to f
	vec3 local_3d(pt_2d.x, pt_2d.y, 1.0f);
	vec3 pt_3d = c.origin_to_local * local_3d;
	vec3 v = unit(pt_3d - c.pos);
	float t = intersect_vec_plane(c.pos, v, p);
	vec3 pt_on_plane = c.pos + t * v;

	vec3 pt_on_plane_by_pt = p.world_to_local * pt_on_plane;
	return vec2(pt_on_plane_by_pt.x, pt_on_plane_by_pt.y); // * scaling?
}

Mat obtain_dest(camera c, plane p, vec2 scale) {
	// "one" in tex pixel coords
	float tex1 = 1024.0;
	vec2 a = corner_to_image(cam_ext, f, w, h, scale * vec2(-w/2,  h/2), p);
	vec2 b = corner_to_image(cam_ext, f, w, h, scale * vec2( w/2,  h/2), p);
	vec2 c = corner_to_image(cam_ext, f, w, h, scale * vec2( w/2, -h/2), p);
	vec2 d = corner_to_image(cam_ext, f, w, h, scale * vec2(-w/2, -h/2), p);
	vec2 src_pixels[] = {
		vec2(0.0f, 0.0f),
		vec2(tex1, 0.0f),
		vec2(tex1, tex1),
		vec2(0.0f, tex1)
	};
	vec2 dst_pixels[] = {a, b, c, d};
	Mat m = getPerspectiveTransform(src_pixels, dst_pixels);
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

void project(camera c, plane p, image src, image dst) {
	vec2 scale(c.physical.w / c.physical.f, c.physical.h / c.physical.f); // TODO
	Mat m = obtain_dest(c, p, scale);
	dst = warpPerspective(img,M,(2000,2000))
}

void test() {
	camdata cams[] = {
		{
			camera(),
			Mat()
		},
		{
			camera(),
			Mat()
		};
	};

	for (camdata& cams: c) {
}
