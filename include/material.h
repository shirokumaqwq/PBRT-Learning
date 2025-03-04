#ifndef MATERIAL_H
#define MATERIAL_H

#include "rtweekend.h"
#include "hittable.h"
#include "texture.h"
#include "onb.h"
#include "pdf.h"
#include "spectrum.h"

enum class TransportMode { Radiance, Importance };

struct hit_record;

struct scatter_record {
    ray specular_ray;
    bool is_specular;
    Color attenuation;
    shared_ptr<pdf> pdf_ptr;
};

class Material {
public:
    // Material Interface
    virtual void ComputeScatteringFunctions(SurfaceInteraction* si,
        //MemoryArena& arena,
        TransportMode mode,
        bool allowMultipleLobes) const = 0;
    virtual ~Material();
    //static void Bump(const std::shared_ptr<Texture<Float>>& d,
    //    SurfaceInteraction* si);
};

class material {
public:
    virtual Color emitted(
        const ray& r_in, const hit_record& rec, Float u, Float v, const point3& p) const {
        return Color(0.f);
    }
    virtual bool scatter(
        const ray& r_in, const hit_record& rec, Color& attenuation, ray& scattered
    ) const {
        return false;
    }
    virtual bool scatter(
        const ray& r_in, const hit_record& rec, scatter_record& srec
    ) const {
        return false;
    }

    virtual Float scattering_pdf(
        const ray& r_in, const hit_record& rec, const ray& scattered
    ) const {
        return 0;
    }
};

class lambertian : public material {
public:
    lambertian(const color& a) : albedo(make_shared<solid_color>(a)) {}
    lambertian(shared_ptr<texture> a) : albedo(a) {}


    virtual bool scatter(
        const ray& r_in, const hit_record& rec, scatter_record& srec
    ) const override {
        srec.is_specular = false;
        srec.attenuation = Color::FromRGB(albedo->value(rec.u, rec.v, rec.p));
        srec.pdf_ptr = make_shared<cosine_pdf>(rec.normal);
        return true;
    }

    Float scattering_pdf(
        const ray& r_in, const hit_record& rec, const ray& scattered
    ) const {
        auto cosine = Dot(rec.normal, unit_vector(scattered.direction()));
        return cosine < 0 ? 0 : cosine / Pi;
    }

public:
    shared_ptr<texture> albedo;
};


class metal : public material {
public:
    metal(const color& a, Float f) : albedo(Color::FromRGB(a)), fuzz(f < 1 ? f : 1) {}

    virtual bool scatter(
        const ray& r_in, const hit_record& rec, scatter_record& srec
    ) const override {
        vec3 reflected = reflect(unit_vector(r_in.direction()), rec.normal);
        srec.specular_ray = ray(rec.p, reflected + fuzz * random_in_unit_sphere());
        srec.attenuation = albedo;
        srec.is_specular = true;
        srec.pdf_ptr = 0;
        return true;
    }

public:
    Color albedo;
    Float fuzz;
};

class dielectric : public material {
public:
    dielectric(Float index_of_refraction) : ir(index_of_refraction) {}

    virtual bool scatter(
        const ray& r_in, const hit_record& rec, scatter_record& srec
    ) const override {
        srec.is_specular = true;
        srec.pdf_ptr = nullptr;
        srec.attenuation = Color(1.0);
        Float refraction_ratio = rec.front_face ? (1.0 / ir) : ir; //从哪侧进入决定折射率

        vec3 unit_direction = unit_vector(r_in.direction());
        Float cos_theta = fmin(Dot(-unit_direction, rec.normal), 1.0);
        Float sin_theta = sqrt(1.0 - cos_theta * cos_theta);

        bool cannot_refract = refraction_ratio * sin_theta > 1.0;
        vec3 direction;

        if (cannot_refract || reflectance(cos_theta, refraction_ratio) > RandomFloat())
            direction = reflect(unit_direction, rec.normal);
        else
            direction = refract(unit_direction, rec.normal, refraction_ratio);

        srec.specular_ray = ray(rec.p, direction, r_in.Time());
        return true;
    }

public:
    Float ir; // Index of Refraction

private:
    static Float reflectance(Float cosine, Float ref_idx) {
        // Use Schlick's approximation for reflectance.
        auto r0 = (1 - ref_idx) / (1 + ref_idx);
        r0 = r0 * r0;
        return r0 + (1 - r0) * pow((1 - cosine), 5);
    }
};

class diffuse_light : public material {
public:
    diffuse_light(shared_ptr<texture> a) : emit(a) {}
    diffuse_light(color c) : emit(make_shared<solid_color>(c)) {}

    virtual bool scatter(
        const ray& r_in, const hit_record& rec, Color& attenuation, ray& scattered
    ) const override {
        return false;
    }

    virtual Color emitted(const ray& r_in, const hit_record& rec, Float u, Float v, const point3& p) const override {
        return Color::FromRGB(emit->value(u, v, p),SpectrumType::Illuminant);
    }

public:
    shared_ptr<texture> emit;
};

class isotropic : public material {
public:
    isotropic(color c) : albedo(make_shared<solid_color>(c)) {}
    isotropic(shared_ptr<texture> a) : albedo(a) {}

    virtual bool scatter(
        const ray& r_in, const hit_record& rec, Color& attenuation, ray& scattered
    ) const override {
        scattered = ray(rec.p, random_in_unit_sphere(), r_in.Time());
        attenuation = Color::FromRGB(albedo->value(rec.u, rec.v, rec.p));
        return true;
    }

public:
    shared_ptr<texture> albedo;
};


#endif