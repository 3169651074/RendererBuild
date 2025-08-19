#ifndef RENDERERTEST_SPHERE_HPP
#define RENDERERTEST_SPHERE_HPP

#include <box/BoundingBox.hpp>

namespace renderer {
    /*
     * 球体类，包围盒不在构造函数中计算，而是在构建BVH树时统一计算
     */
    class Sphere {
    public:
        //物体属性
        Ray center;
        double radius;

        //材质属性
        MaterialType materialType;
        size_t materialIndex;

        //当前物体在物体数组中的下标
        size_t objectID;

        //将位于球体表面的点转换为二维坐标（u, v）
        static std::pair<double, double> mapUVPair(const Point3 & surfacePoint) {
            const double theta = std::acos(-surfacePoint[1]);
            const double phi = std::atan2(-surfacePoint[2], surfacePoint[0]) + PI;

            return {phi / (2.0 * PI), theta / PI};
        }

        //构造静止球体
        Sphere(size_t objectID, MaterialType materialType, size_t materialIndex, const Point3 & center, double radius) :
            objectID(objectID), materialType(materialType), materialIndex(materialIndex), center(Ray(center, Vec3())), radius(radius > 0.0 ? radius : 0.0) {}

        //构造运动球体
        Sphere(size_t objectID, MaterialType materialType, size_t materialIndex, const Point3 & from, const Point3 & to, double radius) :
            objectID(objectID), materialType(materialType), materialIndex(materialIndex), center(Ray(from, Point3::constructVector(from, to))), radius(radius > 0.0 ? radius : 0.0) {}

        //构造包围盒
        BoundingBox constructBoundingBox() const {
            const Vec3 edge = Vec3(radius, radius, radius);
            if (center.direction.lengthSquare() < Vec3::VECTOR_LENGTH_SQUARE_ZERO_EPSILON) {
                return {center.origin - edge, center.origin + edge};
            } else {
                const Point3 to = center.origin + center.direction;
                const auto bStart = BoundingBox(center.origin - edge, center.origin + edge);
                const auto bEnd = BoundingBox(to - edge, to + edge);
                return {bStart, bEnd};
            }
        }

        bool hit(const Ray & ray, const Range & range, HitRecord & record) const {
            //获取球体在当前时间的中心位置
            const Point3 currentCenter = center.at(ray.time);

            //解一元二次方程，判断光线和球体的交点个数
            const Vec3 cq = Point3::constructVector(ray.origin, currentCenter);
            const Vec3 dir = ray.direction;
            const double a = Vec3::dot(dir, dir);
            const double b = -2.0 * Vec3::dot(cq, dir);
            const double c = Vec3::dot(cq, cq) - radius * radius;
            double delta = b * b - 4.0 * a * c;

            if (delta < 0.0) return false;
            delta = sqrt(delta);

            //root1对应较小的t值，为距离摄像机较近的交点
            const double root1 = (-b - delta) / (a * 2.0);
            const double root2 = (-b + delta) / (a * 2.0);

            double root;
            if (range.inRange(root1)) { //先判断root1
                root = root1;
            } else if (range.inRange(root2)) {
                root = root2;
            } else {
                return false; //两个根均不在允许范围内
            }

            //设置碰撞信息
            record.t = root;
            record.hitPoint = ray.at(root);
            record.materialType = materialType;
            record.materialIndex = materialIndex;

            //outwardNormal为球面向外的单位法向量，通过此向量和光线方向向量的点积符号判断光线撞击了球的内表面还是外表面
            //若点积小于0，则两向量夹角大于90度，两向量不同方向
            const Vec3 outwardNormal = Point3::constructVector(currentCenter, record.hitPoint).unitVector();
            record.hitFrontFace = Vec3::dot(ray.direction, outwardNormal) < 0;
            record.normalVector = record.hitFrontFace ? outwardNormal : -outwardNormal;

            //将碰撞点从世界坐标系变换到以球心为原点的局部坐标系：直接在世界坐标系中构造向量
            const Vec3 localVector = Point3::constructVector(currentCenter, record.hitPoint).unitVector();
            record.uvPair = mapUVPair(Point3(localVector));
            return true;
        }
    };
}

#endif //RENDERERTEST_SPHERE_HPP
