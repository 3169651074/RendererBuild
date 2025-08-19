#ifndef RENDERERBUILD_RENDER_HPP
#define RENDERERBUILD_RENDER_HPP

#include <Camera.hpp>
#include <hittable/Sphere.hpp>
#include <hittable/Triangle.hpp>
#include <material/Rough.hpp>
#include <material/Metal.hpp>

namespace renderer {
    //sphere的下标和material的下标对应
    Uint32 render(const Camera & cam, Uint32 * pixels, const SDL_PixelFormat * format,
                  const Rough * roughMaterials,
                  const Metal * metalMaterials,
                  const Sphere * spheres, Uint32 sphereCount,
                  const Triangle * triangles, Uint32 triangleCount);

    Ray constructRay(const Camera & cam, Uint32 threadX, Uint32 threadY);

    Color3 rayColor(const Ray & ray, Uint32 iterateDepth,
                    const Sphere * spheres, Uint32 sphereCount);
}

#endif //RENDERERBUILD_RENDER_HPP
