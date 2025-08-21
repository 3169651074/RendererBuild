#ifndef RENDERERBUILD_HITTABLEPDF_HPP
#define RENDERERBUILD_HITTABLEPDF_HPP

#include <basic/Point3.hpp>
#include <basic/Structs.hpp>
#include <hittable/Sphere.hpp>
#include <hittable/Parallelogram.hpp>
#include <hittable/Triangle.hpp>
#include <hittable/Transform.hpp>

namespace renderer {
    /*
     * 直接向物体采样的PDF
     * origin为光线的起点（当前碰撞点）
     */
    class HittablePDF {
    private:
        //物体索引
        PrimitiveType primitiveType;
        size_t primitiveIndex;

        //当前碰撞点
        Point3 origin;

    public:
        HittablePDF(PrimitiveType primitiveType = PrimitiveType::SPHERE, size_t primitiveIndex = 0, const Point3 & origin = Point3()) :
                primitiveType(primitiveType), primitiveIndex(primitiveIndex), origin(origin) {}

        ~HittablePDF() = default;

        Vec3 generate(const Sphere * spheres, const Parallelogram * parallelograms) const {
            //从碰撞点指向物体上任意一点
            switch (primitiveType) {
                case PrimitiveType::SPHERE:
                    return spheres[primitiveIndex].randomVector(origin);
                case PrimitiveType::PARALLELOGRAM:
                    return parallelograms[primitiveIndex].randomVector(origin);
                    //TODO Triangle, Transform的randomVector和pdfValue方法实现
                default:
                    return Vec3();
            }
        }

        double value(const Sphere * spheres, const Parallelogram * parallelograms,
                     const Vec3 &vec) const {
            const Vec3 unitVec = vec.unitVector();
            switch (primitiveType) {
                case PrimitiveType::SPHERE:
                    return spheres[primitiveIndex].pdfValue(origin, unitVec);
                case PrimitiveType::PARALLELOGRAM:
                    return parallelograms[primitiveIndex].pdfValue(origin, unitVec);
                default:
                    return 0.0;
            }
        }
    };
}

#endif //RENDERERBUILD_HITTABLEPDF_HPP
