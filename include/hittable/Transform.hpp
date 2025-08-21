#ifndef RENDERERBUILD_TRANSFORM_HPP
#define RENDERERBUILD_TRANSFORM_HPP

#include <box/BoundingBox.hpp>
#include <hittable/Sphere.hpp>
#include <hittable/Triangle.hpp>
#include <hittable/Parallelogram.hpp>
#include <hittable/Box.hpp>

namespace renderer {
    /*
     * 使用4x4变换矩阵表示的变换类
     */
    class Transform {
    public:
        //指向物体数组的指针和物体索引
        const void * primitiveArray;
        PrimitiveType primitiveType;
        size_t primitiveIndex;

        //变换矩阵
        Matrix transformMatrix;
        Matrix transformInverse;
        Matrix transformInverseTranspose;

        //变换后物体的包围盒和中心点
        BoundingBox transformedBoundingBox;
        Point3 transformedCentroid;

        Transform(const void * primitiveArray, PrimitiveType primitiveType, size_t primitiveIndex, BoundingBox boundingBox, Point3 centroid,
                  const std::array<double, 3> & rotate = {}, const std::array<double, 3> & shift = {}, const std::array<double, 3> & scale = {1.0, 1.0, 1.0}) :
                  primitiveArray(primitiveArray), primitiveType(primitiveType), primitiveIndex(primitiveIndex), transformMatrix(4, 4), transformInverse(4, 4), transformInverseTranspose(4, 4)
        {
            //M = T * R * S，平移 * 旋转 * 缩放
            const auto m1 = Matrix::constructShiftMatrix(shift);
            const auto m2 = Matrix::constructRotateMatrix(rotate);
            const auto m3 = Matrix::constructScaleMatrix(scale);
            transformMatrix = m1 * m2 * m3;
            transformInverse = transformMatrix.inverse();
            transformInverseTranspose = transformInverse.transpose();

            //变换包围盒和中心点
            this->transformedBoundingBox = boundingBox.transformBoundingBox(transformMatrix);
            this->transformedCentroid = (transformMatrix * Matrix::toMatrix(centroid.toVector(), 1.0)).toPoint();
        }
        ~Transform() = default;

        /*
         * Transform类为一个hittable，同时也是一个托管，可能包含任何图元
         */
        bool hit(const Ray &ray, const Range &range, HitRecord &record) const
        {
            /*
             * 将世界空间光线变换到物体的局部空间：使用逆矩阵分别对ray的起点和方向向量进行变换
             * 只有左矩阵的列数和右矩阵的行数相同的矩阵才能相乘，则将三维点变为1列4行的列向量
             */
            auto rayOrigin = Matrix::toMatrix(ray.origin.toVector(), 1.0);
            auto rayDirection = Matrix::toMatrix(ray.direction, 0.0);
            rayOrigin = transformInverse * rayOrigin;
            rayDirection = transformInverse * rayDirection;

            //构造变换后的光线
            const Ray transformed(rayOrigin.toPoint(), rayDirection.toPoint().toVector(), ray.time);

            //在物体空间中对变换后的光线进行相交测试
            bool isHit = false;
            switch (primitiveType) {
                case PrimitiveType::SPHERE: { //case不产生作用域，需要通过大括号手动创建，适用于case内定义变量
                    //C++标准规定void*到具体类型的转换使用static_cast
                    const Sphere * spheres = static_cast<const Sphere *>(primitiveArray);
                    if (spheres[primitiveIndex].hit(transformed, range, record)) {
                        isHit = true;
                    }
                    break;
                }
                case PrimitiveType::TRIANGLE: {
                    const Triangle * triangles = static_cast<const Triangle *>(primitiveArray);
                    if (triangles[primitiveIndex].hit(transformed, range, record)) {
                        isHit = true;
                    }
                    break;
                }
                case PrimitiveType::PARALLELOGRAM: {
                    const Parallelogram * parallelograms = static_cast<const Parallelogram *>(primitiveArray);
                    if (parallelograms[primitiveIndex].hit(transformed, range, record)) {
                        isHit = true;
                    }
                    break;
                }
                case PrimitiveType::BOX: {
                    const Box * boxes = static_cast<const Box *>(primitiveArray);
                    if (boxes[primitiveIndex].hit(transformed, range, record)) {
                        isHit = true;
                    }
                    break;
                }
                default:;
            }

            if (!isHit) {
                return false;
            } else {
                //如果有碰撞，则将将局部空间的命中记录变换回世界空间，t值和uv坐标不需要变换
                //变换碰撞点
                auto point = Matrix::toMatrix(record.hitPoint.toVector(), 1.0);
                point = transformMatrix * point;
                record.hitPoint = point.toPoint();

                //使用逆转置变换矩阵变换法向量
                //向量不受平移影响，将w分量设置为0可完成此目的
                auto normal = Matrix::toMatrix(record.normalVector, 0.0);
                normal = transformInverseTranspose * normal;
                record.normalVector = normal.toPoint().toVector().unitVector();
                record.hitFrontFace = Vec3::dot(ray.direction, record.normalVector) < 0.0;
                return true;
            }
        }
    };
}

#endif //RENDERERBUILD_TRANSFORM_HPP
