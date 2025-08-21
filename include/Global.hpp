#ifndef RENDERERBUILD_GLOBAL_HPP
#define RENDERERBUILD_GLOBAL_HPP

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

#include <string>
#include <vector>
#include <queue>
#include <stack>
#include <memory>
#include <cmath>
#include <cstring>
#include <random>
#include <algorithm>
#include <limits>
#include <array>
#include <stdexcept>

#undef INFINITY
#undef NULL

namespace renderer {
    // ====== 数值常量 ======
    constexpr double FLOAT_VALUE_ZERO_EPSILON = 1e-5;
    constexpr double INFINITY = std::numeric_limits<double>::infinity();
    constexpr double PI = M_PI;
    constexpr Uint32 TOSTRING_BUFFER_SIZE = 200;

    // ====== 工具函数 ======
    inline double degreeToRadian(const double degree) {
        return degree * PI / 180.0;
    }

    inline double radianToDegree(const double radian) {
        return radian * 180.0 / PI;
    }

    //生成一个[0, 1)之间的浮点随机数
    inline double randomDouble() {
        static std::random_device rd; //需要初始化随机设备，让每次运行都能生成不同的随机数
        static std::uniform_real_distribution<> distribution(0.0, 1.0);
        static std::mt19937 generator(rd());
        return distribution(generator);
    }

    //生成一个[min, max)之间的浮点随机数
    inline double randomDouble(const double min, const double max) {
        return min + (max - min) * randomDouble();
    }

    //生成一个[min, max]之间的整数随机数
    inline int randomInt(const int min, const int max) {
        //直接使用 randomDouble() 来生成整数，以共享同一个静态生成器
        return static_cast<int>(randomDouble(min, max + 1));
    }

    //判断浮点数是否接近于0
    inline bool floatValueNearZero(const double val) {
        return std::abs(val) < FLOAT_VALUE_ZERO_EPSILON;
    }

    //判断两个浮点数是否相等
    inline bool floatValueEquals(const double v1, const double v2) {
        return std::abs(v1 - v2) < FLOAT_VALUE_ZERO_EPSILON;
    }

#define arrayLengthOnPos(array) sizeof(array) / sizeof(array[0])

    //====== SDL包装函数 ======

    enum TaskOnError {
        PRINT_MASSAGE, IGNORE, EXIT_PROGRAM
    };

    extern void (*func)();

    //调用此函数设置资源释放函数
    inline void registerReleaseSDLResources(void (*f)()) {
        func = f;
    }

#define releaseSDLResource(function, taskName) function; SDL_Log(taskName"...Done")
#define expandSDLColor(color) color.r, color.g, color.b, color.a

    //检查SDL函数返回值
    void sdlCheckErrorInt(int retVal, const char *taskName, TaskOnError errorTask);
    void sdlCheckErrorPtr(void *retVal, const char *taskName, TaskOnError errorTask);
}

#endif //RENDERERBUILD_GLOBAL_HPP
