#ifndef RAY_HPP
#define RAY_HPP

#include <basic/Point3.hpp>
#include <basic/Color3.hpp>
#include <basic/Structs.hpp>

namespace renderer {
    /*
     * 光线类。P(t) = A + tB，A为光线的起点，t为实数，B为光线的方向向量，一般为单位向量
     */
    class Ray {
    public:
        Point3 origin;
        Vec3 direction;
        double time;

        explicit Ray(const Point3 & origin = Point3(), const Vec3 & direction = Vec3(1.0, 0.0, 0.0),
                     double time = 0.0) : origin(origin), direction(direction), time(time) {}

        // ====== 对象操作函数 ======

        Point3 at(double t) const {
            return origin + t * direction;
        }
    };
}

#endif //RAY_HPP
