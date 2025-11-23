#ifndef MATHUTILS_HPP
#define MATHUTILS_HPP

#include "Vector3.hpp"
#include <cmath>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

struct Matrix3 {
    float m[3][3];

    Matrix3() {
        // 単位行列で初期化
        for(int i=0; i<3; i++) for(int j=0; j<3; j++) m[i][j] = (i==j ? 1.0f : 0.0f);
    }

    void setZero() {
        for(int i=0; i<3; i++) for(int j=0; j<3; j++) m[i][j] = 0.0f;
    }

    // ベクトルとの積 (Matrix * Vector)
    Vector3 operator*(const Vector3& v) const {
        return Vector3(
            m[0][0]*v.x + m[0][1]*v.y + m[0][2]*v.z,
            m[1][0]*v.x + m[1][1]*v.y + m[1][2]*v.z,
            m[2][0]*v.x + m[2][1]*v.y + m[2][2]*v.z
        );
    }

    // 行列同士の積
    Matrix3 operator*(const Matrix3& other) const {
        Matrix3 res;
        res.setZero();
        for(int i=0; i<3; i++) {
            for(int j=0; j<3; j++) {
                for(int k=0; k<3; k++) {
                    res.m[i][j] += m[i][k] * other.m[k][j];
                }
            }
        }
        return res;
    }

    // --- 追加: スカラー倍 (Matrix * float) ---
    Matrix3 operator*(float s) const {
        Matrix3 res;
        for(int i=0; i<3; i++) for(int j=0; j<3; j++) res.m[i][j] = m[i][j] * s;
        return res;
    }

    // --- 追加: 行列の加算 (Matrix + Matrix) ---
    Matrix3 operator+(const Matrix3& other) const {
        Matrix3 res;
        for(int i=0; i<3; i++) for(int j=0; j<3; j++) res.m[i][j] = m[i][j] + other.m[i][j];
        return res;
    }

    // 転置行列
    Matrix3 transpose() const {
        Matrix3 res;
        for(int i=0; i<3; i++) for(int j=0; j<3; j++) res.m[i][j] = m[j][i];
        return res;
    }

    // --- 追加: 正規直交化 (回転行列の誤差蓄積を補正) ---
    void orthonormalize() {
        // 列ベクトルとして取り出す
        Vector3 x(m[0][0], m[1][0], m[2][0]);
        Vector3 y(m[0][1], m[1][1], m[2][1]);
        Vector3 z;

        x = x.normalized();
        z = x.cross(y).normalized(); // Z軸を再計算 (XとYに垂直)
        y = z.cross(x).normalized(); // Y軸を再計算 (ZとXに垂直)

        // 行列に戻す
        m[0][0]=x.x; m[1][0]=x.y; m[2][0]=x.z;
        m[0][1]=y.x; m[1][1]=y.y; m[2][1]=y.z;
        m[0][2]=z.x; m[1][2]=z.y; m[2][2]=z.z;
    }

    // オイラー角(度数法)から回転行列を作成
    static Matrix3 rotate(const Vector3& rotDeg) {
        float radX = rotDeg.x * M_PI / 180.0f;
        float radY = rotDeg.y * M_PI / 180.0f;
        float radZ = rotDeg.z * M_PI / 180.0f;

        float cx = cos(radX), sx = sin(radX);
        float cy = cos(radY), sy = sin(radY);
        float cz = cos(radZ), sz = sin(radZ);

        Matrix3 mX, mY, mZ;
        mX.m[1][1]=cx; mX.m[1][2]=-sx; mX.m[2][1]=sx; mX.m[2][2]=cx;
        mY.m[0][0]=cy; mY.m[0][2]=sy;  mY.m[2][0]=-sy; mY.m[2][2]=cy;
        mZ.m[0][0]=cz; mZ.m[0][1]=-sz; mZ.m[1][0]=sz; mZ.m[1][1]=cz;

        // Z * Y * X の順で適用 (一般的なオイラー角順序)
        return mZ * mY * mX;
    }

    // --- 追加: 回転行列からオイラー角(度数法)を復元 ---
    Vector3 toEuler() const {
        Vector3 angle;
        // Z * Y * X 行列の成分から逆算
        // m[2][0] は -sin(y) に相当
        float sy = -m[2][0];
        float cy = sqrt(1.0f - sy * sy);

        if (cy > 1e-6f) {
            angle.x = atan2(m[2][1], m[2][2]);
            angle.y = atan2(sy, cy);
            angle.z = atan2(m[1][0], m[0][0]);
        } else {
            // ジンバルロック対策 (Y軸が+/-90度付近)
            angle.x = atan2(-m[1][2], m[1][1]);
            angle.y = atan2(sy, cy);
            angle.z = 0.0f;
        }
        
        return angle * (180.0f / M_PI);
    }
};

#endif