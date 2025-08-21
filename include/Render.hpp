#ifndef RENDERERBUILD_RENDER_HPP
#define RENDERERBUILD_RENDER_HPP

#include <Camera.hpp>
#include <hittable/Sphere.hpp>
#include <hittable/Triangle.hpp>
#include <hittable/Parallelogram.hpp>
#include <hittable/Transform.hpp>
#include <material/Rough.hpp>
#include <material/Metal.hpp>
#include <material/DiffuseLight.hpp>
#include <material/Dielectric.hpp>
#include <box/BVHTree.hpp>
#include <pdf/MixturePDF.hpp>

namespace renderer {
    Uint32 render(const Camera & cam, SDL_Window * window, SDL_Surface * surface,
                  const Rough * roughMaterials, const Metal * metalMaterials,
                  const DiffuseLight * lightMaterials, const Dielectric * dielectricMaterials,
                  const Sphere * spheres, Uint32 sphereCount,
                  const Triangle * triangles, Uint32 triangleCount,
                  const Parallelogram * parallelograms, Uint32 parallelogramCount,
                  const Transform * transforms, Uint32 transformCount,
                  const Box * boxes, Uint32 boxCount,
                  const Sphere * hittablePDFSphere, size_t hittablePDFSphereCount,
                  const Parallelogram * hittablePDFParallelogram, size_t hittablePDFParallelogramCount);
}

#endif //RENDERERBUILD_RENDER_HPP
