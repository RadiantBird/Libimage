//NOTE: g++ -I/opt/homebrew/include -L/opt/homebrew/lib -o engine engine.cpp Shader.cpp -framework OpenGL -lglfw -lGLEW -lm -DSTB_IMAGE_IMPLEMENTATION

#define GL_SILENCE_DEPRECATION

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <OpenGL/gl.h>
#include <cmath>
#include <vector>
#include <tuple>
#include <iostream>
#include <algorithm>
#include "stb_image.h"

// --- 変更点: ヘッダファイルの読み込み ---
#include "Vector3.hpp" // ベクトル定義
#include "Shader.hpp"  // シェーダクラス定義
// ------------------------------------

// --- 定数 ---
const float SCREEN_W = 800;
const float SCREEN_H = 600;
const float FOV_Y = 60.0f;
const float Z_NEAR = 0.1f;
const float Z_FAR = 2000.0f;

// ※ ここにあった Vector3 定義、シェーダ文字列、コンパイル関数は全て削除！

// -----------------------------
// Cube (Vector3定義は削除済)
// -----------------------------
struct Cube {
    Vector3 size, pos, color, rotation;
    unsigned int textureID;
    bool useTexture;
    bool anchored;
    Vector3 velocity, acceleration;
    Vector3 angularVelocity; 
    
    float mass, invMass, invInertiaTensor;
    bool isPlayer, onGround;

    Cube(Vector3 s, Vector3 p, Vector3 c, Vector3 r = Vector3(0,0,0),
         unsigned int texID = 0, bool useTex = false, bool an = false, bool isPl = false)
         : size(s), pos(p), color(c), rotation(r), textureID(texID), 
           useTexture(useTex), anchored(an), velocity(0,0,0), acceleration(0,0,0), 
           angularVelocity(0,0,0), isPlayer(isPl), onGround(false)
    {
        float density = 1.0f;
        mass = s.x * s.y * s.z * density;
        if (mass < 1e-6f) mass = 1.0f; 
        invMass = 1.0f / mass;
        float I = (1.0f/18.0f) * mass * (size.x*size.x + size.y*size.y + size.z*size.z); 
        if (I < 1e-6f) I = 1.0f;
        invInertiaTensor = 1.0f / I;
        
        if (isPl || an) { invInertiaTensor = 0.0f; }
        if (an) { mass = 1e10f; invMass = 0.0f; }
    }
};

std::vector<Cube> workspace;
unsigned int skyboxTextureID = 0;
unsigned int whiteTextureID = 0;
const size_t PLAYER_INDEX = 0;
bool isFreeCam = false;
Vector3 camOffsetLocal(0.0f, 50.0f, 100.0f);

// --- 変更点: グローバル変数をポインタに変更 (初期化タイミング制御のため) ---
Shader* lightingShader = nullptr; 
// -------------------------------------------------------------------

// -----------------------------
// カメラ
// -----------------------------
struct Camera {
    Vector3 pos;
    Vector3 rotation;

    Camera() : pos(0,0,0), rotation(0,0,0) {}

    std::tuple<Vector3,Vector3,Vector3> get_directions() const {
        float yaw = rotation.y*M_PI/180, pitch = rotation.x*M_PI/180;
        Vector3 forward(-sin(yaw)*cos(pitch), -sin(pitch), -cos(yaw)*cos(pitch));
        Vector3 right(cos(yaw), 0, -sin(yaw));
        Vector3 up = right.cross(forward); 
        return {forward.normalized(), right.normalized(), up.normalized()};
    }
};

// -----------------------------
// テクスチャ読み込み
// -----------------------------
unsigned int loadTexture(const char* filename){
    unsigned int tex;
    glGenTextures(1,&tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    int w,h,nc;
    stbi_set_flip_vertically_on_load(true);
    unsigned char *data = stbi_load(filename,&w,&h,&nc,0);
    if(data){
        GLenum fmt = (nc==1?GL_RED:(nc==3?GL_RGB:GL_RGBA));
        glTexImage2D(GL_TEXTURE_2D,0,fmt,w,h,0,fmt,GL_UNSIGNED_BYTE,data);
    } else { tex=0; }
    stbi_image_free(data);
    return tex;
}

unsigned int createWhiteTexture() {
    unsigned int tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    unsigned char data[] = {255, 255, 255};
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    return tex;
}

// -----------------------------
// 物理
// -----------------------------
Vector3 rotateX(Vector3 v, float deg){
    float rad = deg*M_PI/180.0f;
    return Vector3(v.x, v.y*cos(rad)-v.z*sin(rad), v.y*sin(rad)+v.z*cos(rad));
}
Vector3 rotateY(Vector3 v, float deg){
    float rad = deg*M_PI/180.0f;
    return Vector3(v.x*cos(rad)+v.z*sin(rad), v.y, -v.x*sin(rad)+v.z*cos(rad));
}
Vector3 rotateZ(Vector3 v, float deg){
    float rad = deg*M_PI/180.0f;
    return Vector3(v.x*cos(rad)-v.y*sin(rad), v.x*sin(rad)+v.y*cos(rad), v.z);
}
Vector3 localToWorld(Vector3 v, Vector3 rotationDeg) {
    v = rotateX(v, rotationDeg.x);
    v = rotateY(v, rotationDeg.y);
    v = rotateZ(v, rotationDeg.z);
    return v;
}

void simulatePhysics(float dt){
    Vector3 gravity(0,-98.0f,0);
    const int subSteps = 4;
    float subDt = dt / subSteps;
    
    if (PLAYER_INDEX < workspace.size()) workspace[PLAYER_INDEX].onGround = false;

    for(int step=0; step<subSteps; ++step){
        for(auto &c:workspace){
            if(c.anchored) continue;
            c.velocity += gravity * subDt;
            c.velocity *= 0.98f; c.angularVelocity *= 0.98f;
            c.pos += c.velocity * subDt;
            c.rotation += c.angularVelocity * subDt;
        }

        for(size_t i=0;i<workspace.size();++i){
            Cube &a = workspace[i];
            for(size_t j=i+1;j<workspace.size();++j){
                Cube &b = workspace[j]; 
                if(a.anchored && b.anchored) continue; 

                Vector3 delta = a.pos - b.pos;
                Vector3 overlap(
                    (a.size.x/2 + b.size.x/2) - std::abs(delta.x),
                    (a.size.y/2 + b.size.y/2) - std::abs(delta.y),
                    (a.size.z/2 + b.size.z/2) - std::abs(delta.z)
                );

                if(overlap.x>0 && overlap.y>0 && overlap.z>0){
                    int axis = 0; float minO = overlap.x;
                    if(overlap.y<minO){ axis=1; minO=overlap.y; }
                    if(overlap.z<minO){ axis=2; minO=overlap.z; }

                    Vector3 normal(0,0,0);
                    if(axis==0) normal.x = (delta.x>0?1:-1);
                    else if(axis==1) normal.y = (delta.y>0?1:-1);
                    else normal.z = (delta.z>0?1:-1);

                    if (a.isPlayer && axis == 1 && normal.y == 1) a.onGround = true;
                    if (b.isPlayer && axis == 1 && normal.y == -1) b.onGround = true;

                    float invMassSum = a.invMass + b.invMass; if(invMassSum < 1e-6) continue;
                    float push = minO / invMassSum;
                    a.pos += normal * (push * a.invMass);
                    if(!b.anchored) b.pos -= normal * (push * b.invMass);
                    
                    Vector3 relVel = a.velocity - b.velocity;
                    float vn = relVel.dot(normal);
                    if(vn < 0){
                        float jVal = -(1.0f + 0.2f) * vn / invMassSum;
                        Vector3 impulse = normal * jVal;
                        a.velocity += impulse * a.invMass;
                        if(!b.anchored) b.velocity -= impulse * b.invMass;
                    }
                }
            }
        }
    }
}

// -----------------------------
// 描画
// -----------------------------
void renderCube(Cube &cube){
    // --- 変更点: シェーダクラスの使用 ---
    if (lightingShader) lightingShader->use();
    // -------------------------------

    glActiveTexture(GL_TEXTURE0);
    if (cube.useTexture && cube.textureID != 0) {
        glBindTexture(GL_TEXTURE_2D, cube.textureID);
    } else {
        glBindTexture(GL_TEXTURE_2D, whiteTextureID);
    }

    glPushMatrix();
    glTranslatef(cube.pos.x, cube.pos.y, cube.pos.z);
    glRotatef(cube.rotation.z, 0, 0, 1);
    glRotatef(cube.rotation.y, 0, 1, 0);
    glRotatef(cube.rotation.x, 1, 0, 0);

    float hx = cube.size.x/2, hy = cube.size.y/2, hz = cube.size.z/2;
    glColor3f(cube.color.x/255.0f, cube.color.y/255.0f, cube.color.z/255.0f);

    glBegin(GL_QUADS); 
    
    glNormal3f(0, 0, 1);
    glTexCoord2f(0, 1); glVertex3f(-hx, -hy, hz);
    glTexCoord2f(1, 1); glVertex3f( hx, -hy, hz);
    glTexCoord2f(1, 0); glVertex3f( hx,  hy, hz);
    glTexCoord2f(0, 0); glVertex3f(-hx,  hy, hz);
    
    glNormal3f(0, 0, -1);
    glTexCoord2f(0, 1); glVertex3f( hx, -hy, -hz);
    glTexCoord2f(1, 1); glVertex3f(-hx, -hy, -hz);
    glTexCoord2f(1, 0); glVertex3f(-hx,  hy, -hz);
    glTexCoord2f(0, 0); glVertex3f( hx,  hy, -hz);

    glNormal3f(-1, 0, 0);
    glTexCoord2f(0, 1); glVertex3f(-hx, -hy, -hz);
    glTexCoord2f(1, 1); glVertex3f(-hx, -hy,  hz);
    glTexCoord2f(1, 0); glVertex3f(-hx,  hy,  hz);
    glTexCoord2f(0, 0); glVertex3f(-hx,  hy, -hz);

    glNormal3f(1, 0, 0);
    glTexCoord2f(0, 1); glVertex3f( hx, -hy,  hz);
    glTexCoord2f(1, 1); glVertex3f( hx, -hy, -hz);
    glTexCoord2f(1, 0); glVertex3f( hx,  hy, -hz);
    glTexCoord2f(0, 0); glVertex3f( hx,  hy,  hz);

    glNormal3f(0, 1, 0);
    glTexCoord2f(0, 1); glVertex3f(-hx,  hy,  hz);
    glTexCoord2f(1, 1); glVertex3f( hx,  hy,  hz);
    glTexCoord2f(1, 0); glVertex3f( hx,  hy, -hz);
    glTexCoord2f(0, 0); glVertex3f(-hx,  hy, -hz);

    glNormal3f(0, -1, 0);
    glTexCoord2f(0, 1); glVertex3f(-hx, -hy, -hz);
    glTexCoord2f(1, 1); glVertex3f( hx, -hy, -hz);
    glTexCoord2f(1, 0); glVertex3f( hx, -hy,  hz);
    glTexCoord2f(0, 0); glVertex3f(-hx, -hy,  hz);

    glEnd();
    glPopMatrix();
    glUseProgram(0);
}

void applyLookAt(const Vector3& eye, const Vector3& center, const Vector3& up) {
    Vector3 f = (center - eye).normalized();
    Vector3 s = f.cross(up).normalized();
    Vector3 u = s.cross(f);

    float m[16] = {
         s.x,  u.x, -f.x,  0,
         s.y,  u.y, -f.y,  0,
         s.z,  u.z, -f.z,  0,
           0,    0,    0,  1
    };
    
    glMultMatrixf(m);
    glTranslatef(-eye.x, -eye.y, -eye.z);
}

void renderScene(const Camera& cam, const Vector3& target){
    glLoadIdentity();
    applyLookAt(cam.pos, target, Vector3(0, 1, 0));
    for(auto &c:workspace) renderCube(c);
}

// -----------------------------
// main
// -----------------------------
int main(){
    if(!glfwInit()) return -1;
    GLFWwindow* win=glfwCreateWindow((int)SCREEN_W,(int)SCREEN_H,"3D Engine",NULL,NULL);
    if(!win){ glfwTerminate(); return -1; }
    glfwMakeContextCurrent(win); glfwSwapInterval(1);
    if (glewInit() != GLEW_OK) return -1;

    glEnable(GL_DEPTH_TEST); glDepthFunc(GL_LESS);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float aspect = SCREEN_W / SCREEN_H;
    float fH = tan(FOV_Y / 360.0f * M_PI) * Z_NEAR;
    float fW = fH * aspect;
    glFrustum(-fW, fW, -fH, fH, Z_NEAR, Z_FAR);
    
    glMatrixMode(GL_MODELVIEW);

    // --- 変更点: シェーダクラスのインスタンス化 ---
    // ファイルからシェーダを読み込む
    lightingShader = new Shader("basic.vert", "basic.frag");
    lightingShader->use();
    lightingShader->setInt("tex0", 0);
    glUseProgram(0);
    // -----------------------------------------

    Camera mainCamera;
    skyboxTextureID = loadTexture("salad-cat.jpg"); 
    whiteTextureID = createWhiteTexture();

    workspace.push_back(Cube(Vector3(4,6,2),Vector3(0, 0, 0),Vector3(255,100,0),Vector3(0,0,0),0,false,false, true)); 
    workspace.push_back(Cube(Vector3(2048,5,2048),Vector3(0,-5,0),Vector3(0,255,0),Vector3(0,0,0),0,false,true)); 
    workspace.push_back(Cube(Vector3(10,10,10),Vector3(5,10,20),Vector3(0,100,200),Vector3(30,0,45))); 
    workspace.push_back(Cube(Vector3(5,5,5),Vector3(-10,10,0),Vector3(0,150,100))); 
    workspace.push_back(Cube(Vector3(6,6,6),Vector3(10, 50, 10),Vector3(255,0,0))); 
    if (skyboxTextureID != 0) {
        workspace.push_back(Cube(Vector3(8,8,8),Vector3(-20, 0, 10),Vector3(255,255,255),Vector3(0,0,0), skyboxTextureID, true, false));
    }

    float lastTime=glfwGetTime();
    Vector3 lookTarget(0,0,0);

    while(!glfwWindowShouldClose(win)){
        float cur=glfwGetTime(); float dt=cur-lastTime; lastTime=cur;
        if(dt>0.1f) dt=0.1f; 

        glfwPollEvents();

        if (PLAYER_INDEX < workspace.size()) {
            Cube &p = workspace[PLAYER_INDEX];
            lookTarget = p.pos + Vector3(0, 20.0f, 0);
        } else {
             lookTarget = Vector3(0, 0, 0);
        }

        if(glfwGetKey(win,GLFW_KEY_P)==GLFW_PRESS) isFreeCam = !isFreeCam;

        if(glfwGetKey(win,GLFW_KEY_UP)) mainCamera.rotation.x -= 1.5f; 
        if(glfwGetKey(win,GLFW_KEY_DOWN)) mainCamera.rotation.x += 1.5f;
        if(glfwGetKey(win,GLFW_KEY_LEFT)) mainCamera.rotation.y -= 1.5f;
        if(glfwGetKey(win,GLFW_KEY_RIGHT)) mainCamera.rotation.y += 1.5f;
        mainCamera.rotation.x = std::max(-89.0f, std::min(89.0f, mainCamera.rotation.x));

        Vector3 f,r,u; std::tie(f,r,u)=mainCamera.get_directions();

        if (isFreeCam) {
            float s = 50.0f * dt;
            if(glfwGetKey(win,GLFW_KEY_W)) mainCamera.pos+=f*s;
            if(glfwGetKey(win,GLFW_KEY_S)) mainCamera.pos-=f*s;
            if(glfwGetKey(win,GLFW_KEY_A)) mainCamera.pos-=r*s;
            if(glfwGetKey(win,GLFW_KEY_D)) mainCamera.pos+=r*s;
            if(glfwGetKey(win,GLFW_KEY_Q)) mainCamera.pos-=u*s; 
            if(glfwGetKey(win,GLFW_KEY_E)) mainCamera.pos+=u*s; 
        } else if (PLAYER_INDEX < workspace.size()) {
            Cube &p = workspace[PLAYER_INDEX];
            Vector3 flatF = Vector3(f.x, 0, f.z).normalized();
            Vector3 flatR = Vector3(r.x, 0, r.z).normalized();
            
            Vector3 targetV(0, p.velocity.y, 0);
            float speed = 50.0f;
            if(glfwGetKey(win,GLFW_KEY_W)) targetV += flatF * speed;
            if(glfwGetKey(win,GLFW_KEY_S)) targetV -= flatF * speed;
            if(glfwGetKey(win,GLFW_KEY_A)) targetV -= flatR * speed;
            if(glfwGetKey(win,GLFW_KEY_D)) targetV += flatR * speed;
            if(glfwGetKey(win,GLFW_KEY_SPACE) && p.onGround) targetV.y = 100.0f;

            p.velocity.x += (targetV.x - p.velocity.x)*0.1f;
            p.velocity.z += (targetV.z - p.velocity.z)*0.1f;
            p.velocity.y = targetV.y;

            Vector3 f,r,u; std::tie(f,r,u)=mainCamera.get_directions();
            float distance = 100.0f;
            mainCamera.pos = lookTarget - f * distance;
            workspace[PLAYER_INDEX].rotation.y = mainCamera.rotation.y;
        }

        simulatePhysics(dt);
        glClearColor(0.2f,0.3f,0.3f,1); 
        glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        renderScene(mainCamera, lookTarget);
        glfwSwapBuffers(win);
    }

    // 終了処理
    delete lightingShader;
    glfwTerminate();
    return 0;
}