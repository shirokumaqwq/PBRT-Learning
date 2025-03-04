#ifndef PDF_H
#define PDF_H

#include "rtweekend.h"
#include "onb.h"
#include "hittable.h"
class pdf {
public:
    virtual ~pdf() {}

    virtual Float value(const vec3& direction) const = 0;
    virtual vec3 generate() const = 0;
};



class cosine_pdf : public pdf {
public:
    cosine_pdf(const vec3& w) { uvw.build_from_w(w); }

    virtual Float value(const vec3& direction) const override {
        auto cosine = Dot(unit_vector(direction), uvw.w());
        return (cosine <= 0) ? 0 : cosine / Pi;
    }

    virtual vec3 generate() const override {
        return uvw.local(RandomCosineDirection());
    }

public:
    onb uvw;
};

class hittable_pdf : public pdf {
public:
    hittable_pdf(shared_ptr<hittable> p, const point3& origin) : ptr(p), o(origin) {}

    virtual Float value(const vec3& direction) const override {
        return ptr->pdf_value(o, direction);
    }

    virtual vec3 generate() const override {
        return ptr->random(vec3(o.x,o.y,o.z));
    }

public:
    point3 o;
    shared_ptr<hittable> ptr;
};

class mixture_pdf : public pdf {
public:
    mixture_pdf(shared_ptr<pdf> p0, shared_ptr<pdf> p1) {
        p[0] = p0;
        p[1] = p1;
    }

    virtual Float value(const vec3& direction) const override {
        return 0.5 * p[0]->value(direction) + 0.5 * p[1]->value(direction);
    }

    virtual vec3 generate() const override {
        if (RandomFloat() < 0.5)
            return p[0]->generate();
        else
            return p[1]->generate();
    }

public:
    shared_ptr<pdf> p[2];
};
#endif