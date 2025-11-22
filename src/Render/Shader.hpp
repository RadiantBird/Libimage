#pragma once

#include <GL/glew.h> 
#include <string>

#include "src/Math/Vector3.hpp" 
#include "src/Math/Matrix4x4.hpp" // 【追加】

class Shader {
public:
    unsigned int ID;

    Shader(const char* vertexPath, const char* fragmentPath);
    void use();
    
    // Uniform設定関数
    void setInt(const std::string &name, int value) const;
    void setBool(const std::string &name, bool value) const; // 【修正: 重複 void 削除】
    void setVector3(const std::string &name, const Vector3 &value) const;
    void setVector3(const std::string &name, float x, float y, float z) const;
    void setMatrix4(const std::string &name, const Matrix4x4 &matrix) const; // 【追加】

private:
    void checkCompileErrors(unsigned int shader, std::string type);
};