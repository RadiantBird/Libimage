#ifndef MATHUTILS_HPP
#define MATHUTILS_HPP

#include "Vector3.hpp"
#include <cmath>
#include <algorithm>

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

    // 転置行列
    Matrix3 transpose() const {
        Matrix3 res;
        for(int i=0; i<3; i++) for(int j=0; j<3; j++) res.m[i][j] = m[j][i];
        return res;
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
};

#endif