#include <Render.hpp>
#include <box/BVHTree.hpp>

using namespace std;

namespace renderer {
    /*
     * 像素渲染函数：根据每个像素的光线对象和场景物体列表进行光线计算
     * 物体数量信息包含在BVH树的节点中，求交函数通过判断叶子节点终止递归
     */
    Color3 rayColor(const Color3 & background, const Ray & ray, Uint32 iterateDepth,
                    const BVHTree::BVHTreeNode * tree, const std::pair<PrimitiveType, size_t> * indexArray,
                    const Sphere * spheres,
                    const Triangle * triangles,
                    const Rough * roughMaterials,
                    const Metal * metalMaterials)
    {
        HitRecord record;
        Ray currentRay(ray);
        Color3 result(1.0, 1.0, 1.0);

        for (size_t currentIterateDepth = 0; currentIterateDepth < iterateDepth; currentIterateDepth++) {
            //每次追踪，都遍历所有球体，判断是否相交
            if (BVHTree::hit(tree, indexArray, spheres, triangles, currentRay, Range(0.001, INFINITY), record)) {
                //发生碰撞，调用材质的散射函数，获取下一次迭代的光线
                Ray out;
                Color3 attenuation;

                //根据材质类型调用对应的散射函数
                switch (record.materialType) {
                    default:
                    case MaterialType::ROUGH:
                        roughMaterials[record.materialIndex].scatter(currentRay, record, attenuation, out);
                        break;
                    case MaterialType::METAL:
                        metalMaterials[record.materialIndex].scatter(currentRay, record, attenuation, out);
                        break;
                }

                currentRay = out;
                result *= attenuation; //光线衰减系数
            } else {
                result *= background; //没有发生碰撞，将背景光颜色作为光源乘入结果并结束追踪循环
                break;
            }

            /*
            auto closestIndex = (size_t)-1;
            double closestT = INFINITY;
            for (size_t i = 0; i < spheres.size(); i++) {
                //取所有交点中最近的交点作为实际发生光线相交的球体
                if (spheres[i].hit(currentRay, Range(0.001, closestT), record)) {
                    closestT = record.t;
                    closestIndex = i;
                }
            }

            if (closestIndex != (size_t)-1) {
                //发生碰撞，调用材质的散射函数，获取下一次迭代的光线
                Ray out;
                Color3 attenuation;
                roughMaterials[spheres[closestIndex].materialIndex].scatter(currentRay, record, attenuation, out);

                currentRay = out;
                result *= attenuation; //光线衰减系数
            } else {
                result *= background; //没有发生碰撞，将背景光颜色作为光源乘入结果并结束追踪循环
                break;
            }
             */
        }
        return result;
    }

    /*
     * 光线构造函数：根据相机对象和线程下标构造光线
     */
    Ray constructRay(const Camera & cam, Uint32 threadX, Uint32 threadY) {
        //当前像素对应位置
        const Point3 samplePoint =
                cam.pixelOrigin + (threadY + randomDouble(-cam.sampleRange, cam.sampleRange)) * cam.viewPortPixelDy
                + (threadX + randomDouble(-cam.sampleRange, cam.sampleRange)) * cam.viewPortPixelDx;

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
    Uint32 render(const Camera & cam, Uint32 * pixels, const SDL_PixelFormat * format,
                  const Rough * roughMaterials,
                  const Metal * metalMaterials,
                  const Sphere * spheres, Uint32 sphereCount,
                  const Triangle * triangles, Uint32 triangleCount)
    {
        //构建BVH
        auto sphereVector = vector<Sphere>(spheres, spheres + sphereCount);
        auto triangleVector = vector<Triangle>(triangles, triangles + triangleCount);

        //先利用vector的返回值传递接收数组，再转换为指针
        const auto ret = BVHTree::constructBVHTree(sphereVector, triangleVector);
        //获取原始指针，用于在GPU函数间传递
        const BVHTree::BVHTreeNode * tree = ret.first.data();
        const std::pair<PrimitiveType, size_t> * indexArray = ret.second.data();

        //分配线程，GPU线程执行
        const Uint32 startTick = SDL_GetTicks();
        for (Uint32 i = 0; i < cam.windowHeight; i++) {
            for (Uint32 j = 0; j < cam.windowWidth; j++) {
                Color3 result;

                //抗锯齿采样
                for (size_t k = 0; k < cam.sampleCount; k++) {
                    //构造光线
                    const Ray ray = constructRay(cam, j, i);

                    //进行像素独立的计算
                    result += rayColor(cam.backgroundColor, ray, cam.rayTraceDepth, tree, indexArray, spheres, triangles, roughMaterials, metalMaterials);
                }
                result /= cam.sampleCount;

                //写入颜色
                result.writeColor(pixels + (i * cam.windowWidth + j), format);
            }
        }
        return SDL_GetTicks() - startTick;
    }
}