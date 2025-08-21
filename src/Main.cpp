#include <Render.hpp>

using namespace renderer;

namespace {
    constexpr Uint32 WINDOW_WIDTH = 800;
    constexpr Uint32 WINDOW_HEIGHT = 450;
    SDL_Window * window = nullptr;
    SDL_Surface * surface = nullptr;

    void initSDLResources();
    void releaseSDLResourcesImpl();
}

int main(int argc, char * argv[]) {
    initSDLResources();

//    const Camera cam(
//            WINDOW_WIDTH, WINDOW_HEIGHT, Color3(0.7, 0.8, 1.0),
//            Point3(0.0, 0.0, -5.0), Point3(0.0, 1.0, 0.0),
//            80, 0.0, Range(0.0, 1.0), 10,
//            0.5, 10, Vec3(0.0, 1.0, 0.0)
//    );
//
//    //材质列表
//    const Rough roughs[] = {
//            Rough(Color3(0.9, 0.5, 0.6)),
//            Rough(Color3(0.4, 0.8, 0.4))
//    };
//
//    const Metal metals[] = {
//            Metal(Color3(0.7, 0.6, 0.5), 0.0)
//    };
//
//    const DiffuseLight lights[] = {
//            DiffuseLight(Color3(15.0, 15.0, 15.0))
//    };
//
//    //物体列表
//    const Sphere spheres[] = {
//            Sphere(0, MaterialType::ROUGH, 0, Point3(0.0, -10.0, 0.0), 10.0),
//            Sphere(1, MaterialType::METAL, 0, Point3(0.0, 2.0, 0.0), 2.0)
//    };
//
//    const Triangle triangles[] = {
//            Triangle(0, MaterialType::ROUGH, 1, Point3(-1.0, 0.0, -2.0), Point3(1.0, 0.0, -2.0), Point3(0.0, 1.0, -2.0))
//    };
//
//    const Parallelogram parallelograms[] = {
//            Parallelogram(0, MaterialType::DIFFUSE_LIGHT, 0, Point3(-1.0, 0.0, 2.0), Vec3(2.0, 0.0, 0.0), Vec3(0.0, 2.0, 0.0))
//    };
//
//    const Transform transforms[] = {
//            Transform(PrimitiveType::PARALLELOGRAM, 0, parallelograms[0].constructBoundingBox(), parallelograms[0].centroid(),
//                      std::array<double, 3>{0.0, 30.0, 0.0}, std::array<double, 3>{0.0, 0.0, 0.0}, std::array<double, 3>{1.0, 1.5, 1.0})
//    };
//
//    SDL_Log("Render Start...");
//    SDL_Log("Render completed. Time: %u ms",
//            render(cam, pixels, surface->format, roughs, metals, lights,
//                   spheres, arrayLengthOnPos(spheres),
//                   triangles, arrayLengthOnPos(triangles),
//                   parallelograms, 0,
//                   transforms, 0)
//    );
//    SDL_UpdateWindowSurface(window);
//    SDL_Delay(1000 * 3);

    Camera cam(
            WINDOW_WIDTH, WINDOW_HEIGHT,
            //Color3(.75, .75, .75),
            Color3(),
            Point3(278, 278, -600), Point3(278, 278, 0),
            80, 0.0, Range(0.0, 1.0), 10,
            0.5, 10, Vec3(0.0, 1.0, 0.0)
    );

    //材质列表
    const Rough roughs[] = {
            Rough(Color3(.65, .05, .05)),    //红色
            Rough(Color3(.73, .73, .73)),    //白色
            Rough(Color3(.12, .45, .15))     //绿色
    };

    const Metal metals[] = {
            Metal(Color3(0.8, 0.85, 0.88), 0.0)
    };

    const DiffuseLight lights[] = {
            DiffuseLight(Color3(15.0, 15.0, 15.0))
    };

    const Dielectric dielectrics[] = {
            Dielectric(1.5)
    };

    //物体列表
    const Sphere spheres[] = {
            Sphere(0, MaterialType::DIELECTRIC, 0, Point3(190.0, 90.0, 190.0), 90.0)
    };

    //墙壁和灯光
    const auto light =  Parallelogram(5, MaterialType::DIFFUSE_LIGHT, 0, Point3(213.0, 554.0, 227.0), Vec3(130.0, 0.0, 0.0), Vec3(0.0, 0.0, 105.0));
    const Parallelogram parallelograms[] = {
            Parallelogram(0, MaterialType::ROUGH, 2, Point3(555.0, 0.0, 0.0), Vec3(0.0, 0.0, 555.0), Vec3(0.0, 555.0, 0.0)),
            Parallelogram(1, MaterialType::ROUGH, 0, Point3(0.0, 0.0, 555.0), Vec3(0.0, 0.0, -555.0), Vec3(0.0, 555.0, 0.0)),
            Parallelogram(2, MaterialType::ROUGH, 1, Point3(0.0, 555.0, 0.0), Vec3(555.0, 0.0, 0.0), Vec3(0.0, 0.0, 555.0)),
            Parallelogram(3, MaterialType::ROUGH, 1, Point3(0.0, 0.0, 555.0), Vec3(555.0, 0.0, 0.0), Vec3(0.0, 0.0, -555.0)),
            Parallelogram(4, MaterialType::ROUGH, 1, Point3(555.0, 0.0, 555.0), Vec3(-555.0, 0.0, 0.0), Vec3(0.0, 555.0, 0.0)),
            light
    };

    //直接重要性采样物体列表，引用已存在的图元对象，无需重新创建
    const Parallelogram hittableParallelogram[] = {
            light
    };
    const Sphere hittableSphere[] = {
            spheres[0]
    };

    //盒子
    const Box boxes[] = {
            Box(MaterialType::ROUGH, 1, Point3(), Point3(165.0, 165.0, 165.0)),
            Box(MaterialType::METAL, 0, Point3(), Point3(165.0, 330.0, 165.0))
    };

    //变换
    const Transform transforms[] = {
            //Transform(boxes, PrimitiveType::BOX, 0, boxes[0].constructBoundingBox(), boxes[0].centroid(), std::array<double, 3>{0.0, -15.0, 0.0}, std::array<double, 3>{130, 0.0, 65.0}/*, std::array<double, 3>{1.5, 1.5, 1.5}*/),
            Transform(boxes, PrimitiveType::BOX, 1, boxes[1].constructBoundingBox(), boxes[1].centroid(), std::array<double, 3>{0.0, 18.0, 0.0}, std::array<double, 3>{265.0, 0.0, 295.0}/*, std::array<double, 3>{1.5, 1.5, 1.5}*/)
    };

    SDL_Log("Render Start...");
    SDL_Log("Render completed. Time: %u ms",
            render(cam, window, surface,
                    roughs, metals,
                    lights, dielectrics,
                    //Sphere
                    spheres, arrayLengthOnPos(spheres),
                    //nullptr, 0,
                    //Triangle
                    nullptr, 0,
                    //Parallelogram
                    parallelograms, arrayLengthOnPos(parallelograms),
                    //Transform
                    //nullptr, 0,
                    transforms, arrayLengthOnPos(transforms),
                    //Box
                    //boxes, arrayLengthOnPos(boxes),
                    nullptr, 0,

                    //HittablePDF
                    hittableSphere, arrayLengthOnPos(hittableSphere),
                    hittableParallelogram, arrayLengthOnPos(hittableParallelogram))
    );

    SDL_UpdateWindowSurface(window);
    SDL_Delay(1000 * 3);

    //保存渲染结果
    IMG_SavePNG(surface, "../files/output.png");

    releaseSDLResourcesImpl();
    return 0;
}

namespace {
    void initSDLResources() {
        registerReleaseSDLResources(releaseSDLResourcesImpl);
        int ret;

        //初始化SDL库
        ret = SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_EVENTS);
        sdlCheckErrorInt(ret, "Init", EXIT_PROGRAM);
        window = SDL_CreateWindow("Test", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                  WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN);
        sdlCheckErrorPtr(window, "Create Window", EXIT_PROGRAM);
        surface = SDL_GetWindowSurface(window);
        sdlCheckErrorPtr(surface, "Get Surface", EXIT_PROGRAM);

        //初始化图像库
        ret = IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
        sdlCheckErrorInt(ret, "Init IMG", EXIT_PROGRAM);

        //打印SDL信息
        SDL_Log("SDL Version: %d.%d.%d", SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL);
        SDL_Log("SDL Color Format: %s", SDL_GetPixelFormatName(surface->format->format));
    }

    void releaseSDLResourcesImpl() {
        releaseSDLResource(IMG_Quit(), "Quit IMG");
        if (surface != nullptr) {
            releaseSDLResource(SDL_FreeSurface(surface), "Free Surface");
        }
        if (window != nullptr) {
            releaseSDLResource(SDL_DestroyWindow(window), "Destroy Window");
        }
        releaseSDLResource(SDL_Quit(), "Quit");
    }
}
