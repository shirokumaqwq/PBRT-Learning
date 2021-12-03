#ifndef HITTABLE_H
#define HITTABLE_H

#include "rtweekend.h"
#include "aabb.h"
class material;
class Shape;
class aabb;

struct hit_record {
    point3 p;
    vec3 normal;
    Float time;
    Float u;
    Float v;
    bool front_face;
    vec3 pError;
    Normal n;
    vec3 wo;
    shared_ptr<material> mat_ptr;
    hit_record():time(0){}
    hit_record(const point3& p, const Normal& n, const vec3& pError,
        const vec3& wo, Float time)
        : p(p), time(time), pError(pError), wo(wo), n(n){ }

    inline void set_face_normal(const ray& r, const vec3& outward_normal) {
        front_face = Dot(r.direction(), outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
    bool IsSurfaceInteraction() const {
        return n != Normal();
    }
};

typedef hit_record Interaction;

class SurfaceInteraction :public hit_record {
public:

    struct {
        Normal n;
        vec3 dpdu, dpdv;
        Normal dndu, dndv;
    } shading;

    SurfaceInteraction() {}
    SurfaceInteraction(
        const point3& p, const vec3& pError, Point2f uv,
       // Float u,Float v,
        const vec3& wo, const vec3& dpdu, const vec3& dpdv,
        const Normal& dndu, const Normal& dndv, Float time, const Shape* shape,
        int faceIndex = 0);

    void SetShadingGeometry(const vec3& dpdus,
        const vec3& dpdvs, const Normal& dndus,
        const Normal& dndvs, bool orientationIsAuthoritative) {
            shading.n = Normalize((Normal)Cross(dpdus, dpdvs));
        /*if (shape && (shape->reverseOrientation ^
            shape->transformSwapsHandedness))
            shading.n = -shading.n;*/
        if (orientationIsAuthoritative)
            n = Faceforward(n, vec3(shading.n));
        else
            shading.n = Faceforward(shading.n, vec3(n));

        shading.dpdu = dpdus;
        shading.dpdv = dpdvs;
        shading.dndu = dndus;
        shading.dndv = dndvs;

    }

    Point2f uv;
    vec3 dpdu, dpdv;
    Normal dndu, dndv;
    const Shape* shape = nullptr;

    mutable vec3 dpdx, dpdy;
    mutable Float dudx = 0, dvdx = 0, dudy = 0, dvdy = 0;

};


class hittable {
public:
    virtual bool hit(const ray& r, Float t_min, Float t_max, hit_record& rec) const = 0;

    virtual bool bounding_box(Float time0, Float time1, aabb& output_box) const = 0;
    virtual Float pdf_value(const point3& o, const vec3& v) const {
        return 0.0;
    }

    virtual vec3 random(const vec3& o) const {
        return vec3(1, 0, 0);
    }
};

class translate : public hittable {
public:
    translate(shared_ptr<hittable> p, const vec3& displacement)
        : ptr(p), offset(displacement) {}

    virtual bool hit(
        const ray& r, Float t_min, Float t_max, hit_record& rec) const override;

    virtual bool bounding_box(Float time0, Float time1, aabb& output_box) const override;

public:
    shared_ptr<hittable> ptr;
    vec3 offset;
};

class rotate_y : public hittable {
public:
    rotate_y(shared_ptr<hittable> p, Float angle);

    virtual bool hit(
        const ray& r, Float t_min, Float t_max, hit_record& rec) const override;

    virtual bool bounding_box(Float time0, Float time1, aabb& output_box) const override;

public:
    shared_ptr<hittable> ptr;
    Float sin_theta;
    Float cos_theta;
    bool hasbox;
    aabb bbox;
};

//inline bool translate::hit(const ray& r, Float t_min, Float t_max, hit_record& rec) const {
//    ray moved_r(r.origin() - offset, r.direction(), r.Time());
//    if (!ptr->hit(moved_r, t_min, t_max, rec))
//        return false;
//
//    rec.p += offset;
//    rec.set_face_normal(moved_r, rec.normal);
//
//    return true;
//}
//
//inline bool translate::bounding_box(Float time0, Float time1, aabb& output_box) const {
//    if (!ptr->bounding_box(time0, time1, output_box))
//        return false;
//
//    output_box = aabb(
//        output_box.min() + offset,
//        output_box.max() + offset);
//
//    return true;
//}
//
//inline rotate_y::rotate_y(shared_ptr<hittable> p, Float angle) : ptr(p) {
//    auto radians = Radians(angle);
//    sin_theta = sin(radians);
//    cos_theta = cos(radians);
//    hasbox = ptr->bounding_box(0, 1, bbox);
//
//    point3 min(Infinity, Infinity, Infinity);
//    point3 max(-Infinity, -Infinity, -Infinity);
//
//    for (int i = 0; i < 2; i++) {
//        for (int j = 0; j < 2; j++) {
//            for (int k = 0; k < 2; k++) {
//                auto x = i * bbox.max().x + (1 - i) * bbox.min().x;
//                auto y = j * bbox.max().y + (1 - j) * bbox.min().y;
//                auto z = k * bbox.max().z + (1 - k) * bbox.min().z;
//
//                auto newx = cos_theta * x + sin_theta * z;
//                auto newz = -sin_theta * x + cos_theta * z;
//
//                vec3 tester(newx, y, newz);
//
//                for (int c = 0; c < 3; c++) {
//                    min[c] = fmin(min[c], tester[c]);
//                    max[c] = fmax(max[c], tester[c]);
//                }
//            }
//        }
//    }
//
//    bbox = aabb(min, max);
//}
//
//inline bool rotate_y::hit(const ray& r, Float t_min, Float t_max, hit_record& rec) const {
//    auto origin = r.origin();
//    auto direction = r.direction();
//
//    origin[0] = cos_theta * r.origin()[0] - sin_theta * r.origin()[2];
//    origin[2] = sin_theta * r.origin()[0] + cos_theta * r.origin()[2];
//
//    direction[0] = cos_theta * r.direction()[0] - sin_theta * r.direction()[2];
//    direction[2] = sin_theta * r.direction()[0] + cos_theta * r.direction()[2];
//
//    ray rotated_r(origin, direction, r.Time());
//
//    if (!ptr->hit(rotated_r, t_min, t_max, rec))
//        return false;
//
//    auto p = rec.p;
//    auto normal = rec.normal;
//
//    p[0] = cos_theta * rec.p[0] + sin_theta * rec.p[2];
//    p[2] = -sin_theta * rec.p[0] + cos_theta * rec.p[2];
//
//    normal[0] = cos_theta * rec.normal[0] + sin_theta * rec.normal[2];
//    normal[2] = -sin_theta * rec.normal[0] + cos_theta * rec.normal[2];
//
//    rec.p = p;
//    rec.set_face_normal(rotated_r, normal);
//
//    return true;
//}

#endif