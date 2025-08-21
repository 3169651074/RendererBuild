#include <util/Matrix.hpp>

namespace renderer {
    //辅助函数
    int Matrix::eliminateBottomElements(Matrix &matrix, size_t *swapCount) {
        size_t cnt = 0;
        //前向消元
        for (size_t i = 1; i <= matrix.row; i++) {
            //主元选择，高斯消元法需要实数，无论模板参数如何，都使用浮点数进行计算
            double main = std::abs(matrix.data[i][i]);
            size_t maxRow = i;
            for (size_t p = i + 1; p <= matrix.row; p++) {
                if (std::abs(matrix.data[p][i]) > main) {
                    main = fabs(matrix.data[p][i]);
                    maxRow = p;
                }
            }
            //检查最大主元
            if (floatValueNearZero(main)) {
                return 1;
            }

            if (maxRow != i) {
                //交换两行
                const size_t sz = matrix.col * sizeof(double);
                auto *tmp = new double[matrix.col];
                memcpy(tmp, matrix.data[i] + 1, sz);
                memcpy(matrix.data[i] + 1, matrix.data[maxRow] + 1, sz);
                memcpy(matrix.data[maxRow] + 1, tmp, sz);
                //记录交换次数
                cnt++;
                delete[] tmp;
            }

            //操作当前行（i）的下方所有行
            for (size_t j = i + 1; j <= matrix.row; j++) {
                double factor = matrix.data[j][i] / matrix.data[i][i];
                //操作一行
                for (size_t k = i; k <= matrix.col; k++) {
                    matrix.data[j][k] -= factor * matrix.data[i][k];
                }
            }
        }
        if (swapCount != nullptr) { *swapCount = cnt; }
        return 0;
    }

    int Matrix::eliminateTopElements(Matrix &matrix, size_t *swapCount) {
        for (size_t i = matrix.row; i >= 1; i--) {
            if (floatValueNearZero(matrix.data[i][i])) {
                if (!floatValueNearZero(matrix.data[i][matrix.col])) {
                    return 1; //无解
                } else {
                    return 2; //有无数个解
                }
            }

            double factor = 1 / matrix.data[i][i];
            //归一化系数，增广部分的所有元素都需要进行缩放
            for (size_t p = i; p <= matrix.col; p++) {
                matrix.data[i][p] *= factor;
            }
            for (size_t j = i - 1; j >= 1; j--) {
                factor = matrix.data[j][i];
                for (size_t k = j; k <= matrix.col; k++) {
                    matrix.data[j][k] -= factor * matrix.data[i][k];
                }
            }
        }
        return 0;
    }

    //矩阵运算函数
    Matrix Matrix::transpose() const {
        Matrix ret(col, row);
        //将第i行变为第i列
        for (size_t i = 1; i <= row; i++) {
            //取出第i行的每个元素，放到第i列的每个元素中
            for (size_t j = 1; j <= col; j++) {
                ret.data[j][i] = data[i][j];
            }
        }
        return ret;
    }

    Matrix Matrix::operator*(const Matrix &right) const {
        //检查两个矩阵是否满足相乘条件
        if (col != right.row) {
            //R"(string)" 为C++11原始字符串字面量，无需使用转义符
            throw std::runtime_error(R"("this" cannot multiply with "right"!)");
        }

        //创建新的矩阵，行数为左矩阵的行数，列数为右矩阵的列数
        Matrix ret(row, right.col);

        //矩阵乘法，逐个元素赋值
        for (size_t i = 1; i <= ret.row; i++) {
            for (size_t j = 1; j <= ret.col; j++) {
                double sum = 0.0;
                //计算左矩阵的第i行和右矩阵的第j列元素乘积之和，左矩阵的列数和右矩阵的行数相等
                for (size_t n = 1; n <= col; n++) {
                    sum += data[i][n] * right.data[n][j];
                }
                ret.data[i][j] = sum;
            }
        }
        return ret;
    }

    //求矩阵的行列式
    double Matrix::det() const {
        if (row != col) {
            throw std::runtime_error("Matrix does not have determinant!");
        }

        //需要对矩阵进行变换，则使用拷贝
        Matrix matrix(*this);

        /*
         * 对矩阵进行初等行变换，化为上三角矩阵，行列式的值为主对角线上元素乘积
         * 行列式交换两行，行列式反号，则在选择主元的过程中，需要统计交换次数
         */
        Matrix operateMatrix(row, col);
        for (size_t i = 1; i <= row; ++i) {
            for (size_t j = 1; j <= col; ++j) {
                operateMatrix.data[i][j] = this->data[i][j];
            }
        }

        size_t swapCount;
        if (eliminateBottomElements(operateMatrix, &swapCount) == 1) {
            /*
             * 如果最大主元为0，则说明此列所有元素都是0，同时防止下方被0除
             * 这种情况下，第i列可以被表示为前i-1列的线性组合，证明了矩阵的列向量是线性相关的
             * 则两列可以相消生成零列，行列式为0
             */
            return 0.0;
        }

        //使用辅助的对角线元素计算最终结果，原矩阵不再使用
        double ret = 1.0;
        for (size_t i = 1; i <= row; i++) {
            ret *= operateMatrix.data[i][i];
        }
        return swapCount % 2 == 0 ? ret : -ret;
    }

    Matrix Matrix::inverse() const {
        //必须是方阵才能有逆矩阵
        if (row != col) {
            throw std::runtime_error("Matrix is singular and has no inverse!");
        }

        //构造同阶单位矩阵，并将其合并到参数矩阵的右侧
        Matrix operateMatrix(row, col * 2);
        for (size_t i = 1; i <= row; i++) {
            //将原矩阵数据填入左半部分
            for (size_t j = 1; j <= col; j++) {
                operateMatrix.data[i][j] = this->data[i][j];
            }
            //将右半部分设置为单位矩阵
            operateMatrix.data[i][col + i] = 1.0;
        }

        //对合并后的矩阵进行两次消元
        if (eliminateBottomElements(operateMatrix, nullptr) != 0 || eliminateTopElements(operateMatrix) != 0) {
            //矩阵奇异（不满秩），无法求逆
            throw std::runtime_error("Matrix is singular and has no inverse!");
        }

        //操作矩阵的右半部分即为所求，转换类型后拷贝到结果矩阵上
        Matrix ret(*this);
        for (size_t i = 1; i <= row; i++) {
            for (size_t j = 1; j <= col; j++) {
                ret.data[i][j] = operateMatrix.data[i][j + col];
            }
        }
        return ret;
    }

    Matrix Matrix::orthogonalization() const {
        Matrix src(*this);

        //检查矩阵是否满秩
        Matrix tmp(src);
        if (eliminateBottomElements(tmp, nullptr) != 0) {
            //矩阵不满秩，无法正交化
            throw std::runtime_error("Matrix is not full rank so that cannot be orthogonalized!");
        }

        //施密特正交化过程中，每一个正交向量的计算都需要依靠之前n个向量的原始值，则不能直接修改参数矩阵
        Matrix ret(row, col);

        for (size_t i = 1; i <= row; i++) {
            //取出当前行向量
            Vector alphaS = src.getVector(i, true);

            //当前行结果向量
            Vector betaS(alphaS);
            for (int j = 1; j <= i - 1; j++) {
                //求乘积因子
                //此处为递推式，betaI由上一次计算结果得出，所以从ret矩阵中拿而不是src
                Vector betaI = ret.getVector(j, true);
                const double r1 = alphaS.dot(betaI);
                const double r2 = betaI.dot(betaI);
                if (floatValueNearZero(r2)) continue;
                betaI.multiplyByNumber(r1 / r2);

                //将当前向量从betaS中减去
                betaS.add(betaI, false);
            }

            //归一化向量
            const double mod = betaS.mod();
            if (floatValueNearZero(mod)) {
                throw std::runtime_error("Cannot normalize a zero vector during orthogonalization!");
            }
            betaS.multiplyByNumber(1.0 / mod);
            ret.setVector(&betaS, i, true);
        }
        return ret;
    }

    Point3 Matrix::toPoint() const {
        if (row != 4 || col != 1) {
            throw std::runtime_error("Only matrix with 4 row and 1 col can be cast to Point3!");
        }
        return Point3(data[1][1], data[2][1], data[3][1]);
    }

    Matrix Matrix::toMatrix(const Vec3 &obj, double w) {
        Matrix ret(4, 1);
        for (size_t i = 0; i < 3; i++) {
            ret.data[i + 1][1] = obj[i];
        }
        ret.data[4][1] = w;
        return ret;
    }

    Matrix Matrix::constructShiftMatrix(const std::array<double, 3> &shift) {
        return Matrix(4, 4, {
                1.0, 0.0, 0.0, shift[0],
                0.0, 1.0, 0.0, shift[1],
                0.0, 0.0, 1.0, shift[2],
                0.0, 0.0, 0.0, 1.0
        });
    }

    Matrix Matrix::constructScaleMatrix(const std::array<double, 3> &scale) {
        return Matrix(4, 4, {
                scale[0], 0.0, 0.0, 0.0,
                0.0, scale[1], 0.0, 0.0,
                0.0, 0.0, scale[2], 0.0,
                0.0, 0.0, 0.0, 1.0
        });
    }

    Matrix Matrix::constructRotateMatrix(double degree, int axis) {
        const double theta = degreeToRadian(degree);
        switch (axis) {
            case 0:
                return Matrix(4, 4, {
                        1.0, 0.0, 0.0, 0.0,
                        0.0, cos(theta), -sin(theta), 0.0,
                        0.0, sin(theta), cos(theta), 0.0,
                        0.0, 0.0, 0.0, 1.0
                });
            case 1:
                return Matrix(4, 4, {
                        cos(theta), 0.0, sin(theta), 0.0,
                        0.0, 1.0, 0.0, 0.0,
                        -sin(theta), 0.0, cos(theta), 0.0,
                        0.0, 0.0, 0.0, 1.0
                });
            case 2:
                return Matrix(4, 4, {
                        cos(theta), -sin(theta), 0.0, 0.0,
                        sin(theta), cos(theta), 0.0, 0.0,
                        0.0, 0.0, 1.0, 0.0,
                        0.0, 0.0, 0.0, 1.0
                });
            default:
                throw std::runtime_error("Invalid axis index!");
        }
    }

    Matrix Matrix::constructRotateMatrix(const std::array<double, 3> &rotate) {
        auto mx = constructRotateMatrix(rotate[0], 0);
        auto my = constructRotateMatrix(rotate[1], 1);
        auto mz = constructRotateMatrix(rotate[2], 2);
        return mx * my * mz;
    }

    Matrix::Vector Matrix::getVector(size_t n, bool isRow) const {
        if (n < 1 || n > (isRow ? col : row)) {
            throw std::runtime_error("Argument \"n\" is out of range!");
        }
        Vector ret(isRow ? col : row);
        //拷贝一行/一列元素
        if (isRow) {
            memcpy(ret.data + 1, data[n] + 1, col * sizeof(double));
        } else {
            for (size_t i = 1; i <= row; i++) {
                ret.data[i] = data[i][n];
            }
        }
        return ret;
    }

    void Matrix::setVector(const Vector *vector, size_t n, bool isRow) {
        //如果要操作一行，则长度为矩阵的列数
        if (vector->length != (isRow ? col : row)) {
            throw std::runtime_error(R"("matrix" and "vector" have different length!)");
        }
        if (n < 1 || n > (isRow ? col : row)) {
            throw std::runtime_error("Argument \"n\" is out of range!");
        }
        if (isRow) {
            memcpy(data[n] + 1, vector->data + 1, col * sizeof(double));
        } else {
            for (size_t i = 1; i <= row; i++) {
                data[i][n] = vector->data[i];
            }
        }
    }

    //构造析构函数
    Matrix::Matrix(size_t rowCount, size_t colCount) : row(rowCount), col(colCount) {
        data = new double *[row + 1]{};
        for (size_t i = 0; i < row + 1; i++) {
            data[i] = new double[col + 1]{};
        }
    }

    Matrix::Matrix(const Matrix &obj) : row(obj.row), col(obj.col) {
        data = new double *[row + 1]{};
        data[0] = new double[col + 1]{};

        for (size_t i = 1; i < row + 1; i++) {
            data[i] = new double[col + 1];
            //逐行复制内存
            memcpy(data[i] + 1, obj.data[i] + 1, col * sizeof(double));
        }
    }

    Matrix::Matrix(size_t rowCount, size_t colCount, std::initializer_list<double> list) : row(rowCount),
                                                                                           col(colCount) {
        data = new double *[row + 1]{};
        data[0] = new double[col + 1]{};

        auto it = list.begin();
        for (size_t i = 1; i < row + 1; i++) {
            data[i] = new double[col + 1]{};
            for (size_t j = 1; j < col + 1; j++) {
                if (it != list.end()) {
                    data[i][j] = *it;
                    it++;
                } //给出的参数数量不够时，自动使用类型默认值
            }
        }
    }

    Matrix &Matrix::operator=(const Matrix &obj) {
        if (this == &obj) return *this;

        //如果obj的大小和当前矩阵不同，则释放当前内存并重新分配
        if (obj.row != row || obj.col != col) {
            for (size_t i = 0; i < row + 1; i++) {
                delete[] data[i];
            }
            delete[] data;

            data = new double *[obj.row + 1]{};
            for (size_t i = 0; i < obj.row + 1; i++) {
                data[i] = new double[obj.col + 1]{};
            }
        }
        //拷贝数据
        for (size_t i = 1; i <= row; i++) {
            for (size_t j = 1; j <= col; j++) {
                data[i][j] = obj.data[i][j];
            }
        }
        return *this;
    }

    Matrix::~Matrix() {
        for (size_t i = 0; i < row + 1; i++) {
            delete[] data[i];
        }
        delete[] data;
    }

    Matrix &Matrix::operator+=(const Matrix &obj) {
        if (row != obj.row || col != obj.col) {
            throw std::runtime_error("Operate matrices must have same row and col!");
        }
        for (size_t i = 1; i <= row; i++) {
            for (size_t j = 1; j <= col; j++) {
                data[i][j] += obj.data[i][j];
            }
        }
        return *this;
    }

    Matrix &Matrix::operator-=(const Matrix &obj) {
        if (row != obj.row || col != obj.col) {
            throw std::runtime_error("Operate matrices must have same row and col!");
        }
        for (size_t i = 1; i <= row; i++) {
            for (size_t j = 1; j <= col; j++) {
                data[i][j] -= obj.data[i][j];
            }
        }
        return *this;
    }

    Matrix &Matrix::operator*=(double num) {
        for (size_t i = 1; i <= row; i++) {
            for (size_t j = 1; j <= col; j++) {
                data[i][j] *= num;
            }
        }
        return *this;
    }

    Matrix &Matrix::operator/=(double num) {
        for (size_t i = 1; i <= row; i++) {
            for (size_t j = 1; j <= col; j++) {
                data[i][j] /= num;
            }
        }
        return *this;
    }
}