#include "Dependencies/glew/glew.h"
#include "Dependencies/GLFW/glfw3.h"
#include "Dependencies/glm/glm.hpp"
#include "Dependencies/glm/gtc/matrix_transform.hpp"
#include <iostream>
#include <fstream>
#include <vector>

using namespace std;

// Mesh 结构
struct Mesh {
    GLuint VAO, VBO, EBO;
    int drawCount;
    GLenum drawMode;
    glm::vec3 position;
    glm::vec3 rotation;
    glm::vec3 scale;
    glm::vec3 color;
};

// Mesh 设置函数
void setupMesh(Mesh& mesh, float* vertices, int vSize, unsigned int* indices, int iSize, int vertexAttribs, bool hasColor) {
    glGenVertexArrays(1, &mesh.VAO);
    glGenBuffers(1, &mesh.VBO);
    if (indices) glGenBuffers(1, &mesh.EBO);

    glBindVertexArray(mesh.VAO);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.VBO);
    glBufferData(GL_ARRAY_BUFFER, vSize, vertices, GL_STATIC_DRAW);

    if (indices) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh.EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, iSize, indices, GL_STATIC_DRAW);
        mesh.drawCount = iSize / sizeof(unsigned int);
    }
    else {
        mesh.drawCount = vSize / (vertexAttribs * sizeof(float));
    }

    int stride = hasColor ? 6 * sizeof(float) : 3 * sizeof(float);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(0);

    if (hasColor) {
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
    }
    glBindVertexArray(0);
}

// Mesh 渲染函数
void renderMesh(const Mesh& mesh, GLuint programID) {
    if (mesh.VAO == 0) return;
    glUseProgram(programID);

    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, mesh.position);
    model = glm::rotate(model, glm::radians(mesh.rotation.x), glm::vec3(1, 0, 0));
    model = glm::rotate(model, glm::radians(mesh.rotation.y), glm::vec3(0, 1, 0));
    model = glm::rotate(model, glm::radians(mesh.rotation.z), glm::vec3(0, 0, 1));
    model = glm::scale(model, mesh.scale);

    glUniformMatrix4fv(glGetUniformLocation(programID, "model"), 1, GL_FALSE, &model[0][0]);
    glUniform3fv(glGetUniformLocation(programID, "incolor"), 1, &mesh.color[0]);

    glBindVertexArray(mesh.VAO);
    if (mesh.drawMode == GL_TRIANGLES && mesh.EBO != 0) {
        glDrawElements(mesh.drawMode, mesh.drawCount, GL_UNSIGNED_INT, 0);
    }
    else {
        glDrawArrays(mesh.drawMode, 0, mesh.drawCount);
    }
    glBindVertexArray(0);
}

// 全局变量
GLint programID;
const unsigned int SCR_WIDTH = 1600;
const unsigned int SCR_HEIGHT = 1600;
float x_press_num = 0.0f, y_press_num = 0.0f, z_press_num = 0.0f;
glm::vec3 cameraPos = glm::vec3(0.0f, -8.0f, 8.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, -1.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 0.5f, 0.0f);
float deltaTime = 0.0f, lastFrame = 0.0f;
bool firstMouse = true;
float yaw = -90.0f, pitch = 45.0f, lastX = SCR_WIDTH / 2.0f, lastY = SCR_HEIGHT / 2.0f, fov = 45.0f;
float aspect = (float)SCR_WIDTH / SCR_HEIGHT;
glm::vec3 colorPositions[] = {
    glm::vec3(0.01f, 0.01f, 0.01f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f),
    glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(1.0f, 1.0f, 0.0f), glm::vec3(1.0f, 0.0f, 1.0f),
    glm::vec3(0.0f, 1.0f, 1.0f)
};
int body_color_index = 1;
std::vector<Mesh> objects;
const GLfloat PI = 3.14159265358979323846f;
float sunAngle = 0.0f; // 太阳旋转角度
float grassColorTime = 0.0f; // 用于小草颜色变化的时间

// 生成球体的顶点和索引
void generateSphere(float radius, int sectors, int stacks, std::vector<float>& vertices, std::vector<unsigned int>& indices, glm::vec3 color) {
    vertices.clear();
    indices.clear();

    for (int i = 0; i <= stacks; ++i) {
        float phi = PI / 2 - i * PI / stacks;
        float y = radius * sin(phi);
        float r = radius * cos(phi);

        for (int j = 0; j <= sectors; ++j) {
            float theta = 2 * PI * j / sectors;
            float x = r * cos(theta);
            float z = r * sin(theta);
            vertices.push_back(x); vertices.push_back(y); vertices.push_back(z); // 位置
            vertices.push_back(color.r); vertices.push_back(color.g); vertices.push_back(color.b); // 颜色
        }
    }

    for (int i = 0; i < stacks; ++i) {
        for (int j = 0; j < sectors; ++j) {
            int a = i * (sectors + 1) + j;
            int b = a + 1;
            int c = (i + 1) * (sectors + 1) + j;
            int d = c + 1;
            indices.push_back(a); indices.push_back(b); indices.push_back(c);
            indices.push_back(b); indices.push_back(d); indices.push_back(c);
        }
    }
}
void updateGrassColor(std::vector<Mesh>& grasses) {
    grassColorTime += deltaTime; // 更新时间
    for (auto& grass : grasses) {
        // 使用正弦函数生成平滑的颜色变化
        float r = 0.5f + 0.5f * sin(grassColorTime + grass.position.x); // 红色分量
        float g = 0.5f + 0.5f * sin(grassColorTime + grass.position.y + 2.0f); // 绿色分量
        float b = 0.5f + 0.5f * sin(grassColorTime + grass.position.z + 4.0f); // 蓝色分量
        grass.color = glm::vec3(r, g, b);
    }
}

// Utility functions
void get_OpenGL_info() {
    const GLubyte* name = glGetString(GL_VENDOR);
    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* glversion = glGetString(GL_VERSION);
    std::cout << "OpenGL company: " << name << "\nRenderer: " << renderer << "\nVersion: " << glversion << std::endl;
}

bool checkStatus(GLuint objectID, PFNGLGETSHADERIVPROC objectPropertyGetterFunc,
    PFNGLGETSHADERINFOLOGPROC getInfoLogFunc, GLenum statusType) {
    GLint status;
    objectPropertyGetterFunc(objectID, statusType, &status);
    if (status != GL_TRUE) {
        GLint infoLogLength;
        objectPropertyGetterFunc(objectID, GL_INFO_LOG_LENGTH, &infoLogLength);
        GLchar* buffer = new GLchar[infoLogLength];
        GLsizei bufferSize;
        getInfoLogFunc(objectID, infoLogLength, &bufferSize, buffer);
        std::cout << buffer << std::endl;
        delete[] buffer;
        return false;
    }
    return true;
}

bool checkShaderStatus(GLuint shaderID) { return checkStatus(shaderID, glGetShaderiv, glGetShaderInfoLog, GL_COMPILE_STATUS); }
bool checkProgramStatus(GLuint programID) { return checkStatus(programID, glGetProgramiv, glGetProgramInfoLog, GL_LINK_STATUS); }

std::string readShaderCode(const char* fileName) {
    std::ifstream meInput(fileName);
    if (!meInput.good()) {
        std::cerr << "Failed to load shader file: " << fileName << std::endl;
        exit(1);
    }
    return std::string(std::istreambuf_iterator<char>(meInput), std::istreambuf_iterator<char>());
}

void installShaders() {
    GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    std::string vertexCode = readShaderCode("VertexShaderCode.glsl");
    const GLchar* adapter[1] = { vertexCode.c_str() };
    glShaderSource(vertexShaderID, 1, adapter, 0);

    std::string fragmentCode = readShaderCode("FragmentShaderCode.glsl");
    adapter[0] = fragmentCode.c_str();
    glShaderSource(fragmentShaderID, 1, adapter, 0);

    glCompileShader(vertexShaderID);
    glCompileShader(fragmentShaderID);

    if (!checkShaderStatus(vertexShaderID) || !checkShaderStatus(fragmentShaderID)) {
        std::cerr << "Shader compilation failed!" << std::endl;
        glDeleteShader(vertexShaderID);
        glDeleteShader(fragmentShaderID);
        return;
    }

    programID = glCreateProgram();
    glAttachShader(programID, vertexShaderID);
    glAttachShader(programID, fragmentShaderID);
    glLinkProgram(programID);

    if (!checkProgramStatus(programID)) {
        std::cerr << "Shader program linking failed!" << std::endl;
        glDeleteShader(vertexShaderID);
        glDeleteShader(fragmentShaderID);
        glDeleteProgram(programID);
        return;
    }

    glDeleteShader(vertexShaderID);
    glDeleteShader(fragmentShaderID);
    glUseProgram(programID);
}

void updateFishMovement(Mesh& fish) {
    static float moveSpeed = 0.5f;
    static float rotationAngle = 0.0f;
    static glm::vec3 prevPos = fish.position;
    static bool movingRight = true;

    if (movingRight) {
        fish.position.x += moveSpeed * deltaTime;
        if (fish.position.x > 4.0f) {
            fish.position.x = 4.0f;
            movingRight = false;
        }
    }
    else {
        fish.position.x -= moveSpeed * deltaTime;
        if (fish.position.x < -4.0f) {
            fish.position.x = -4.0f;
            movingRight = true;
        }
    }

    fish.position.y = -1.5f;
    fish.position.z = 0.2f;

    glm::vec3 moveDir = fish.position - prevPos;
    if (glm::length(moveDir) > 0) {
        float targetAngle = movingRight ? 0.0f : 180.0f;
        rotationAngle = glm::mix(rotationAngle, targetAngle, 0.1f);
    }
    fish.rotation.y = rotationAngle;
    prevPos = fish.position;
}

// 修改 updateSunRotation 函数中太阳的旋转高度
void updateSunRotation(Mesh& sun) {
    sunAngle += 10.0f * deltaTime; // 调整旋转速度
    if (sunAngle >= 360.0f) sunAngle -= 360.0f;
    float radius = 5.0f; // 太阳绕平面旋转的半径
    sun.position.x = radius * cos(glm::radians(sunAngle));
    sun.position.z = radius * sin(glm::radians(sunAngle));
    sun.position.y = 3.0f; // 降低太阳高度到 3.0f
}

void sendDataToOpenGL() {
    objects.clear();

    // Ground
    float quadVertices[] = {
        -4.0f, -4.0f, 0.0f, 0.8f, 0.6f, 0.2f,
         4.0f, -4.0f, 0.0f, 0.8f, 0.6f, 0.2f,
         4.0f,  4.0f, 0.0f, 0.8f, 0.6f, 0.2f,
        -4.0f,  4.0f, 0.0f, 0.8f, 0.6f, 0.2f
    };
    unsigned int quadIndices[] = { 0, 1, 2, 0, 2, 3 };
    Mesh quad;
    setupMesh(quad, quadVertices, sizeof(quadVertices), quadIndices, sizeof(quadIndices), 6, true);
    quad.drawMode = GL_TRIANGLES;
    quad.position = glm::vec3(0.0f, 0.0f, 0.0f);
    quad.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    quad.scale = glm::vec3(1.0f);
    quad.color = glm::vec3(0.8f, 0.6f, 0.2f);
    objects.push_back(quad);

    // River with stripes
    float riverVertices[] = {
        // 第一部分：深蓝色区域
        -4.0f, -0.5f, 0.05f, 0.0f, 0.3f, 0.8f, // 深蓝色
         4.0f, -0.5f, 0.05f, 0.0f, 0.3f, 0.8f,
         4.0f, -0.3f, 0.05f, 0.0f, 0.3f, 0.8f,
        -4.0f, -0.3f, 0.05f, 0.0f, 0.3f, 0.8f,
        // 第二部分：浅蓝色条纹
        -4.0f, -0.3f, 0.05f, 0.0f, 0.5f, 1.0f, // 浅蓝色
         4.0f, -0.3f, 0.05f, 0.0f, 0.5f, 1.0f,
         4.0f, -0.1f, 0.05f, 0.0f, 0.5f, 1.0f,
        -4.0f, -0.1f, 0.05f, 0.0f, 0.5f, 1.0f,
        // 第三部分：深蓝色区域
        -4.0f, -0.1f, 0.05f, 0.0f, 0.3f, 0.8f,
         4.0f, -0.1f, 0.05f, 0.0f, 0.3f, 0.8f,
         4.0f,  0.1f, 0.05f, 0.0f, 0.3f, 0.8f,
        -4.0f,  0.1f, 0.05f, 0.0f, 0.3f, 0.8f,
        // 第四部分：浅蓝色条纹
        -4.0f,  0.1f, 0.05f, 0.0f, 0.5f, 1.0f,
         4.0f,  0.1f, 0.05f, 0.0f, 0.5f, 1.0f,
         4.0f,  0.3f, 0.05f, 0.0f, 0.5f, 1.0f,
        -4.0f,  0.3f, 0.05f, 0.0f, 0.5f, 1.0f,
        // 第五部分：深蓝色区域
        -4.0f,  0.3f, 0.05f, 0.0f, 0.3f, 0.8f,
         4.0f,  0.3f, 0.05f, 0.0f, 0.3f, 0.8f,
         4.0f,  0.5f, 0.05f, 0.0f, 0.3f, 0.8f,
        -4.0f,  0.5f, 0.05f, 0.0f, 0.3f, 0.8f
    };

    unsigned int riverIndices[] = {
        0, 1, 2, 0, 2, 3,       // 第一部分
        4, 5, 6, 4, 6, 7,       // 第二部分（条纹）
        8, 9, 10, 8, 10, 11,    // 第三部分
        12, 13, 14, 12, 14, 15, // 第四部分（条纹）
        16, 17, 18, 16, 18, 19  // 第五部分
    };

    // 设置河流 Mesh
    Mesh river;
    setupMesh(river, riverVertices, sizeof(riverVertices), riverIndices, sizeof(riverIndices), 6, true);
    river.drawMode = GL_TRIANGLES;
    river.position = glm::vec3(0.0f, -1.5f, 0.0f);
    river.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    river.scale = glm::vec3(1.0f);
    river.color = glm::vec3(1.0f, 1.0f, 1.0f); // 使用顶点颜色，不覆盖
    objects.push_back(river);

    // Tree
    float treeVertices[] = {
        -0.1f, -0.1f, 0.0f, 0.55f, 0.27f, 0.07f,
         0.1f, -0.1f, 0.0f, 0.55f, 0.27f, 0.07f,
         0.1f,  0.1f, 0.0f, 0.55f, 0.27f, 0.07f,
        -0.1f,  0.1f, 0.0f, 0.55f, 0.27f, 0.07f,
        -0.1f, -0.1f, 0.4f, 0.55f, 0.27f, 0.07f,
         0.1f, -0.1f, 0.4f, 0.55f, 0.27f, 0.07f,
         0.1f,  0.1f, 0.4f, 0.55f, 0.27f, 0.07f,
        -0.1f,  0.1f, 0.4f, 0.55f, 0.27f, 0.07f,
        -0.3f, -0.3f, 0.4f, 0.0f,  0.6f,  0.0f,
         0.3f, -0.3f, 0.4f, 0.0f,  0.6f,  0.0f,
         0.3f,  0.3f, 0.4f, 0.0f,  0.6f,  0.0f,
        -0.3f,  0.3f, 0.4f, 0.0f,  0.6f,  0.0f,
         0.0f,  0.0f, 0.8f, 0.0f,  0.6f,  0.0f
    };
    unsigned int treeIndices[] = {
        0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7,
        0, 1, 5, 0, 4, 5, 2, 3, 7, 2, 6, 7, 0, 3, 7, 0, 4, 7, 1, 2, 6, 1, 5, 6,
        8, 9, 12, 9, 10, 12, 10, 11, 12, 11, 8, 12
    };
    Mesh tree;
    setupMesh(tree, treeVertices, sizeof(treeVertices), treeIndices, sizeof(treeIndices), 6, true);
    tree.drawMode = GL_TRIANGLES;
    tree.position = glm::vec3(-1.5f, 0.0f, 0.0f);
    tree.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    tree.scale = glm::vec3(1.5f); // 树变大
    tree.color = glm::vec3(1.0f, 1.0f, 1.0f);
    objects.push_back(tree);


    // Sun 
    std::vector<float> sunVertices;
    std::vector<unsigned int> sunIndices;
    generateSphere(0.5f, 20, 20, sunVertices, sunIndices, glm::vec3(1.0f, 0.0f, 0.0f));
    Mesh sun;
    setupMesh(sun, sunVertices.data(), sunVertices.size() * sizeof(float), sunIndices.data(), sunIndices.size() * sizeof(unsigned int), 6, true);
    sun.drawMode = GL_TRIANGLES;
    sun.position = glm::vec3(0.0f, 3.0f, 0.0f); 
    sun.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    sun.scale = glm::vec3(2.0f);
    sun.color = glm::vec3(1.0f, 0.0f, 0.0f);
    objects.push_back(sun);

    // Cloud 1 - 调整位置到天空
    float cloudVertices[] = {
       -0.6f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
       -0.3f, 0.3f, 0.0f, 1.0f, 1.0f, 1.0f,
        0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        0.3f, 0.3f, 0.0f, 1.0f, 1.0f, 1.0f,
        0.6f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f
    };
    unsigned int cloudIndices[] = { 0, 1, 2, 1, 2, 3, 2, 3, 4 };
    Mesh cloud1;
    setupMesh(cloud1, cloudVertices, sizeof(cloudVertices), cloudIndices, sizeof(cloudIndices), 6, true);
    cloud1.drawMode = GL_TRIANGLES;
    cloud1.position = glm::vec3(-1.0f, 5.0f, 2.0f); 
    cloud1.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    cloud1.scale = glm::vec3(1.5f);
    cloud1.color = glm::vec3(1.0f, 1.0f, 1.0f);
    objects.push_back(cloud1);

    // Cloud 2 - 调整位置到天空
    Mesh cloud2;
    setupMesh(cloud2, cloudVertices, sizeof(cloudVertices), cloudIndices, sizeof(cloudIndices), 6, true);
    cloud2.drawMode = GL_TRIANGLES;
    cloud2.position = glm::vec3(2.0f, 4.5f, 2.0f); // 调整位置到天空
    cloud2.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    cloud2.scale = glm::vec3(1.2f);
    cloud2.color = glm::vec3(1.0f, 1.0f, 1.0f);
    objects.push_back(cloud2);

    // Cloud 3 - 调整位置到天空
    Mesh cloud3;
    setupMesh(cloud3, cloudVertices, sizeof(cloudVertices), cloudIndices, sizeof(cloudIndices), 6, true);
    cloud3.drawMode = GL_TRIANGLES;
    cloud3.position = glm::vec3(-3.0f, 5.5f, 2.0f); // 调整位置到天空
    cloud3.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    cloud3.scale = glm::vec3(1.8f);
    cloud3.color = glm::vec3(1.0f, 1.0f, 1.0f);
    objects.push_back(cloud3);

    // Person - Head
    float headVertices[] = {
        -0.1f, -0.1f, 0.4f, 1.0f, 0.8f, 0.6f,
         0.1f, -0.1f, 0.4f, 1.0f, 0.8f, 0.6f,
         0.1f,  0.1f, 0.4f, 1.0f, 0.8f, 0.6f,
        -0.1f,  0.1f, 0.4f, 1.0f, 0.8f, 0.6f,
        -0.1f, -0.1f, 0.6f, 1.0f, 0.8f, 0.6f,
         0.1f, -0.1f, 0.6f, 1.0f, 0.8f, 0.6f,
         0.1f,  0.1f, 0.6f, 1.0f, 0.8f, 0.6f,
        -0.1f,  0.1f, 0.6f, 1.0f, 0.8f, 0.6f
    };
    unsigned int headIndices[] = { 0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7, 0, 1, 5, 0, 4, 5, 2, 3, 7, 2, 6, 7, 0, 3, 7, 0, 4, 7, 1, 2, 6, 1, 5, 6 };
    Mesh head;
    setupMesh(head, headVertices, sizeof(headVertices), headIndices, sizeof(headIndices), 6, true);
    head.drawMode = GL_TRIANGLES;
    head.position = glm::vec3(x_press_num, y_press_num, z_press_num + 0.4f);
    head.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    head.scale = glm::vec3(1.0f);
    head.color = glm::vec3(1.0f, 0.8f, 0.6f);
    objects.push_back(head);

    // Person - Body
    float bodyVertices[] = {
        -0.2f, -0.2f, 0.0f, 0.0f, 0.0f, 0.0f,
         0.2f, -0.2f, 0.0f, 0.0f, 0.0f, 0.0f,
         0.2f,  0.2f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.2f,  0.2f, 0.0f, 0.0f, 0.0f, 0.0f,
        -0.2f, -0.2f, 0.4f, 0.0f, 0.0f, 0.0f,
         0.2f, -0.2f, 0.4f, 0.0f, 0.0f, 0.0f,
         0.2f,  0.2f, 0.4f, 0.0f, 0.0f, 0.0f,
        -0.2f,  0.2f, 0.4f, 0.0f, 0.0f, 0.0f
    };
    unsigned int bodyIndices[] = { 0, 1, 2, 0, 2, 3, 4, 5, 6, 4, 6, 7, 0, 1, 5, 0, 4, 5, 2, 3, 7, 2, 6, 7, 0, 3, 7, 0, 4, 7, 1, 2, 6, 1, 5, 6 };
    Mesh body;
    setupMesh(body, bodyVertices, sizeof(bodyVertices), bodyIndices, sizeof(bodyIndices), 6, true);
    body.drawMode = GL_TRIANGLES;
    body.position = glm::vec3(x_press_num, y_press_num, z_press_num + 0.2f);
    body.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    body.scale = glm::vec3(1.0f);
    body.color = glm::vec3(0.0f, 0.0f, 0.0f);
    objects.push_back(body);

    // Person - Left Arm
    float leftArmVertices[] = {
        -0.3f, -0.05f, 0.2f, 1.0f, 0.8f, 0.6f,
        -0.3f,  0.05f, 0.2f, 1.0f, 0.8f, 0.6f,
        -0.3f, -0.05f, 0.4f, 1.0f, 0.8f, 0.6f,
        -0.3f,  0.05f, 0.4f, 1.0f, 0.8f, 0.6f,
        -0.5f, -0.05f, 0.2f, 1.0f, 0.8f, 0.6f,
        -0.5f,  0.05f, 0.2f, 1.0f, 0.8f, 0.6f,
        -0.5f, -0.05f, 0.4f, 1.0f, 0.8f, 0.6f,
        -0.5f,  0.05f, 0.4f, 1.0f, 0.8f, 0.6f
    };
    unsigned int leftArmIndices[] = { 0, 1, 2, 1, 2, 3, 4, 5, 6, 5, 6, 7, 0, 1, 5, 0, 4, 5, 2, 3, 7, 2, 6, 7, 0, 2, 6, 0, 4, 6, 1, 3, 7, 1, 5, 7 };
    Mesh leftArm;
    setupMesh(leftArm, leftArmVertices, sizeof(leftArmVertices), leftArmIndices, sizeof(leftArmIndices), 6, true);
    leftArm.drawMode = GL_TRIANGLES;
    leftArm.position = glm::vec3(x_press_num, y_press_num, z_press_num + 0.2f);
    leftArm.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    leftArm.scale = glm::vec3(1.0f);
    leftArm.color = glm::vec3(1.0f, 0.8f, 0.6f);
    objects.push_back(leftArm);

    // Person - Right Arm
    float rightArmVertices[] = {
         0.3f, -0.05f, 0.2f, 1.0f, 0.8f, 0.6f,
         0.3f,  0.05f, 0.2f, 1.0f, 0.8f, 0.6f,
         0.3f, -0.05f, 0.4f, 1.0f, 0.8f, 0.6f,
         0.3f,  0.05f, 0.4f, 1.0f, 0.8f, 0.6f,
         0.5f, -0.05f, 0.2f, 1.0f, 0.8f, 0.6f,
         0.5f,  0.05f, 0.2f, 1.0f, 0.8f, 0.6f,
         0.5f, -0.05f, 0.4f, 1.0f, 0.8f, 0.6f,
         0.5f,  0.05f, 0.4f, 1.0f, 0.8f, 0.6f
    };
    unsigned int rightArmIndices[] = { 0, 1, 2, 1, 2, 3, 4, 5, 6, 5, 6, 7, 0, 1, 5, 0, 4, 5, 2, 3, 7, 2, 6, 7, 0, 2, 6, 0, 4, 6, 1, 3, 7, 1, 5, 7 };
    Mesh rightArm;
    setupMesh(rightArm, rightArmVertices, sizeof(rightArmVertices), rightArmIndices, sizeof(rightArmIndices), 6, true);
    rightArm.drawMode = GL_TRIANGLES;
    rightArm.position = glm::vec3(x_press_num, y_press_num, z_press_num + 0.2f);
    rightArm.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    rightArm.scale = glm::vec3(1.0f);
    rightArm.color = glm::vec3(1.0f, 0.8f, 0.6f);
    objects.push_back(rightArm);

    // Person - Left Leg
    float leftLegVertices[] = {
        -0.15f, -0.05f, -0.2f, 1.0f, 0.8f, 0.6f,
        -0.15f,  0.05f, -0.2f, 1.0f, 0.8f, 0.6f,
        -0.15f, -0.05f,  0.0f, 1.0f, 0.8f, 0.6f,
        -0.15f,  0.05f,  0.0f, 1.0f, 0.8f, 0.6f,
        -0.25f, -0.05f, -0.2f, 1.0f, 0.8f, 0.6f,
        -0.25f,  0.05f, -0.2f, 1.0f, 0.8f, 0.6f,
        -0.25f, -0.05f,  0.0f, 1.0f, 0.8f, 0.6f,
        -0.25f,  0.05f,  0.0f, 1.0f, 0.8f, 0.6f
    };
    unsigned int leftLegIndices[] = { 0, 1, 2, 1, 2, 3, 4, 5, 6, 5, 6, 7, 0, 1, 5, 0, 4, 5, 2, 3, 7, 2, 6, 7, 0, 2, 6, 0, 4, 6, 1, 3, 7, 1, 5, 7 };
    Mesh leftLeg;
    setupMesh(leftLeg, leftLegVertices, sizeof(leftLegVertices), leftLegIndices, sizeof(leftLegIndices), 6, true);
    leftLeg.drawMode = GL_TRIANGLES;
    leftLeg.position = glm::vec3(x_press_num, y_press_num, z_press_num + 0.2f);
    leftLeg.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    leftLeg.scale = glm::vec3(1.0f);
    leftLeg.color = glm::vec3(1.0f, 0.8f, 0.6f);
    objects.push_back(leftLeg);

    // Person - Right Leg
    float rightLegVertices[] = {
         0.15f, -0.05f, -0.2f, 1.0f, 0.8f, 0.6f,
         0.15f,  0.05f, -0.2f, 1.0f, 0.8f, 0.6f,
         0.15f, -0.05f,  0.0f, 1.0f, 0.8f, 0.6f,
         0.15f,  0.05f,  0.0f, 1.0f, 0.8f, 0.6f,
         0.25f, -0.05f, -0.2f, 1.0f, 0.8f, 0.6f,
         0.25f,  0.05f, -0.2f, 1.0f, 0.8f, 0.6f,
         0.25f, -0.05f,  0.0f, 1.0f, 0.8f, 0.6f,
         0.25f,  0.05f,  0.0f, 1.0f, 0.8f, 0.6f
    };
    unsigned int rightLegIndices[] = { 0, 1, 2, 1, 2, 3, 4, 5, 6, 5, 6, 7, 0, 1, 5, 0, 4, 5, 2, 3, 7, 2, 6, 7, 0, 2, 6, 0, 4, 6, 1, 3, 7, 1, 5, 7 };
    Mesh rightLeg;
    setupMesh(rightLeg, rightLegVertices, sizeof(rightLegVertices), rightLegIndices, sizeof(rightLegIndices), 6, true);
    rightLeg.drawMode = GL_TRIANGLES;
    rightLeg.position = glm::vec3(x_press_num, y_press_num, z_press_num + 0.2f);
    rightLeg.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    rightLeg.scale = glm::vec3(1.0f);
    rightLeg.color = glm::vec3(1.0f, 0.8f, 0.6f);
    objects.push_back(rightLeg);

    // Fish
    float fishVertices[] = {
        0.5f,  0.0f, 0.0f, 1.0f, 0.8f, 0.0f,
        0.0f,  0.2f, 0.0f, 1.0f, 0.8f, 0.0f,
        0.0f, -0.2f, 0.0f, 1.0f, 0.8f, 0.0f,
       -0.5f,  0.0f, 0.0f, 1.0f, 0.8f, 0.0f,
       -0.7f,  0.1f, 0.0f, 1.0f, 0.8f, 0.0f,
       -0.7f, -0.1f, 0.0f, 1.0f, 0.8f, 0.0f
    };
    unsigned int fishIndices[] = { 0, 1, 2, 1, 2, 3, 3, 4, 5 };
    Mesh fish;
    setupMesh(fish, fishVertices, sizeof(fishVertices), fishIndices, sizeof(fishIndices), 6, true);
    fish.drawMode = GL_TRIANGLES;
    fish.position = glm::vec3(1.0f, -1.5f, 0.2f);
    fish.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    fish.scale = glm::vec3(1.0f);
    fish.color = glm::vec3(1.0f, 0.8f, 0.0f);
    objects.push_back(fish);

    // Mountain
    float Mountain[] = {
        -1.0f, -1.0f,  0.0f,  0.4f, 0.3f, 0.2f, // Bottom - Deep brown
         1.0f, -1.0f,  0.0f,  0.4f, 0.3f, 0.2f,
         1.0f,  1.0f,  0.0f,  0.4f, 0.3f, 0.2f,
        -1.0f,  1.0f,  0.0f,  0.4f, 0.3f, 0.2f,
        -0.5f, -0.5f,  1.0f,  0.8f, 0.8f, 0.8f, // Top - Light gray
         0.5f, -0.5f,  1.0f,  0.8f, 0.8f, 0.8f,
         0.5f,  0.5f,  1.0f,  0.8f, 0.8f, 0.8f,
        -0.5f,  0.5f,  1.0f,  0.8f, 0.8f, 0.8f,
        -0.75f, -0.75f,  0.5f,  0.6f, 0.5f, 0.4f, // Middle - Mid brown
         0.75f, -0.75f,  0.5f,  0.6f, 0.5f, 0.4f,
         0.75f,  0.75f,  0.5f,  0.6f, 0.5f, 0.4f,
        -0.75f,  0.75f,  0.5f,  0.6f, 0.5f, 0.4f,
    };

    unsigned int Mountain_indices[] = {
        0, 1, 3, 1, 2, 3, 2, 10, 11, 2, 11, 3, 2, 10, 1, 1, 10, 9,
        1, 8, 9, 1, 0, 9, 3, 0, 8, 3, 11, 8, 7, 11, 6, 10, 11, 6,
        6, 10, 5, 5, 10, 9, 4, 5, 9, 9, 4, 8, 4, 7, 11, 4, 8, 11,
        4, 6, 7, 4, 5, 6
    };

    Mesh mountain;
    setupMesh(mountain, Mountain, sizeof(Mountain), Mountain_indices, sizeof(Mountain_indices), 6, true);
    mountain.drawMode = GL_TRIANGLES;
    mountain.position = glm::vec3(2.0f, 2.0f, 0.1f); // Positioned in the background
    mountain.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    mountain.scale = glm::vec3(2.0f); // Scaled up for visibility
    mountain.color = glm::vec3(1.0f, 1.0f, 1.0f); // Use vertex colors
    objects.push_back(mountain);
    // House vertices and indices
    float House[] = {
        0.4f, 0.4f, 0.6f, 1.0f, 0.8f, 0.6f, 0.4f, -0.4f, 0.6f, 1.0f, 0.8f, 0.6f,
       -0.4f, -0.4f, 0.6f, 1.0f, 0.8f, 0.6f, -0.4f, 0.4f, 0.6f, 1.0f, 0.8f, 0.6f,
        0.0f, 0.4f, 1.0f, 1.0f, 0.8f, 0.6f, 0.0f, -0.4f, 1.0f, 1.0f, 0.8f, 0.6f,
        0.4f, 0.4f, 0.6f, 0.8f, 0.3f, 0.1f, 0.4f, -0.4f, 0.6f, 0.8f, 0.3f, 0.1f,
       -0.4f, -0.4f, 0.6f, 0.8f, 0.3f, 0.1f, -0.4f, 0.4f, 0.6f, 0.8f, 0.3f, 0.1f,
        0.0f, 0.4f, 1.0f, 0.8f, 0.3f, 0.1f, 0.0f, -0.4f, 1.0f, 0.8f, 0.3f, 0.1f,
        0.3f, 0.3f, 0.0f, 0.9f, 0.8f, 0.8f, 0.3f, -0.3f, 0.0f, 0.9f, 0.8f, 0.8f,
       -0.3f, -0.3f, 0.0f, 0.9f, 0.8f, 0.8f, -0.3f, 0.3f, 0.0f, 0.9f, 0.8f, 0.8f,
        0.3f, 0.3f, 0.6f, 0.9f, 0.8f, 0.8f, 0.3f, -0.3f, 0.6f, 0.9f, 0.8f, 0.8f,
       -0.3f, -0.3f, 0.6f, 0.9f, 0.8f, 0.8f, -0.3f, 0.3f, 0.6f, 0.9f, 0.8f, 0.8f,
        0.0f, 0.31f, 0.0f, 1.0f, 0.4f, 0.4f, 0.2f, 0.31f, 0.0f, 1.0f, 0.4f, 0.4f,
        0.0f, 0.31f, 0.4f, 1.0f, 0.4f, 0.4f, 0.2f, 0.31f, 0.4f, 1.0f, 0.4f, 0.4f,
       -0.1f, 0.31f, 0.4f, 0.5f, 0.8f, 1.0f, -0.2f, 0.31f, 0.4f, 0.5f, 0.8f, 1.0f,
       -0.1f, 0.31f, 0.5f, 0.5f, 0.8f, 1.0f, -0.2f, 0.31f, 0.5f, 0.5f, 0.8f, 1.0f,
    };

    unsigned int House_indices[] = {
        0, 3, 4, 2, 1, 5, 6, 10, 7, 7, 10, 11, 8, 9, 10, 8, 10, 11,
        12, 13, 15, 13, 15, 14, 12, 15, 19, 12, 19, 16, 12, 13, 16,
        13, 16, 17, 13, 14, 18, 13, 17, 18, 14, 15, 18, 15, 19, 18,
        20, 21, 22, 21, 22, 23, 24, 25, 26, 25, 26, 27
    };

    // House lines vertices and indices
    float House_Lines[] = {
        0.4f, 0.4f, 0.6f, 1.0f, 1.0f, 1.0f, 0.4f, -0.4f, 0.6f, 1.0f, 1.0f, 1.0f,
       -0.4f, -0.4f, 0.6f, 1.0f, 1.0f, 1.0f, -0.4f, 0.4f, 0.6f, 1.0f, 1.0f, 1.0f,
        0.0f, 0.4f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, -0.4f, 1.0f, 1.0f, 1.0f, 1.0f,
        0.3f, 0.3f, 0.0f, 1.0f, 1.0f, 1.0f, 0.3f, -0.3f, 0.0f, 1.0f, 1.0f, 1.0f,
       -0.3f, -0.3f, 0.0f, 1.0f, 1.0f, 1.0f, -0.3f, 0.3f, 0.0f, 1.0f, 1.0f, 1.0f,
        0.3f, 0.3f, 0.6f, 1.0f, 1.0f, 1.0f, 0.3f, -0.3f, 0.6f, 1.0f, 1.0f, 1.0f,
       -0.3f, -0.3f, 0.6f, 1.0f, 1.0f, 1.0f, -0.3f, 0.3f, 0.6f, 1.0f, 1.0f, 1.0f,
        0.0f, 0.31f, 0.0f, 1.0f, 1.0f, 1.0f, 0.2f, 0.31f, 0.0f, 1.0f, 1.0f, 1.0f,
        0.0f, 0.31f, 0.4f, 1.0f, 1.0f, 1.0f, 0.2f, 0.31f, 0.4f, 1.0f, 1.0f, 1.0f,
       -0.1f, 0.31f, 0.4f, 1.0f, 1.0f, 1.0f, -0.2f, 0.31f, 0.4f, 1.0f, 1.0f, 1.0f,
       -0.1f, 0.31f, 0.5f, 1.0f, 1.0f, 1.0f, -0.2f, 0.31f, 0.5f, 1.0f, 1.0f, 1.0f,
       -0.15f, 0.31f, 0.4f, 1.0f, 1.0f, 1.0f, -0.2f, 0.31f, 0.45f, 1.0f, 1.0f, 1.0f,
       -0.1f, 0.31f, 0.45f, 1.0f, 1.0f, 1.0f, -0.15f, 0.31f, 0.5f, 1.0f, 1.0f, 1.0f,
    };

    unsigned int House_Lines_indices[] = {
        0, 3, 0, 4, 3, 4, 0, 1, 1, 5, 5, 4, 1, 2, 2, 5, 2, 3, 6, 9,
        6, 7, 7, 8, 8, 9, 6, 10, 7, 11, 8, 12, 9, 13, 10, 13, 10, 11,
        11, 12, 12, 13, 14, 16, 16, 17, 15, 17, 18, 20, 20, 21, 21, 19,
        19, 18, 22, 25, 23, 24
    };
    // House
    Mesh house;
    setupMesh(house, House, sizeof(House), House_indices, sizeof(House_indices), 6, true);
    house.drawMode = GL_TRIANGLES;
    house.position = glm::vec3(-2.0f, -3.0f, 0.0f); // 将 y 坐标从 -4.0f 改为 -5.0f
    house.rotation = glm::vec3(0.0f, 0.0f, 180.0f);
    house.scale = glm::vec3(1.5f);
    house.color = glm::vec3(1.0f, 0.8f, 0.6f);
    objects.push_back(house);

    // House Lines
    Mesh houseLines;
    setupMesh(houseLines, House_Lines, sizeof(House_Lines), House_Lines_indices, sizeof(House_Lines_indices), 6, true);
    houseLines.drawMode = GL_LINES;
    houseLines.position = glm::vec3(-2.0f, -3.0f, 0.0f); // 将 y 坐标从 -4.0f 改为 -5.0f
    houseLines.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    houseLines.scale = glm::vec3(1.5f);
    houseLines.color = glm::vec3(1.0f, 1.0f, 1.0f);
    objects.push_back(houseLines);

    // Tree 2（后方树木）
    Mesh tree2;
    setupMesh(tree2, treeVertices, sizeof(treeVertices), treeIndices, sizeof(treeIndices), 6, true);
    tree2.drawMode = GL_TRIANGLES;
    tree2.position = glm::vec3(-2.0f, 1.0f, -0.0f); // 后方（z 轴负方向）
    tree2.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    tree2.scale = glm::vec3(1.2f); // 稍微小一点
    tree2.color = glm::vec3(1.0f, 1.0f, 1.0f);
    objects.push_back(tree2);

    // Tree 3（后方树木）
    Mesh tree3;
    setupMesh(tree3, treeVertices, sizeof(treeVertices), treeIndices, sizeof(treeIndices), 6, true);
    tree3.drawMode = GL_TRIANGLES;
    tree3.position = glm::vec3(1.5f, 0.0f, -0.0f); // 后方（z 轴负方向）
    tree3.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
    tree3.scale = glm::vec3(1.8f); // 稍微大一点
    tree3.color = glm::vec3(1.0f, 1.0f, 1.0f);
    objects.push_back(tree3);

    // 小草的顶点数据（三棱锥）
    float grassVertices[] = {
        -0.05f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // 底左
         0.05f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f, // 底右
         0.0f,  0.0f, 0.05f * sqrt(3), 1.0f, 1.0f, 1.0f, // 底前
         0.0f,  0.2f, 0.05f * sqrt(3) / 3, 1.0f, 1.0f, 1.0f  // 顶部
    };
    unsigned int grassIndices[] = { 0, 1, 3, 1, 2, 3, 2, 0, 3 };

    // 添加小草
    std::vector<glm::vec3> grassPositions = {
        //glm::vec3(-3.0f, -3.0f, 0.01f), glm::vec3(-2.5f, -2.5f, 0.01f), glm::vec3(-2.0f, -3.5f, 0.01f),
        glm::vec3(2.0f, -3.0f, 0.01f),  glm::vec3(2.5f, -2.5f, 0.01f),  glm::vec3(3.0f, -3.5f, 0.01f),
        glm::vec3(-3.0f, 2.0f, 0.01f),  glm::vec3(-2.5f, 2.5f, 0.01f),  glm::vec3(-2.0f, 1.5f, 0.01f),
        glm::vec3(2.0f, 2.0f, 0.01f),   glm::vec3(2.5f, 2.5f, 0.01f),   glm::vec3(3.0f, 1.5f, 0.01f)
    };

    for (const auto& pos : grassPositions) {
        Mesh grass;
        setupMesh(grass, grassVertices, sizeof(grassVertices), grassIndices, sizeof(grassIndices), 6, true);
        grass.drawMode = GL_TRIANGLES;
        grass.position = pos;
        grass.rotation = glm::vec3(0.0f, 0.0f, 0.0f);
        grass.scale = glm::vec3(2.0f);
        grass.color = glm::vec3(0.0f, 1.0f, 0.0f); // 初始绿色，通过 mesh.color 控制
        objects.push_back(grass);
    }
    // 定义星星的位置（保持与原设计一致）
    std::vector<glm::vec3> starPositions = {
        glm::vec3(-1.0f, 5.0f, 2.0f), // 靠近云1
        glm::vec3(2.0f, 4.5f, 2.0f),  // 靠近云2
        glm::vec3(-3.0f, 5.5f, 2.0f), // 靠近云3
        glm::vec3(0.0f, 6.0f, 3.0f),  // 天空更高处
        glm::vec3(3.0f, 5.0f, 2.5f)   // 另一位置
    };

    // 为每个星星生成小球模型
    for (const auto& pos : starPositions) {
        std::vector<float> starVertices;
        std::vector<unsigned int> starIndices;
        // 生成小球：半径 0.1，细分 10x10，颜色黄色
        generateSphere(0.1f, 10, 10, starVertices, starIndices, glm::vec3(1.0f, 1.0f, 0.0f));

        Mesh star;
        setupMesh(star, starVertices.data(), starVertices.size() * sizeof(float),
            starIndices.data(), starIndices.size() * sizeof(unsigned int), 6, true);
        star.drawMode = GL_TRIANGLES;              // 使用三角形绘制
        star.position = pos;                       // 设置位置
        star.rotation = glm::vec3(0.0f, 0.0f, 0.0f); // 初始无旋转
        star.scale = glm::vec3(1.0f);              // 缩放为 1.0（半径已定义）
        star.color = glm::vec3(1.0f, 1.0f, 0.0f); // 黄色
        objects.push_back(star);                   // 添加到场景对象列表
    }
}

void paintGL(void) {
    glClearColor(0.5f, 0.8f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 projection = glm::perspective(glm::radians(fov), aspect, 0.1f, 20.0f);
    glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
    glUniformMatrix4fv(glGetUniformLocation(programID, "projection"), 1, GL_FALSE, &projection[0][0]);
    glUniformMatrix4fv(glGetUniformLocation(programID, "view"), 1, GL_FALSE, &view[0][0]);

    // 更新动态对象
    objects[7].position = glm::vec3(x_press_num, y_press_num, z_press_num + 0.4f); // Head
    for (int i = 8; i <= 12; i++) { // Body, arms, legs
        objects[i].position = glm::vec3(x_press_num, y_press_num, z_press_num + 0.2f);
    }
    objects[7].color = colorPositions[body_color_index]; // 更新身体颜色
    objects[9].color = colorPositions[body_color_index]; // 更新身体颜色
    objects[10].color = colorPositions[body_color_index]; // 更新身体颜色
    objects[11].color = colorPositions[body_color_index]; // 更新身体颜色
    objects[12].color = colorPositions[body_color_index]; // 更新身体颜色
    updateFishMovement(objects[13]); // Fish
    updateSunRotation(objects[3]);   // Sun
    // 更新小草颜色（从索引 19 开始）
    for (size_t i = 19; i < objects.size()-5; ++i) {
        float r = 0.5f + 0.5f * sin(grassColorTime + objects[i].position.x);
        float g = 0.5f + 0.5f * sin(grassColorTime + objects[i].position.y + 2.0f);
        float b = 0.5f + 0.5f * sin(grassColorTime + objects[i].position.z + 4.0f);
        objects[i].color = glm::vec3(r, g, b);
    }
    grassColorTime += deltaTime;
    // Rotate stars
    for (size_t i = 31; i < objects.size(); ++i) {
        objects[i].rotation.y += 30.0f * deltaTime; // Rotate 30 degrees per second
        if (objects[i].rotation.y >= 360.0f) objects[i].rotation.y -= 360.0f;
    }
    // 渲染所有对象
    for (const auto& obj : objects) {
        renderMesh(obj, programID);
    }
}
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
    if (key == GLFW_KEY_UP && action == GLFW_PRESS) y_press_num = (y_press_num + 0.5f > 2.5f) ? -2.5f : y_press_num + 0.5f;
    if (key == GLFW_KEY_DOWN && action == GLFW_PRESS) y_press_num = (y_press_num - 0.5f < -2.5f) ? 2.5f : y_press_num - 0.5f;
    if (key == GLFW_KEY_LEFT && action == GLFW_PRESS) x_press_num = (x_press_num - 0.5f < -2.5f) ? 2.5f : x_press_num - 0.5f;
    if (key == GLFW_KEY_RIGHT && action == GLFW_PRESS) x_press_num = (x_press_num + 0.5f > 2.5f) ? -2.5f : x_press_num + 0.5f;
    if (key == GLFW_KEY_P && action == GLFW_PRESS) body_color_index = (body_color_index + 1) % 7; // 改为 'P' 键触发颜色变化
    if (key == GLFW_KEY_Q && action == GLFW_PRESS) aspect += 0.1f; // 增大宽高比
    if (key == GLFW_KEY_E && action == GLFW_PRESS) aspect = (aspect - 0.1f < 0.1f) ? 0.1f : aspect - 0.1f; // 减小宽高比
}

void mouse_callback(GLFWwindow* window, double xposIn, double yposIn) {
    float xpos = static_cast<float>(xposIn);
    float ypos = static_cast<float>(yposIn);
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;
    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;
    yaw += xoffset;
    pitch += yoffset;
    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;
    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraFront = glm::normalize(front);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    const float zoomSpeed = 2.0f;  // 缩放速度，可调整此值以控制灵敏度
    fov -= (float)yoffset * zoomSpeed;  // 保持原有方向，向上滚动放大
    if (fov < 10.0f) fov = 10.0f;  // 最小 fov，视野较窄
    if (fov > 90.0f) fov = 90.0f;  // 最大 fov，视野较宽
}

void initializedGL(void) {
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW in initializedGL" << std::endl;
        exit(-1);
    }
    sendDataToOpenGL();
    installShaders();
    glEnable(GL_DEPTH_TEST);
}

int main(int argc, char* argv[]) {
    GLFWwindow* window;
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Assignment 1", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    get_OpenGL_info();
    initializedGL();

    double jump_start_time = -1.0;
    double jump_duration = 1.0;

    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        float cameraSpeed = 2.5f * deltaTime;

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) cameraPos += cameraSpeed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) cameraPos -= cameraSpeed * cameraFront;
        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;

        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && jump_start_time < 0.0) {
            jump_start_time = glfwGetTime();
        }
        double current_time = glfwGetTime();
        if (jump_start_time >= 0.0 && current_time <= jump_start_time + jump_duration) {
            float jump_progress = (current_time - jump_start_time) / jump_duration;
            float jump_height = 1.0f;
            z_press_num = jump_height * sin(jump_progress * PI);
        }
        else if (current_time > jump_start_time + jump_duration) {
            jump_start_time = -1.0;
            z_press_num = 0.0f;
        }

        paintGL();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}