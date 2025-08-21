#include <Render.hpp>

using namespace std;

namespace renderer {
    /*
     * 像素渲染函数：根据每个像素的光线对象和场景物体列表进行光线计算
     * 物体数量信息包含在BVH树的节点中，求交函数通过判断叶子节点终止递归
     */
    Color3 rayColor(Camera & cam, const Ray & ray, size_t sampleIndex,
                    const BVHTree::BVHTreeNode * tree, const std::pair<PrimitiveType, size_t> * indexArray,
                    const Sphere * spheres,
                    const Triangle * triangles,
                    const Parallelogram * parallelograms,
                    const Transform * transforms,
                    const Box * boxes,
                    const Rough * roughMaterials, const Metal * metalMaterials,
                    const DiffuseLight * lightMaterials, const Dielectric * dielectricMaterials,
                    const Sphere * hittablePDFSphere, size_t hittablePDFSphereCount,
                    const Parallelogram * hittablePDFParallelogram, size_t hittablePDFParallelogramCount)
    {
        HitRecord record;
        Ray currentRay(ray);
        Color3 result(1.0, 1.0, 1.0);

        for (size_t currentIterateDepth = 0; currentIterateDepth < cam.rayTraceDepth; currentIterateDepth++) {
            if (BVHTree::hit(tree, indexArray, spheres, triangles, parallelograms, transforms, boxes,
                             currentRay, Range(0.001, INFINITY), record)) {
                Ray out;
                Color3 attenuation;

                //光源
                if (record.materialType == MaterialType::DIFFUSE_LIGHT) {
                    //光源是光路的终点，需要综合之前的颜色，并结束光路
                    return result * lightMaterials[record.materialIndex].emitted(currentRay, record);
                }

                //非光源，根据材质类型调用对应的散射函数
                switch (record.materialType) {
                    case MaterialType::ROUGH: {
                        //CosinePDF用于材质表面采样
                        const CosinePDF cosinePDF[] = {
                                CosinePDF(record.normalVector)
                        };

                        //将HittablePDF和CosinePDF组合进MixturePDF
                        HittablePDF hittablePDF[32] {};
                        size_t hittablePDFCount = 0;
                        for (size_t i = 0; i < hittablePDFSphereCount; i++) {
                            hittablePDF[hittablePDFCount++] = HittablePDF(PrimitiveType::SPHERE, i, record.hitPoint);
                        }
                        for (size_t i = 0; i < hittablePDFParallelogramCount; i++) {
                            hittablePDF[hittablePDFCount++] = HittablePDF(PrimitiveType::PARALLELOGRAM, i, record.hitPoint);
                        }

                        const MixturePDF pdf(cosinePDF, hittablePDF,
                                             1, hittablePDFSphereCount + hittablePDFParallelogramCount);

                        //使用MixturePDF生成一个新的光线方向
                        out = Ray(record.hitPoint, pdf.generate(hittablePDFSphere, hittablePDFParallelogram), ray.time);
                        const double pdfValue = pdf.value(hittablePDFSphere, hittablePDFParallelogram, out.direction);

                        //pdfValue有效性检查
                        if (isnan(pdfValue) || isinf(pdfValue) || floatValueNearZero(pdfValue)) {
                            return Color3(); //此处return result会使得画面严重偏白，PDF无效时，整条光路结果为黑色
                        }

                        const Color3 BRDFvalue = roughMaterials[record.materialIndex].evalBRDF(currentRay, record);
                        const double cosTheta = roughMaterials[record.materialIndex].cosTheta(out, record);
                        result *= BRDFvalue * cosTheta / pdfValue;

                        attenuation = BRDFvalue * PI;
                        currentRay = out;
                        break;
                    }
                    case MaterialType::METAL: {
                        if (metalMaterials[record.materialIndex].scatter(currentRay, record, attenuation, out)) {
                            result *= attenuation;
                            currentRay = out;
                        } else {
                            return result;
                        }
                        break;
                    }
                    case MaterialType::DIELECTRIC: {
                        dielectricMaterials[record.materialIndex].scatter(currentRay, record, attenuation, out);
                        result *= attenuation;
                        currentRay = out;
                        break;
                    }
                    default:;
                }

                //记录降噪器信息
                if (!cam.isRecordList[sampleIndex]) {
                    cam.albedoList[sampleIndex] = attenuation;
                    cam.normalList[sampleIndex] = cam.base.transformToLocal(record.normalVector);
                    cam.isRecordList[sampleIndex] = true;
                }
            } else {
                result *= cam.backgroundColor; //没有发生碰撞，将背景光颜色作为光源乘入结果并结束追踪循环
                break;
            }
        }
        return result;
    }

    /*
     * 光线构造函数：根据相机对象和线程下标构造光线
     */
    Ray constructRay(const Camera & cam, const Point3 & samplePoint) {
        //离焦采样：在离焦半径内随机选取一个点，以这个点发射光线
        Point3 rayOrigin = cam.cameraCenter;
        if (cam.focusDiskRadius > 0.0) {
            const Vec3 defocusVector = Vec3::randomPlaneVector(cam.focusDiskRadius);
            //使用视口方向向量定位采样点
            rayOrigin = cam.cameraCenter + defocusVector[0] * cam.cameraU + defocusVector[1] * cam.cameraV;
        }

        //在快门开启时段内随机找一个时刻发射光线
        const Vec3 rayDirection = Point3::constructVector(rayOrigin, samplePoint).unitVector();
        return Ray(rayOrigin, rayDirection, randomDouble(cam.shutterRange.min, cam.shutterRange.max));
    }

    /*
     * 主渲染函数
     *
     * 需要传入场景信息：相机对象和物体列表，窗口信息
     *     以及输出信息：用于写入颜色数据的指针
     */
    Uint32 render(Camera & cam, SDL_Window * window, SDL_Surface * surface,
                  const Rough * roughMaterials, const Metal * metalMaterials,
                  const DiffuseLight * lightMaterials, const Dielectric * dielectricMaterials,
                  const Sphere * spheres, Uint32 sphereCount,
                  const Triangle * triangles, Uint32 triangleCount,
                  const Parallelogram * parallelograms, Uint32 parallelogramCount,
                  const Transform * transforms, Uint32 transformCount,
                  const Box * boxes, Uint32 boxCount,
                  const Sphere * hittablePDFSphere, size_t hittablePDFSphereCount,
                  const Parallelogram * hittablePDFParallelogram, size_t hittablePDFParallelogramCount)
    {

        auto pixels = static_cast<Uint32 *>(surface->pixels);
        auto format = surface->format;

        //构建BVH
        auto sphereVector = vector<Sphere>(spheres, spheres + sphereCount);
        auto triangleVector = vector<Triangle>(triangles, triangles + triangleCount);
        auto parallelogramVector = vector<Parallelogram>(parallelograms, parallelograms + parallelogramCount);
        auto transformVector = vector<Transform>(transforms, transforms + transformCount);
        auto boxVector = vector<Box>(boxes, boxes + boxCount);

        //先利用vector的返回值传递接收数组，再转换为指针
        //在此处构造包含所有物体的HittableList的BVH
        const auto ret = BVHTree::constructBVHTree(sphereVector, triangleVector, parallelogramVector, transformVector, boxVector);

        //获取原始指针，用于在GPU函数间传递
        const BVHTree::BVHTreeNode * tree = ret.first.data();
        const std::pair<PrimitiveType, size_t> * indexArray = ret.second.data();

#define REFRESH_ON_RENDER
#ifdef REFRESH_ON_RENDER
        const Uint32 refreshRate = 1;
        Uint32 lastRate = 0;
#endif

        //分配线程，由GPU线程执行主渲染逻辑
        const Uint32 startTick = SDL_GetTicks();
        for (Uint32 i = 0; i < cam.windowHeight; i++) {
            for (Uint32 j = 0; j < cam.windowWidth; j++) {
                SDL_Event event;
                while (SDL_PollEvent(&event)) {
                    if (event.type == SDL_QUIT) { exit(1); }
                }

                Color3 result;
                Color3 albedo;
                Vec3 normal;
                fill(cam.isRecordList.begin(), cam.isRecordList.end(), false);

                //抗锯齿采样
#define JITTERING
#ifndef JITTERING
                for (size_t k = 0; k < cam.sampleCount; k++) {
                    //构造光线
                    const Point3 samplePoint =
                            cam.pixelOrigin + (i + randomDouble(-cam.sampleRange, cam.sampleRange)) * cam.viewPortPixelDy
                            + (j + randomDouble(-cam.sampleRange, cam.sampleRange)) * cam.viewPortPixelDx;
                    const Ray ray = constructRay(cam, samplePoint);

                    //进行像素独立的计算
                    result += rayColor(cam.backgroundColor, ray, cam.rayTraceDepth, tree, indexArray,
                                       spheres, triangles, parallelograms, transforms,
                                       roughMaterials, metalMaterials, lightMaterials);
                }
#else

                for (size_t sampleI = 0; sampleI < cam.sqrtSampleCount; sampleI++) {
                    for (size_t sampleJ = 0; sampleJ < cam.sqrtSampleCount; sampleJ++) {
                        const double offsetX = ((sampleJ + randomDouble()) * cam.reciprocalSqrtSampleCount) - 0.5;
                        const double offsetY = ((sampleI + randomDouble()) * cam.reciprocalSqrtSampleCount) - 0.5;
                        const Point3 samplePoint =
                                cam.pixelOrigin + ((j + offsetX) * cam.viewPortPixelDx) + ((i + offsetY) * cam.viewPortPixelDy);

                        //构造光线
                        const Ray ray = constructRay(cam, samplePoint);

                        //发射光线
                        const size_t sampleIndex = sampleI * cam.sqrtSampleCount + sampleJ;
                        result += rayColor(cam, ray, sampleIndex, tree, indexArray,
                                           spheres, triangles, parallelograms, transforms, boxes,
                                           roughMaterials, metalMaterials, lightMaterials, dielectricMaterials,
                                           hittablePDFSphere, hittablePDFSphereCount,
                                           hittablePDFParallelogram, hittablePDFParallelogramCount);

                        //累加当前采样点的降噪数据
                        albedo += cam.albedoList[sampleIndex];
                        normal += cam.normalList[sampleIndex];
                    }
                }
#endif
                result *= cam.reciprocalSqrtSampleCount * cam.reciprocalSqrtSampleCount;

                //写入颜色
                result.writeColor(pixels + (i * cam.windowWidth + j), format);

                //将降噪数据写入全局缓冲区
                albedo *= cam.reciprocalSqrtSampleCount * cam.reciprocalSqrtSampleCount;
                normal.unitize();

                const size_t pixelIndex = (i * cam.windowWidth + j) * 3;
                for (int k = 0; k < 3; k++) {
                    cam.denoiser.colorPtr[pixelIndex + k] = static_cast<float>(result[k]);
                    cam.denoiser.albedoPtr[pixelIndex + k] = static_cast<float>(albedo[k]);
                    cam.denoiser.normalPtr[pixelIndex + k] = static_cast<float>(normal[k]);
                }
            }

#ifdef REFRESH_ON_RENDER
            const auto rate = static_cast<Uint32>(i * 100 / cam.windowHeight);
            if (rate / refreshRate != lastRate) {
                lastRate = rate / refreshRate;
                SDL_Log("Rendered %u%%", rate);
                SDL_UpdateWindowSurface(window);
            }
#endif
        }

        //降噪并显示
        cam.denoiser.denoiseAndWrite(pixels, format);
        return SDL_GetTicks() - startTick;
    }
}