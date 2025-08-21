#ifndef RENDERERTEST_ROUGH_HPP
#define RENDERERTEST_ROUGH_HPP

#include <basic/Ray.hpp>
#include <basic/Color3.hpp>

namespace renderer {
    /*
     * 粗糙材质类
     */
    class Rough {
    private:
        Color3 albedo;

    public:
        explicit Rough(const Color3 & albedo) : albedo(albedo) {}

        /*
         * 粗糙材质：漫反射
         * 此材质的BRDF为rho / π，rho为材质的albedo，则BRDF为一个常数
         * 渲染方程中的余弦项为法向量和出射方向向量的夹角
         */
//        bool scatter(const Ray & in, const HitRecord & record, Color3 & attenuation, Ray & out) const {
//            //使用正交基，以record.normalVector作为z轴
//            //遵守余弦概率密度
//            const OrthonormalBase base(record.normalVector, 2);
//            Vec3 randomVec = Vec3::randomCosineVector(2, true);
//
//            //将randomVec从局部坐标系转换到世界坐标系，randomVec为随机选择的反射方向
//            randomVec = base.transform(randomVec);
//
//            //从反射点出发构造反射光线
//            out = Ray(record.hitPoint, randomVec.unitVector(), in.time);
//            attenuation = albedo;
//            return true;
//        }
//
//        double scatterPDF(const Ray & in, const HitRecord & record, const Ray & out) const {
//            return std::max(0.0, Vec3::dot(record.normalVector, out.direction.unitVector()) / PI);
//        }

        /*
         * BRDF是一个函数，它能根据入射光线的方向，摄像机的方向和法线方向计算出特定方向上的反射光强度
         * Bidirectional Reflectance Distribution Function
         */
        Color3 evalBRDF(const Ray & in, const HitRecord & record) const {
            return albedo / PI;
        }

        //计算渲染方程中的余弦项
        double cosTheta(const Ray & out, const HitRecord & record) const {
            return std::max(0.0, Vec3::dot(record.normalVector, out.direction.unitVector()));
            //return Vec3::dot(record.normalVector, out.direction.unitVector());
        }
    };
}

#endif //RENDERERTEST_ROUGH_HPP
