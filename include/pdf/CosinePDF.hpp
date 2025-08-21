#ifndef RENDERERBUILD_COSINEPDF_HPP
#define RENDERERBUILD_COSINEPDF_HPP

#include <util/OrthonormalBase.hpp>

namespace renderer {
    /*
     * 余弦概率密度，离法向量近的方向概率密度高
     */
    class CosinePDF {
    private:
        //由法向量构造局部坐标系
        OrthonormalBase base;

    public:
        explicit CosinePDF(const Vec3 & normal) : base(normal, 2) {}

        ~CosinePDF() = default;

        Vec3 generate() const {
            //将生成的局部空间向量（randomCosineVector）变换到世界空间
            return base.transform(Vec3::randomCosineVector(2, true));
        }

        double value(const Vec3 &vec) const {
            //保证概率密度不为负
            return std::max(0.0, Vec3::dot(vec.unitVector(), base[2]) / PI);
        }

        std::string toString() const {
            std::string ret("Cosine PDF: Base = ");
            return ret + base.toString();
        }
    };
}

#endif //RENDERERBUILD_COSINEPDF_HPP
