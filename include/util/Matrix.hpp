#ifndef RENDERERBUILD_MATRIX_HPP
#define RENDERERBUILD_MATRIX_HPP

#include <basic/Point3.hpp>

namespace renderer {
    class Matrix {
    public:
        size_t row{}, col{};
        double ** data;

        // ====== 构造析构函数 ======

        //构造空矩阵，所有元素初始化为0
        Matrix(size_t rowCount, size_t colCount);

        //使用初始化列表创建矩阵
        Matrix(size_t rowCount, size_t colCount, std::initializer_list<double> list);

        //拷贝构造和赋值运算符
        Matrix(const Matrix & obj);
        Matrix & operator=(const Matrix & obj);

        ~Matrix();

        // ====== 对象操作函数 ======

        //4行1列矩阵转为空间点
        Point3 toPoint() const;

        //矩阵加减
        Matrix & operator+=(const Matrix & obj);
        Matrix operator+(const Matrix & obj) const {
            Matrix ret(*this); ret += obj; return ret;
        }

        Matrix & operator-=(const Matrix & obj);
        Matrix operator-(const Matrix & obj) const {
            Matrix ret(*this); ret -= obj; return ret;
        }

        //矩阵数乘除，允许左操作数为数字
        Matrix & operator*=(double num);
        Matrix operator*(double num) const {
            Matrix ret(*this); ret *= num; return ret;
        }
        friend Matrix operator*(double num, const Matrix & obj) {
            return obj * num;
        }

        Matrix & operator/=(double num);
        Matrix operator/(double num) const {
            Matrix ret(*this); ret /= num; return ret;
        }
        friend Matrix operator/(double num, const Matrix & obj) {
            return obj / num;
        }

        //转置当前矩阵，返回新矩阵
        Matrix transpose() const;

        //矩阵乘法
        Matrix operator*(const Matrix & right) const;

        //求矩阵的行列式
        double det() const;

        //使用初等变换法求方阵的逆矩阵
        Matrix inverse() const;

        /*
         * 求当前矩阵的正交矩阵
         * 使用经典施密特正交法（CGS），数值稳定性较差，会产生误差累积，特别是在行向量接近线性相关时
         * 改进的施密特正交法MGS：迭代地用当前计算结果更新后面的向量，数值稳定性好，实际工程中应用
         */
        Matrix orthogonalization() const;

        // ====== 静态操作函数 ======

        //使用空间点或空间向量构造4行1列矩阵，点通过toVector方法转为向量后传入
        static Matrix toMatrix(const Vec3 & obj, double w);

        //构造三维平移矩阵
        static Matrix constructShiftMatrix(const std::array<double, 3> & shift);

        //构造三维缩放矩阵
        static Matrix constructScaleMatrix(const std::array<double, 3> & scale);

        //构造三维旋转矩阵，0，1，2表示x，y，z轴
        static Matrix constructRotateMatrix(double degree, int axis);
        static Matrix constructRotateMatrix(const std::array<double, 3> & rotate);

    private:
        // ====== 辅助函数 ======

        //使用高斯消元将矩阵化为阶梯型（上三角）矩阵，函数修改参数矩阵，当主元为0时返回1
        static int eliminateBottomElements(Matrix & matrix, size_t * swapCount = nullptr);

        //使用高斯消元将矩阵变为下三角矩阵，函数修改参数矩阵
        static int eliminateTopElements(Matrix & matrix, size_t * swapCount = nullptr);

        //矩阵行/列向量操作类
        //内部使用的Vector类，同矩阵类为1基
        class Vector {
        public:
            double * data;
            size_t length;

            explicit Vector(size_t length) : length(length) {
                data = new double[length + 1] {};
            }

            Vector(const Vector & obj) : length(obj.length) {
                data = new double[length + 1] {};
                memcpy(data + 1, obj.data + 1, length * sizeof(double));
            }

            Vector & operator=(const Vector & obj) {
                if (this == &obj) return *this;

                //如果obj的长度和当前向量不同，则释放当前内存并重新分配
                if (obj.length != length) {
                    delete[] data;
                    length = obj.length;
                    data = new double[length + 1] {};
                }
                //拷贝数据
                memcpy(data + 1, obj.data + 1, length * sizeof(double));
                return *this;
            }

            ~Vector() {
                delete[] data;
            }

            // ====== 对象操作函数 ======

            double mod() const {
                //n元向量的模为所有元素平方和开平方根
                double sum = 0.0;
                for (size_t i = 1; i <= length; i++) {
                    sum += data[i] * data[i];
                }
                return std::sqrt(sum);
            }

            Vector & multiplyByNumber(double num) {
                for (size_t i = 1; i <= length; i++) {
                    data[i] *= num;
                }
                return *this;
            }

            Vector & add(const Vector & addVector, bool isAdd) {
                for (size_t i = 1; i <= length; i++) {
                    data[i] = isAdd ? data[i] + addVector.data[i] : data[i] - addVector.data[i];
                }
                return *this;
            }

            double dot(const Vector & v2) const {
                double ret = 0.0;
                for (size_t i = 1; i <= length; i++) {
                    ret += static_cast<double>(data[i]) * static_cast<double>(v2.data[i]);
                }
                return ret;
            }
        };

        Vector getVector(size_t n, bool isRow) const;
        void setVector(const Vector * vector, size_t n, bool isRow);
    };
}

#endif //RENDERERBUILD_MATRIX_HPP
