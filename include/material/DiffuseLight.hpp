#ifndef RENDERERBUILD_DIFFUSELIGHT_HPP
#define RENDERERBUILD_DIFFUSELIGHT_HPP

#include <basic/Ray.hpp>
#include <basic/Color3.hpp>

namespace renderer {
    class DiffuseLight {
    private:
        Color3 light;

    public:
        explicit DiffuseLight(const Color3 & lightColor) : light(lightColor) {}
        ~DiffuseLight() = default;

        Color3 emitted(const Ray & ray, const HitRecord & record) const {
            //仅当光线和法线同向时返回光源颜色
            if (record.hitFrontFace) {
                return light;
            } else {
                return Color3();
            }
        }
    };
}

#endif //RENDERERBUILD_DIFFUSELIGHT_HPP
