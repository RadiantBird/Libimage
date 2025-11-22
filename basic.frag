#version 150 core // GLSL 150 に戻す
in vec3 vNormal;
in vec3 vPosition;
in vec4 vColor;
in vec2 vTexCoord;

uniform sampler2D tex0;
uniform vec3 objectColor; 
uniform bool useTexture; 

// 【修正点】 gl_FragColor の代わりに、出力変数を宣言
out vec4 FragColor;

void main() {
    vec3 lightDir = normalize(vec3(1.0, 1.0, 0.5));
    vec3 normal = normalize(vNormal);
    vec3 viewDir = normalize(-vPosition);

    float diff = max(dot(normal, lightDir), 0.0);
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = 0.0;
    if(diff > 0.0)
        spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);

    vec3 texBaseColor;
    if (useTexture) {
        // vColor は頂点シェーダで常に vec4(1.0) のため、テクスチャの色のみ
        texBaseColor = texture(tex0, vTexCoord).rgb; // texture() は GLSL 150 の関数
    } else {
        // objectColor のみ
        texBaseColor = objectColor; 
    }
    
    vec3 ambient = texBaseColor * 0.3;
    vec3 diffuse = texBaseColor * diff * 0.8;
    vec3 specular = vec3(1.0) * spec * 0.5;
    
    // 【修正点】 gl_FragColor の代わりに FragColor に出力
    FragColor = vec4(clamp(ambient + diffuse + specular, 0.0, 1.0), 1.0); 
}