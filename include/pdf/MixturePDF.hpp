#ifndef RENDERERBUILD_MIXTUREPDF_HPP
#define RENDERERBUILD_MIXTUREPDF_HPP

#include <pdf/CosinePDF.hpp>
#include <pdf/HittablePDF.hpp>

namespace renderer {
    class MixturePDF {
    private:
        struct PDFTypeInfo {
            PDFType type;
            size_t index;

            PDFTypeInfo(PDFType type = PDFType::MIXTURE, size_t index = 0) : type(type), index(index) {}
        };

        //PDF列表，前半部分为余弦PDF，后半部分为物体PDF
        PDFTypeInfo infoArray[32];
        size_t pdfCount;

        //将需要混合的PDF数组作为成员指针
        const CosinePDF * cosinePDFs;
        const HittablePDF * hittablePDFs;

    public:
        MixturePDF(const CosinePDF * cosinePDFs, const HittablePDF * hittablePDFs,
                   Uint32 cosinePDFCount, Uint32 hittablePDFCount) :
                   cosinePDFs(cosinePDFs), hittablePDFs(hittablePDFs), pdfCount(0) { //必须初始化pdfCount，否则为随机值
            for (size_t i = 0; i < hittablePDFCount; i++) {
                infoArray[pdfCount++] = PDFTypeInfo(PDFType::HITTABLE, i);
            }
            for (size_t i = 0; i < cosinePDFCount; i++) {
                infoArray[pdfCount++] = PDFTypeInfo(PDFType::COSINE, i);
            }
        }

        //当前支持球体和平行四边形作为采样物体
        Vec3 generate(const Sphere * spheres, const Parallelogram * parallelograms) const {
            //从PDF列表中随机选择一个
            const int randomIndex = randomInt(0, static_cast<int>(pdfCount) - 1);
            switch (infoArray[randomIndex].type) {
                case PDFType::COSINE:
                    return cosinePDFs[infoArray[randomIndex].index].generate();
                case PDFType::HITTABLE:
                    return hittablePDFs[infoArray[randomIndex].index].generate(spheres, parallelograms);
                default:
                    return Vec3();
            }
        }

        double value(const Sphere * spheres, const Parallelogram * parallelograms, const Vec3 &vec) const {
            //求所有PDF的加权平均值
            const double weight = 1.0 / static_cast<int>(pdfCount);

            double sum = 0.0;
            for (size_t i = 0; i < pdfCount; i++) {
                switch (infoArray[i].type) {
                    case PDFType::COSINE:
                        sum += weight * cosinePDFs[infoArray[i].index].value(vec);
                        break;
                    case PDFType::HITTABLE:
                        sum += weight * hittablePDFs[infoArray[i].index].value(spheres, parallelograms, vec);
                        break;
                    default:
                        return 0.0;
                }
            }
            return sum;
        }
    };
}

#endif //RENDERERBUILD_MIXTUREPDF_HPP
