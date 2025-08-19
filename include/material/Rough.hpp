#ifndef RENDERERTEST_ROUGH_HPP
#define RENDERERTEST_ROUGH_HPP

#include <basic/Ray.hpp>
#include <basic/Color3.hpp>

namespace renderer {
    /*
     * 粗糙材质类
     */
    class Rough {
    private:
        Color3 albedo;

    public:
        explicit Rough(const Color3 & albedo) : albedo(albedo) {}

        //粗糙材质：漫反射
        bool scatter(const Ray & in, const HitRecord & record, Color3 & attenuation, Ray & out) const {
            //随机选择一个反射方向
            Vec3 reflectDirection = (record.normalVector + Vec3::randomSpaceVector(1.0)).unitVector();

            //随机的反射方向可能和法向量相互抵消，此时取消随机反射
            if (floatValueEquals(reflectDirection.lengthSquare(), Vec3::VECTOR_LENGTH_SQUARE_ZERO_EPSILON)) {
                reflectDirection = record.normalVector;
            }

            //从反射点出发构造反射光线
            out = Ray(record.hitPoint, reflectDirection, in.time);
            attenuation = albedo;
            return true;
        }
    };
}

#endif //RENDERERTEST_ROUGH_HPP
