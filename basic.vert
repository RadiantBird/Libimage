#version 150 core // macOSの一般的なモダンOpenGLバージョン

// C++ 側で VAO 属性 0 に設定された頂点位置
layout (location = 0) in vec3 aPos; 

// C++ 側から渡される行列
uniform mat4 projection; 
uniform mat4 view;       
uniform mat4 model;      

// Fragment Shader に渡す情報
out vec3 vNormal;
out vec3 vPosition;
out vec4 vColor;
out vec2 vTexCoord;

void main() {
    // 頂点位置の変換: MVP * Position
    vec4 worldPos = model * vec4(aPos, 1.0);
    gl_Position = projection * view * worldPos;
    
    // モデルビュー空間での位置 (視点空間)
    vPosition = vec3(view * worldPos); 
    
    // 法線変換 (VBOに法線データがないため、仮の法線 (0, 1, 0) を View空間に変換)
    mat3 modelView3x3 = mat3(view * model); 
    vNormal = normalize(modelView3x3 * vec3(0.0, 1.0, 0.0));

    // 色とテクスチャ座標 (VBOにデータがないため、固定値を使用)
    vColor = vec4(1.0, 1.0, 1.0, 1.0); 
    vTexCoord = vec2(0.0, 0.0); 
}