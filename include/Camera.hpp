#ifndef RENDERERTEST_CAMERA_HPP
#define RENDERERTEST_CAMERA_HPP

#include <basic/Ray.hpp>
#include <basic/Color3.hpp>

namespace renderer {
    /*
     * 相机类
     * 相机类包含视口信息，决定了光线的发射位置和方向，作为参数传递到渲染函数中
     */
    class Camera {
    public:
        // ====== 窗口属性 ======
        Uint32 windowWidth;
        Uint32 windowHeight;

        //默认背景颜色，填充没有物体的区域
        Color3 backgroundColor;

        // ====== 相机属性 ======
        Point3 cameraCenter;
        Point3 cameraTarget;                    //相机放置在cameraCenter，看向cameraTarget，此两点间距为焦距
        double horizontalFOV;                   //水平方向的视角，构造时单位为角度，决定视口宽度

        //视口属性
        double viewPortWidth;
        double viewPortHeight;

        /*
         * 视口平面的基向量，用于定位视口
         * U指向相机的右侧（视口的右边界，V指向相机的上方（视口的上边界），W从center指向target
         */
        Vec3 cameraU, cameraV, cameraW;

        Vec3 viewPortX, viewPortY;              //视口平面方向向量
        Vec3 viewPortPixelDx, viewPortPixelDy;  //视口平面方向变化向量，用于定位每一个像素的空间位置
        Point3 viewPortOrigin;                  //视口原点（左上角）空间坐标
        Point3 pixelOrigin;                     //第一个像素的空间坐标

        //采样属性
        double focusDiskRadius;                 //光线虚化强度，采样平面的圆盘半径
        double focusDistance;                   //焦距

        Range shutterRange;                     //相机快门的开启时间段

        Uint32 sampleCount;                     //SSAA：每像素采样数
        double sampleRange;                     //SSAA：采样偏移半径

        Uint32 rayTraceDepth;                   //光线追踪深度

        Camera(Uint32 windowWidth, Uint32 windowHeight, const Color3 & backgroundColor,
               const Point3 & center, const Point3 & target, double fov, double focusDiskRadius,
               const Range & shutterRange, Uint32 sampleCount, double sampleRange,
               Uint32 rayTraceDepth, const Vec3 & upDirection) :
            windowWidth(windowWidth), windowHeight(windowHeight), backgroundColor(backgroundColor),
            cameraCenter(center), cameraTarget(target), horizontalFOV(fov), focusDiskRadius(focusDiskRadius),
            shutterRange(shutterRange), sampleCount(sampleCount), sampleRange(sampleRange),
            rayTraceDepth(rayTraceDepth), focusDistance(Point3::distance(cameraCenter, cameraTarget))
        {
            const double thetaFOV = degreeToRadian(horizontalFOV);
            const double vWidth = 2.0 * tan(thetaFOV / 2.0) * focusDistance;
            const double vHeight = vWidth / (windowWidth * 1.0 / windowHeight);

            this->viewPortWidth = vWidth;
            this->viewPortHeight = vHeight;

            this->cameraW = Point3::constructVector(cameraCenter, cameraTarget).unitVector();
            this->cameraU = Vec3::cross(cameraW, upDirection).unitVector();
            this->cameraV = Vec3::cross(cameraU, cameraW).unitVector();

            this->viewPortX = vWidth * cameraU;
            this->viewPortY = vHeight * -cameraV; //屏幕Y轴和空间坐标系的Y轴反向

            this->viewPortPixelDx = viewPortX / windowWidth;
            this->viewPortPixelDy = viewPortY / windowHeight;

            this->viewPortOrigin = cameraCenter + focusDistance * cameraW - viewPortX * 0.5 - viewPortY * 0.5;
            this->pixelOrigin = viewPortOrigin + viewPortPixelDx * 0.5 + viewPortPixelDy * 0.5;
        }

        std::string toString() const {
            std::string ret("Renderer Camera:\n");
            char buffer[4 * TOSTRING_BUFFER_SIZE] = { 0 };
            snprintf(buffer, 4 * TOSTRING_BUFFER_SIZE,
                     "\tWindow Size: %u x %u\n\tBackground Color: %s\n\t"
                     "Camera Direction: %s --> %s, FOV: %.4lf\n\t"
                     "Viewport Size: %.4lf x %.4lf\n\t"
                     "Viewport Base Vector: U = %s, V = %s, W = %s\n\t"
                     "Viewport Delta Vector: dx = %s, dy = %s\n\t"
                     "Viewport Origin: %s, Pixel Origin: %s\n\t"
                     "Sample Disk Radius: %.4lf, Focus Distance: %.4lf\n\t"
                     "Shutter %s\n\tSSAA Sample Count: %u, Range: %.2lf\n\t"
                     "Raytrace Depth: %u",
                     windowWidth, windowHeight, backgroundColor.toString().c_str(),
                     cameraCenter.toString().c_str(), cameraTarget.toString().c_str(),
                     horizontalFOV, viewPortWidth, viewPortHeight,
                     cameraU.toString().c_str(), cameraV.toString().c_str(), cameraW.toString().c_str(),
                     viewPortPixelDx.toString().c_str(), viewPortPixelDy.toString().c_str(),
                     viewPortOrigin.toString().c_str(), pixelOrigin.toString().c_str(),
                     focusDiskRadius, focusDistance, shutterRange.toString().c_str(), sampleCount, sampleRange, rayTraceDepth
            );
            return ret + buffer;
        }
    };
}

#endif //RENDERERTEST_CAMERA_HPP
