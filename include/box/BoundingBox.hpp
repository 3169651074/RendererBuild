#ifndef RENDERERBUILD_BOUNDINGBOX_HPP
#define RENDERERBUILD_BOUNDINGBOX_HPP

#include <basic/Ray.hpp>
#include <util/Range.hpp>

namespace renderer {
    /*
     * 轴对齐包围盒
     */
    class BoundingBox {
    private:
        Range range[3];

        //确保包围盒体积有效
        void ensureVolume() {
            constexpr double EPSILON = FLOAT_VALUE_ZERO_EPSILON;
            for (auto & i : range) {
                if (i.length() < EPSILON) {
                    i.expand(EPSILON);
                }
            }
        }

    public:
        //默认构造空包围盒
        explicit BoundingBox(const Range & x = Range(), const Range & y = Range(), const Range & z = Range()) {
            range[0] = x; range[1] = y; range[2] = z;
            ensureVolume();
        }

        //使用两个对角点构造长方体
        BoundingBox(const Point3 & p1, const Point3 & p2) {
            //取两个点每个分量的有效值
            for (size_t i = 0; i < 3; i++) {
                range[i] = p1[i] < p2[i] ? Range(p1[i], p2[i]) : Range(p2[i], p1[i]);
            }
            ensureVolume();
        }

        //使用bounds数组构造包围盒
        explicit BoundingBox(const double bounds[6]) {
            for (size_t i = 0; i < 6; i += 2) {
                range[i / 2] = Range(bounds[i], bounds[i + 1]);
            }
        }

        //构造两个包围盒的合并
        BoundingBox(const BoundingBox & b1, const BoundingBox & b2) {
            for (size_t i = 0; i < 3; i++) {
                range[i] = Range(b1.range[i], b2.range[i]);
            }
            //合并不会减小包围盒的体积
        }

        bool hit(const Ray & ray, const Range & checkRange, double & t) const {
            const Point3 & rayOrigin = ray.origin;
            const Vec3 & rayDirection = ray.direction;

            Range currentRange(checkRange);
            for (Uint32 axis = 0; axis < 3; axis++) {
                const Range & axisRange = range[axis];
                const double q = rayOrigin[axis];
                const double d = rayDirection[axis];

                //光线和包围盒平行
                if (std::abs(d) < FLOAT_VALUE_ZERO_EPSILON) {
                    //光线起点不在包围盒内，没有交点
                    if (q < axisRange.min || q > axisRange.max) return false;
                    //光线从包围盒内部发出，继续测试下一个轴
                    continue;
                }

                //计算光在当前轴和边界的两个交点
                double t1 = (axisRange.min - q) / d;
                double t2 = (axisRange.max - q) / d;

                //将currentRange限制到这两个交点的范围内
                if (t1 < t2) {
                    if (t1 > currentRange.min) currentRange.min = t1;
                    if (t2 < currentRange.max) currentRange.max = t2;
                } else {
                    if (t2 > currentRange.min) currentRange.min = t2;
                    if (t1 < currentRange.max) currentRange.max = t1;
                }

                if (!currentRange.isValid()) {
                    return false;
                }
            }

            t = currentRange.min;
            return true;
        }
    };
}

#endif //RENDERERBUILD_BOUNDINGBOX_HPP
