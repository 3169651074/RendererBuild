#ifndef RENDERERBUILD_ORTHONORMALBASE_HPP
#define RENDERERBUILD_ORTHONORMALBASE_HPP

#include <basic/Vec3.hpp>

namespace renderer {
    class OrthonormalBase {
    private:
        Vec3 elements[3];

    public:
        //根据向量构造正交基，不要求传入单位向量，可以指定传入的向量作为局部坐标系的轴的下标
        OrthonormalBase(const Vec3 & vec, int axis) {
            if (axis < 0 || axis > 2) {
                throw std::runtime_error("Axis out of bound!");
            }

            //将vec的单位向量作为局部坐标系的z轴（axis = 2）
            elements[axis] = vec.unitVector();

            //使用坐标轴构造不平行于vec的“任意”向量a
            Vec3 a;
            if (elements[axis][0] > 0.9) {
                //传入向量和x轴平行，则将y轴作为a
                a[1] = 1.0;
            } else {
                //将x轴作为a
                a[0] = 1.0;
            }

            if (axis == 0 || axis == 2) {
                elements[1] = Vec3::cross(elements[axis], a).unitVector();
                elements[2 - axis] = Vec3::cross(elements[axis], elements[1]);
            } else { //axis == 1
                elements[2] = Vec3::cross(elements[axis], a).unitVector();
                elements[0] = Vec3::cross(elements[axis], elements[1]);
            }
        }

        ~OrthonormalBase() = default;

        //将局部空间中的向量origin变换到世界空间
        Vec3 transform(const Vec3 & origin) const {
            Vec3 ret;
            for (size_t i = 0; i < 3; i++) {
                //将origin的各轴向分量（标量值）和轴向量（单位方向向量）数乘
                ret += elements[i] * origin[i];
            }
            return ret;
        }

        Vec3 operator[](size_t index) const { return elements[index]; }
        Vec3& operator[](size_t index) { return elements[index]; }

        std::string toString() const {
            std::string ret("OrthonormalBase: ");
            for (size_t i = 0; i < 3; i++) {
                ret += "\n\t";
                ret += static_cast<char>('x' + i);
                ret += ": ";
                ret += elements[i].toString();
            }
            return ret;
        }
    };
}

#endif //RENDERERBUILD_ORTHONORMALBASE_HPP
