#include <Render.hpp>

using namespace renderer;

namespace {
    constexpr Uint32 WINDOW_WIDTH = 800;
    constexpr Uint32 WINDOW_HEIGHT = 450;
    SDL_Window * window = null;
    SDL_Surface * surface = null;
    Uint32 * pixels = null;

    void initSDLResources();
    void releaseSDLResourcesImpl();
}

int main(int argc, char * argv[]) {
    initSDLResources();

    const Camera cam(
            WINDOW_WIDTH, WINDOW_HEIGHT, Color3(0.7, 0.8, 1.0),
            Point3(0.0, 0.0, 5.0), Point3(),
            80, 0.0, Range(0.0, 1.0), 10,
            0.5, 10, Vec3(0.0, 1.0, 0.0)
    );

    //材质列表
    const Rough roughs[] = {
            Rough(Color3(0.9, 0.5, 0.6)),
            Rough(Color3(0.4, 0.8, 0.4))
    };

    const Metal metals[] = {
            Metal(Color3(0.7, 0.6, 0.5), 0.0)
    };

    //物体列表
    const Sphere spheres[] = {
            Sphere(0, MaterialType::ROUGH, 0, Point3(0.0, -10.0, 0.0), 10.0),
            Sphere(1, MaterialType::METAL, 0, Point3(0.0, 2.0, 0.0), 2.0)
    };

    const Triangle triangles[] = {
            Triangle(0, MaterialType::ROUGH, 1, Point3(-1.0, 0.0, 2.0), Point3(1.0, 0.0, 2.0), Point3(0.0, 1.0, 2.0))
    };

    SDL_Log("Render Start...");
    SDL_Log("Render completed. Time: %u ms",
            render(cam, pixels, surface->format, roughs, metals, spheres, 2, triangles, 1)
    );
    SDL_UpdateWindowSurface(window);
    SDL_Delay(1000 * 3);

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
        pixels = static_cast<Uint32 *>(surface->pixels);
        sdlCheckErrorPtr(pixels, "Cast Pixel Pointer", EXIT_PROGRAM);

        //初始化图像库
        ret = IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG);
        sdlCheckErrorInt(ret, "Init IMG", EXIT_PROGRAM);

        //打印SDL信息
        SDL_Log("SDL Version: %d.%d.%d", SDL_MAJOR_VERSION, SDL_MINOR_VERSION, SDL_PATCHLEVEL);
        SDL_Log("SDL Color Format: %s", SDL_GetPixelFormatName(surface->format->format));
    }

    void releaseSDLResourcesImpl() {
        releaseSDLResource(IMG_Quit(), "Quit IMG");
        if (surface != null) {
            releaseSDLResource(SDL_FreeSurface(surface), "Free Surface");
        }
        if (window != null) {
            releaseSDLResource(SDL_DestroyWindow(window), "Destroy Window");
        }
        releaseSDLResource(SDL_Quit(), "Quit");
    }
}
