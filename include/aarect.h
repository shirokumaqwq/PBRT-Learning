#ifndef AARECT_H
#define AARECT_H

#include "rtweekend.h"

#include "hittable.h"

class xy_rect : public hittable {
public:
    xy_rect() {}

    //Ϊ����{z=k,x0<x<x1,y0<y<y1}
    xy_rect(Float _x0, Float _x1, Float _y0, Float _y1, Float _k,
        shared_ptr<material> mat)
        : x0(_x0), x1(_x1), y0(_y0), y1(_y1), k(_k), mp(mat) {};

    virtual bool hit(const ray& r, Float t_min, Float t_max, hit_record& rec) const override;

    virtual bool bounding_box(Float time0, Float time1, aabb& output_box) const override {
        // The bounding box must have non-zero width in each dimension, so pad the Z
        // dimension a small amount.
        output_box = aabb(point3(x0, y0, k - 0.0001), point3(x1, y1, k + 0.0001));
        return true;
    }

public:
    shared_ptr<material> mp;
    Float x0, x1, y0, y1, k;
};

bool xy_rect::hit(const ray& r, Float t_min, Float t_max, hit_record& rec) const {
    auto t = (k - r.origin().z) / r.direction().z;
    if (t < t_min || t > t_max)
        return false;
    auto x = r.origin().x + t * r.direction().x;
    auto y = r.origin().y + t * r.direction().y;
    if (x < x0 || x > x1 || y < y0 || y > y1)
        return false;
    rec.u = (x - x0) / (x1 - x0);
    rec.v = (y - y0) / (y1 - y0);
    rec.time = t;
    auto outward_normal = vec3(0, 0, 1);
    rec.set_face_normal(r, outward_normal);
    rec.mat_ptr = mp;
    rec.p = r.at(t);
    return true;
}

class xz_rect : public hittable {
public:
    xz_rect() {}

    xz_rect(Float _x0, Float _x1, Float _z0, Float _z1, Float _k,
        shared_ptr<material> mat)
        : x0(_x0), x1(_x1), z0(_z0), z1(_z1), k(_k), mp(mat) {};

    virtual bool hit(const ray& r, Float t_min, Float t_max, hit_record& rec) const override;

    virtual bool bounding_box(Float time0, Float time1, aabb& output_box) const override {
        // The bounding box must have non-zero width in each dimension, so pad the Y
        // dimension a small amount.
        output_box = aabb(point3(x0, k - 0.0001, z0), point3(x1, k + 0.0001, z1));
        return true;
    }
    virtual Float pdf_value(const point3& origin, const vec3& v) const override {
        hit_record rec;
        if (!this->hit(ray(origin, v), 0.001, Infinity, rec))
            return 0;

        auto area = (x1 - x0) * (z1 - z0);
        auto distance_squared = rec.time * rec.time * v.LengthSquared();
        auto cosine = fabs(Dot(v, rec.normal) / v.Length());

        return distance_squared / (cosine * area);
    }

    virtual vec3 random(const vec3& origin) const override {
        auto random_point = vec3(RandomFloat(x0, x1), k, RandomFloat(z0, z1));
        return random_point - origin;
    }

public:
    shared_ptr<material> mp;
    Float x0, x1, z0, z1, k;
};

class yz_rect : public hittable {
public:
    yz_rect() {}

    yz_rect(Float _y0, Float _y1, Float _z0, Float _z1, Float _k,
        shared_ptr<material> mat)
        : y0(_y0), y1(_y1), z0(_z0), z1(_z1), k(_k), mp(mat) {};

    virtual bool hit(const ray& r, Float t_min, Float t_max, hit_record& rec) const override;

    virtual bool bounding_box(Float time0, Float time1, aabb& output_box) const override {
        // The bounding box must have non-zero width in each dimension, so pad the X
        // dimension a small amount.
        output_box = aabb(point3(k - 0.0001, y0, z0), point3(k + 0.0001, y1, z1));
        return true;
    }

public:
    shared_ptr<material> mp;
    Float y0, y1, z0, z1, k;
};

bool xz_rect::hit(const ray& r, Float t_min, Float t_max, hit_record& rec) const {
    auto t = (k - r.origin().y) / r.direction().y;
    if (t < t_min || t > t_max)
        return false;
    auto x = r.origin().x + t * r.direction().x;
    auto z = r.origin().z + t * r.direction().z;
    if (x < x0 || x > x1 || z < z0 || z > z1)
        return false;
    rec.u = (x - x0) / (x1 - x0);
    rec.v = (z - z0) / (z1 - z0);
    rec.time = t;
    auto outward_normal = vec3(0, 1, 0);
    rec.set_face_normal(r, outward_normal);
    rec.mat_ptr = mp;
    rec.p = r.at(t);
    return true;
}

bool yz_rect::hit(const ray& r, Float t_min, Float t_max, hit_record& rec) const {
    auto t = (k - r.origin().x) / r.direction().x;
    if (t < t_min || t > t_max)
        return false;
    auto y = r.origin().y + t * r.direction().y;
    auto z = r.origin().z + t * r.direction().z;
    if (y < y0 || y > y1 || z < z0 || z > z1)
        return false;
    rec.u = (y - y0) / (y1 - y0);
    rec.v = (z - z0) / (z1 - z0);
    rec.time = t;
    auto outward_normal = vec3(1, 0, 0);
    rec.set_face_normal(r, outward_normal);
    rec.mat_ptr = mp;
    rec.p = r.at(t);
    return true;
}
#endif

#ifndef BOX_H
#define BOX_H

#include "hittable_list.h"

class box : public hittable {
public:
    box() {}
    box(const point3& p0, const point3& p1, shared_ptr<material> ptr);

    virtual bool hit(const ray& r, Float t_min, Float t_max, hit_record& rec) const override;

    virtual bool bounding_box(Float time0, Float time1, aabb& output_box) const override {
        output_box = aabb(box_min, box_max);
        return true;
    }

public:
    point3 box_min;
    point3 box_max;
    hittable_list sides;
};

box::box(const point3& p0, const point3& p1, shared_ptr<material> ptr) {
    box_min = p0;
    box_max = p1;

    sides.add(make_shared<xy_rect>(p0.x, p1.x, p0.y, p1.y, p1.z, ptr));
    sides.add(make_shared<xy_rect>(p0.x, p1.x, p0.y, p1.y, p0.z, ptr));

    sides.add(make_shared<xz_rect>(p0.x, p1.x, p0.z, p1.z, p1.y, ptr));
    sides.add(make_shared<xz_rect>(p0.x, p1.x, p0.z, p1.z, p0.y, ptr));

    sides.add(make_shared<yz_rect>(p0.y, p1.y, p0.z, p1.z, p1.x, ptr));
    sides.add(make_shared<yz_rect>(p0.y, p1.y, p0.z, p1.z, p0.x, ptr));
}

bool box::hit(const ray& r, Float t_min, Float t_max, hit_record& rec) const {
    return sides.hit(r, t_min, t_max, rec);
}

#endif