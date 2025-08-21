#ifndef RENDERERBUILD_DENOISER_HPP
#define RENDERERBUILD_DENOISER_HPP

#include <basic/Color3.hpp>
#include <OpenImageDenoise/oidn.hpp>

namespace renderer {
    class Denoiser {
    private:
        Uint32 windowWidth, windowHeight;

        //降噪器成员变量
        oidn::DeviceRef device;

        //缓冲区数组
        oidn::BufferRef colorBuffer;  //输出颜色
        oidn::BufferRef normalBuffer; //辅助信息：表面法线
        oidn::BufferRef albedoBuffer; //辅助信息：衰减颜色

    public:
        //缓冲区数组原始指针
        float * colorPtr;
        float * normalPtr;
        float * albedoPtr;

        Denoiser(Uint32 windowWidth, Uint32 windowHeight) : windowWidth(windowWidth), windowHeight(windowHeight) {
            device = oidn::newDevice();
            device.commit();

            const size_t bufferSize = (size_t)windowWidth * windowHeight * 3 * sizeof(float);
            colorBuffer = device.newBuffer(bufferSize);
            normalBuffer = device.newBuffer(bufferSize);
            albedoBuffer = device.newBuffer(bufferSize);

            colorPtr = static_cast<float*>(colorBuffer.getData());
            normalPtr = static_cast<float*>(normalBuffer.getData());
            albedoPtr = static_cast<float*>(albedoBuffer.getData());

            if (colorPtr == nullptr || normalPtr == nullptr || albedoPtr == nullptr) {
                throw std::runtime_error("OIDN buffer mapping failed");
            }
        }
        ~Denoiser() = default;

        //降噪并将结果写入到颜色内存
        void denoiseAndWrite(Uint32 * pixels, const SDL_PixelFormat * format) {
            SDL_Log("Denoising...");
            oidn::FilterRef filter = device.newFilter("RT");

            filter.setImage("color",  colorBuffer,  oidn::Format::Float3, windowWidth, windowHeight);
            filter.setImage("albedo", albedoBuffer, oidn::Format::Float3, windowWidth, windowHeight);
            filter.setImage("normal", normalBuffer, oidn::Format::Float3, windowWidth, windowHeight);

            //设置输出图像，内存与colorBuffer共享
            filter.setImage("output", colorBuffer,  oidn::Format::Float3, windowWidth, windowHeight);

            //设置 HDR 参数，输入线性的HDR数据
            filter.set("hdr", true);
            filter.commit();

            //执行降噪
            const Uint32 start = SDL_GetTicks();
            filter.execute();
            const Uint32 end = SDL_GetTicks();
            SDL_Log("Denoise Time: %u ms", end - start);

            //检查错误
            const char * errorMessage;
            if (device.getError(errorMessage) != oidn::Error::None) {
                SDL_Log("OIDN Error: %s, stop denoising", errorMessage);
                return;
            }

            //写入颜色
            const auto * denoisedColorPtr = static_cast<const float*>(colorBuffer.getData());

            for (int i = 0; i < windowHeight; i++) {
                for (int j = 0; j < windowWidth; j++) {
                    const size_t pixel_idx = (i * windowWidth + j) * 3;
                    Color3 color;
                    for (int k = 0; k < 3; k++) {
                        color[k] = denoisedColorPtr[pixel_idx + k];
                    }
                    color.writeColor(pixels + (i * windowWidth + j), format);
                }
            }
        }
    };
}

#endif //RENDERERBUILD_DENOISER_HPP
