#ifndef RENDERERTEST_VEC3_HPP
#define RENDERERTEST_VEC3_HPP

#include <Global.hpp>

namespace renderer {
    /*
     * 三维向量类
     * class作为一种数据组织方式可以上传到GPU，只要类型是POD类型：
     *     没有虚函数
     *     不包含指向外部资源的指针
     *     成员函数中不操作外部资源
     *     对象可以安全地逐字节拷贝
     *
     * 拷贝构造，赋值重载和析构函数均使用编译器默认生成
     */
    class Vec3 {
    private:
        double elements[3] {};

    public:
        static constexpr double VECTOR_LENGTH_SQUARE_ZERO_EPSILON = FLOAT_VALUE_ZERO_EPSILON * FLOAT_VALUE_ZERO_EPSILON;

        explicit Vec3(double x = 0.0, double y = 0.0, double z = 0.0) {
            elements[0] = x; elements[1] = y; elements[2] = z;
        }

        double & operator[](size_t index) {
            return elements[index];
        }
        double operator[](size_t index) const {
            return elements[index];
        }

        // ====== 对象操作函数 ======

        Vec3 operator-() const {
            return Vec3(-elements[0], -elements[1], -elements[2]);
        }

        Vec3 & negate() {
            for (double & element : elements) {
                element = -element;
            }
            return *this;
        }

        Vec3 & operator+=(const Vec3 & obj) {
            for (size_t i = 0; i < 3; i++) {
                elements[i] += obj.elements[i];
            }
            return *this;
        }

        Vec3 operator+(const Vec3 & obj) const {
            Vec3 ret(*this); ret += obj; return ret;
        }

        Vec3 & operator-=(const Vec3 & obj) {
            for (size_t i = 0; i < 3; i++) {
                elements[i] -= obj.elements[i];
            }
            return *this;
        }

        Vec3 operator-(const Vec3 & obj) const {
            Vec3 ret(*this); ret -= obj; return ret;
        }

        Vec3 & operator*=(double num) {
            for (double & element : elements) {
                element *= num;
            }
            return *this;
        }

        Vec3 operator*(double num) const {
            Vec3 ret(*this); ret *= num; return ret;
        }

        Vec3 & operator/=(double num) {
            for (double & element : elements) {
                element /= num;
            }
            return *this;
        }

        Vec3 operator/(double num) const {
            Vec3 ret(*this); ret /= num; return ret;
        }

        //数乘除操作允许左操作数为实数

        friend Vec3 & operator*=(double num, Vec3 & obj) {
            return obj *= num;
        }

        friend Vec3 & operator/=(double num, Vec3 & obj) {
            return obj /= num;
        }

        friend Vec3 operator*(double num, const Vec3 & obj) {
            Vec3 ret(obj); ret *= num; return ret;
        }

        friend Vec3 operator/(double num, const Vec3 & obj) {
            Vec3 ret(obj); ret /= num; return ret;
        }

        double lengthSquare() const {
            double sum = 0.0;
            for (const double & element : elements) {
                sum += element * element;
            }
            return sum;
        }

        double length() const {
            return std::sqrt(lengthSquare());
        }

        double dot(const Vec3 & obj) const {
            double sum = 0.0;
            for (size_t i = 0; i < 3; i++) {
                sum += elements[i] * obj.elements[i];
            }
            return sum;
        }

        Vec3 cross(const Vec3 & obj) const {
            return Vec3(elements[1] * obj.elements[2] - elements[2] * obj.elements[1],
                        elements[2] * obj.elements[0] - elements[0] * obj.elements[2],
                        elements[0] * obj.elements[1] - elements[1] * obj.elements[0]);
        }

        Vec3 & unitize() {
            const double len = length();
            for (double & element : elements) {
                element /= len;
            }
            return *this;
        }

        Vec3 unitVector() const {
            Vec3 ret(*this); ret.unitize(); return ret;
        }

        // ====== 静态操作函数 ======

        //生成遵守按指定轴余弦分布的随机向量，非单位向量
        static inline Vec3 randomCosineVector(int axis, bool toPositive) {
            double coord[3];
            const auto r1 = randomDouble();
            const auto r2 = randomDouble();

            coord[0] = cos(2.0 * PI * r1) * 2.0 * sqrt(r2);
            coord[1] = sin(2.0 * PI * r1) * 2.0 * sqrt(r2);
            coord[2] = sqrt(1.0 - r2);

            switch (axis) {
                case 0:
                    std::swap(coord[0], coord[2]);
                    break;
                case 1:
                    std::swap(coord[1], coord[2]);
                    break;
                case 2:
                default:
                    break;
            }

            if (!toPositive) {
                coord[axis] = -coord[axis];
            }
            return Vec3(coord[0], coord[1], coord[2]);
        }

        //生成每个分量都在指定范围内的随机向量
        static inline Vec3 randomVector(double componentMin, double componentMax) {
            return Vec3(randomDouble(componentMin, componentMax), randomDouble(componentMin, componentMax), randomDouble(componentMin, componentMax));
        }

        //生成平面（x，y，0）上模长不大于maxLength的向量
        static inline Vec3 randomPlaneVector(double maxLength) {
            double x, y;
            do {
                x = randomDouble(-1.0, 1.0);
                y = randomDouble(-1.0, 1.0);
            } while (x * x + y * y > maxLength * maxLength);
            return Vec3(x, y, 0.0);
        }

        //生成模长为length的空间向量
        static inline Vec3 randomSpaceVector(double length) {
            Vec3 ret;
            double lengthSquare;
            //先生成单位向量，再缩放到指定模长
            do {
                for (size_t i = 0; i < 3; i++) {
                    ret[i] = randomDouble(-1.0, 1.0);
                }
                lengthSquare = ret.lengthSquare();
            } while (lengthSquare < VECTOR_LENGTH_SQUARE_ZERO_EPSILON);

            //单位化ret，确定模长
            ret.unitize();
            return ret * length;
        }

        static inline Vec3 negativeVector(const Vec3 & obj) {
            return -obj;
        }

        static inline Vec3 add(const Vec3 & v1, const Vec3 & v2) {
            return v1 + v2;
        }

        static inline Vec3 subtract(const Vec3 & origin, const Vec3 & sub) {
            return origin - sub;
        }

        static inline Vec3 multiply(const Vec3 & obj, double num) {
            return obj * num;
        }

        static inline Vec3 divide(const Vec3 & origin, double num) {
            return origin / num;
        }

        static inline double lengthSquare(const Vec3 & obj) {
            return obj.lengthSquare();
        }

        static inline double length(const Vec3 & obj) {
            return obj.length();
        }

        static inline double dot(const Vec3 & v1, const Vec3 & v2) {
            return v1.dot(v2);
        }

        //v1 x v2
        static inline Vec3 cross(const Vec3 & v1, const Vec3 & v2) {
            return v1.cross(v2);
        }

        static inline Vec3 unitVector(const Vec3 & obj) {
            return obj.unitVector();
        }

        // ====== 类封装函数 ======

        /*
         * 设备端不支持std::string，此函数只能被主机端调用
         */
        std::string toString() const {
            char buffer[TOSTRING_BUFFER_SIZE] = { 0 };
            snprintf(buffer, TOSTRING_BUFFER_SIZE, "Vec3: (%.4lf, %.4lf, %.4lf)", elements[0], elements[1], elements[2]);
            return {buffer};
        }
    };
}

#endif //RENDERERTEST_VEC3_HPP