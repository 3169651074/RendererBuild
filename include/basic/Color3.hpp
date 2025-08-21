#ifndef COLOR3_HPP
#define COLOR3_HPP

#include <util/Range.hpp>

namespace renderer {
    /*
     * 颜色类：RGB三个参数，用[0.0, 1.0]之间的浮点数表示，允许超出范围，在写入时进行裁剪
     */
    class Color3 {
    private:
        double elements[3] {}; //C++11 空列表初始化，数组所有元素初始化为类型默认值

    public:
        explicit Color3(double r = 0.0, double g = 0.0, double b = 0.0) {
            elements[0] = r; elements[1] = g; elements[2] = b;
        }

        double & operator[](size_t index) {
            return elements[index];
        }
        double operator[](size_t index) const {
            return elements[index];
        }

        // ====== 对象操作函数 ======

        //不进行范围校验，只在写入时裁切范围
        Color3 & operator+=(const Color3 & obj) {
            for (size_t i = 0; i < 3; i++) {
                elements[i] += obj.elements[i];
            }
            return *this;
        }

        Color3 operator+(const Color3 & obj) const {
            Color3 ret(*this); ret += obj; return ret;
        }

        Color3 & operator-=(const Color3 & obj) {
            for (size_t i = 0; i < 3; i++) {
                elements[i] -= obj.elements[i];
            }
            return *this;
        }

        Color3 operator-(const Color3 & obj) const {
            Color3 ret(*this); ret -= obj; return ret;
        }

        Color3 & operator*=(double num) {
            for (double & element : elements) {
                element *= num;
            }
            return *this;
        }

        Color3 operator*(double num) const {
            Color3 ret(*this); ret *= num; return ret;
        }

        Color3 & operator/=(double num) {
            for (double & element : elements) {
                element /= num;
            }
            return *this;
        }

        Color3 operator/(double num) const {
            Color3 ret(*this); ret /= num; return ret;
        }

        Color3 & operator*=(const Color3 & obj) {
            for (size_t i = 0; i < 3; i++) {
                elements[i] *= obj.elements[i];
            }
            return *this;
        }

        Color3 operator*(const Color3 & obj) const {
            Color3 ret(*this); ret *= obj; return ret;
        }

        Color3 & operator/=(const Color3 & obj) {
            for (size_t i = 0; i < 3; i++) {
                elements[i] /= obj.elements[i];
            }
            return *this;
        }

        Color3 operator/(const Color3 & obj) const {
            Color3 ret(*this); ret /= obj; return ret;
        }

        friend Color3 operator*(double num, const Color3 & obj) {
            return obj * num;
        }

        friend Color3 operator/(double num, const Color3 & obj) {
            return obj / num;
        }

        //颜色写入函数
        void writeColor(Uint32 * pixelPointer, const SDL_PixelFormat * format, double gamma = 2.0) const {
            //进行伽马校正
            const double power = 1.0 / gamma;
            const double r = std::pow(elements[0], power);
            const double g = std::pow(elements[1], power);
            const double b = std::pow(elements[2], power);

            //将[0.0, 1.0]的颜色值映射到[0, 255]并写入
            const Range intensity(0.0, 0.999);
            const auto r_byte = static_cast<Uint8>(256 * intensity.clamp(r));
            const auto g_byte = static_cast<Uint8>(256 * intensity.clamp(g));
            const auto b_byte = static_cast<Uint8>(256 * intensity.clamp(b));

            *pixelPointer = SDL_MapRGB(format, r_byte, g_byte, b_byte);
        }

        // ====== 静态操作函数 ======

        //生成随机颜色
        static Color3 randomColor(double min = 0.0, double max = 1.0) {
            return Color3(randomDouble(min, max), randomDouble(min, max), randomDouble(min, max));
        }

        // ====== 类封装函数 ======

        std::string toString() const {
            char buffer[TOSTRING_BUFFER_SIZE] = { 0 };
            snprintf(buffer, TOSTRING_BUFFER_SIZE, "Color3: (%.2lf, %.2lf, %.2lf)", elements[0], elements[1], elements[2]);
            return {buffer};
        }
    };
}

#endif //COLOR3_HPP
