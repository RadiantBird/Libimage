#ifndef MATRIX4X4_HPP
#define MATRIX4X4_HPP

#include "Vector3.hpp"
#include "MathUtils.hpp"
#include <cmath>
#include <cstring> 

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

// 4x4 行列構造体 (OpenGLの列優先フォーマットに対応)
struct Matrix4x4 {
    float m[4][4]; 

    Matrix4x4() {
        identity();
    }

    void identity() {
        memset(m, 0, sizeof(m));
        m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1.0f;
    }

    Matrix4x4 operator*(const Matrix4x4& other) const {
        Matrix4x4 res;
        for(int i=0; i<4; i++) {
            for(int j=0; j<4; j++) {
                res.m[i][j] = 0.0f;
                for(int k=0; k<4; k++) { 
                    res.m[i][j] += m[i][k] * other.m[k][j];
                }
            }
        }
        return res;
    }
    
    static Matrix4x4 translate(const Vector3& t) {
        Matrix4x4 res;
        res.m[0][3] = t.x;
        res.m[1][3] = t.y;
        res.m[2][3] = t.z;
        return res;
    }

    static Matrix4x4 scale(const Vector3& s) {
        Matrix4x4 res;
        res.m[0][0] = s.x;
        res.m[1][1] = s.y;
        res.m[2][2] = s.z;
        return res;
    }
    
    static Matrix4x4 fromRotation(const Vector3& rotDeg) {
        Matrix3 rot3 = Matrix3::rotate(rotDeg); 
        
        Matrix4x4 res;
        res.identity();
        
        for(int i=0; i<3; i++) {
            for(int j=0; j<3; j++) {
                res.m[i][j] = rot3.m[i][j];
            }
        }
        return res;
    }
    
    // 透視投影行列 (OpenGLの列優先フォーマットに修正)
    static Matrix4x4 perspective(float fovY, float aspect, float nearP, float farP) {
        Matrix4x4 res;
        memset(res.m, 0, sizeof(res.m)); // すべてゼロで初期化
        
        float f = 1.0f / tan(fovY * 0.5f * (M_PI / 180.0f));

        // OpenGLは列優先なので、m[col][row]としてアクセス
        // 数学的な行列表記 M[row][col] を m[col][row] に配置
        res.m[0][0] = f / aspect;                          // 列0, 行0
        res.m[1][1] = f;                                   // 列1, 行1
        res.m[2][2] = (farP + nearP) / (nearP - farP);     // 列2, 行2
        res.m[2][3] = -1.0f;                               // 列2, 行3
        res.m[3][2] = (2.0f * farP * nearP) / (nearP - farP); // 列3, 行2
        
        return res;
    }
};

#endif // MATRIX4X4_HPP