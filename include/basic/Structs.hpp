#ifndef RENDERERBUILD_STRUCTS_HPP
#define RENDERERBUILD_STRUCTS_HPP

#include <basic/Point3.hpp>

namespace renderer {
    //图元类型枚举
    enum class PrimitiveType {
        SPHERE, TRIANGLE
    };

    //材质类型枚举
    enum class MaterialType {
        ROUGH, METAL
    };

    //碰撞信息
    struct HitRecord {
        Point3 hitPoint;
        Vec3 normalVector;
        double t;
        bool hitFrontFace;

        /*
         * 使用材质类型和材质索引（对应一种材质的不同对象）代替材质指针
         * 两个变量结合定位具体材质对象
         */
        MaterialType materialType;
        size_t materialIndex;

        std::pair<double, double> uvPair;
    };
}

#endif //RENDERERBUILD_STRUCTS_HPP
