#ifndef CONE_H
#define CONE_H

#include "hittable.h"

class cone : public hittable {
  public:
    cone(const point3& vertex, double height, double angle, shared_ptr<material> mat)
      : vertex(vertex), height(height), k(std::tan(angle)), mat(mat) {}

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        vec3 oc = r.origin() - vertex;  // Vector from cone vertex to ray origin

        auto d = r.direction();
        auto k2 = k * k;  // Precompute k squared

        // Coefficients for quadratic equation
        auto A = d.x() * d.x() + d.z() * d.z() - k2 * d.y() * d.y();
        auto B = 2 * (oc.x() * d.x() + oc.z() * d.z() - k2 * oc.y() * d.y());
        auto C = oc.x() * oc.x() + oc.z() * oc.z() - k2 * oc.y() * oc.y();

        // Solve quadratic equation
        auto discriminant = B * B - 4 * A * C;
        if (discriminant < 0)
            return false;

        auto sqrtd = std::sqrt(discriminant);

        // Find nearest valid root
        auto root = (-B - sqrtd) / (2 * A);
        if (!ray_t.surrounds(root)) {
            root = (-B + sqrtd) / (2 * A);
            if (!ray_t.surrounds(root))
                return false;
        }

        // Compute hit point
        rec.t = root;
        rec.p = r.at(rec.t);

        // Check if the hit is within the height limits
        double y = rec.p.y() - vertex.y();
        if (y < 0 || y > height)
            return false;

        // Compute normal
        vec3 outward_normal = vec3(rec.p.x(), -k2 * (rec.p.y() - vertex.y()), rec.p.z());
        rec.set_face_normal(r, outward_normal);
        rec.mat = mat;

        return true;
    }

  private:
    point3 vertex;  // The tip of the cone
    double height;  // Height of the cone
    double k;       // Tangent of the cone's half-angle
    shared_ptr<material> mat;
};

#endif
