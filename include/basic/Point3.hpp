#ifndef POINT3_HPP
#define POINT3_HPP

#include <basic/Vec3.hpp>

namespace renderer {
    /*
     * 空间点类
     */
    class Point3 {
    private:
        double elements[3] {};

    public:
        explicit Point3(double x = 0.0, double y = 0.0, double z = 0.0) {
            elements[0] = x; elements[1] = y; elements[2] = z;
        }
        explicit Point3(const Vec3 & obj) {
            for (size_t i = 0; i < 3; i++) { elements[i] = obj[i]; }
        }

        double & operator[](size_t index) {
            return elements[index];
        }
        double operator[](size_t index) const {
            return elements[index];
        }

        // ====== 对象操作函数 ======

        double distanceSquare(const Point3 & anotherPoint) const {
            double sum = 0.0;
            for (size_t i = 0; i < 3; i++) {
                sum += (elements[i] - anotherPoint.elements[i]) * (elements[i] - anotherPoint.elements[i]);
            }
            return sum;
        }

        double distance(const Point3 & anotherPoint) const {
            return std::sqrt(distanceSquare(anotherPoint));
        }

        Point3 & operator+=(const Vec3 & offset) {
            for (size_t i = 0; i < 3; i++) {
                elements[i] += offset[i];
            }
            return *this;
        }

        Point3 operator+(const Vec3 & offset) const {
            Point3 ret(*this); ret += offset; return ret;
        }

        Point3 & operator-=(const Vec3 & offset) {
            for (size_t i = 0; i < 3; i++) {
                elements[i] -= offset[i];
            }
            return *this;
        }

        Point3 operator-(const Vec3 & offset) const {
            Point3 ret(*this); ret -= offset; return ret;
        }

        Vec3 toVector() const  {
            return Vec3(elements[0], elements[1], elements[2]);
        }

        // ====== 静态操作函数 ======

        static inline double distanceSquare(const Point3 & p1, const Point3 & p2) {
            return p1.distanceSquare(p2);
        }

        static inline double distance(const Point3 & p1, const Point3 & p2) {
            return std::sqrt(p1.distanceSquare(p2));
        }

        static inline Vec3 constructVector(const Point3 & from, const Point3 & to) {
            Vec3 ret;
            for (size_t i = 0; i < 3; i++) {
                ret[i] = to.elements[i] - from.elements[i];
            }
            return ret;
        }

        // ====== 类封装函数 ======

        std::string toString() const {
            char buffer[TOSTRING_BUFFER_SIZE] = { 0 };
            snprintf(buffer, TOSTRING_BUFFER_SIZE, "Point3: (%.4lf, %.4lf, %.4lf)", elements[0], elements[1], elements[2]);
            return {buffer};
        }
    };
}

#endif //POINT3_HPP
