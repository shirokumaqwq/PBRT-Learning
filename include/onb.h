#ifndef ONB_H
#define ONB_H
#include "rtweekend.h"

class onb {
public:
    onb() {}

    inline vec3 operator[](int i) const { return axis[i]; }

    vec3 u() const { return axis[0]; }
    vec3 v() const { return axis[1]; }
    vec3 w() const { return axis[2]; }

    vec3 local(Float a, Float b, Float c) const {
        return a * u() + b * v() + c * w();
    }

    vec3 local(const vec3& a) const {
        return a.x * u() + a.y * v() + a.z * w();
    }

    void build_from_w(const vec3&);

public:
    vec3 axis[3];
};


inline void onb::build_from_w(const vec3& n) {
    axis[2] = unit_vector(n);
    vec3 a = (fabs(w().x) > 0.9) ? vec3(0, 1, 0) : vec3(1, 0, 0);
    axis[1] = unit_vector(Cross(w(), a));
    axis[0] = Cross(w(), v());
}

#endif