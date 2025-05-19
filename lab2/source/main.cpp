/*
Student Information
Student ID:202208040412
Student Name:邹林壮
*/

#include "Dependencies/glew/glew.h"
#include "Dependencies/GLFW/glfw3.h"
#include "Dependencies/glm/glm.hpp"
#include "Dependencies/glm/gtc/matrix_transform.hpp"
#include "Dependencies/glm/gtc/type_ptr.hpp"
#include "Dependencies/stb_image.h"

#include "Texture.h"
#include "Shader.h"
#include "Camera.h"
#include "Model.h"

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <map>

using namespace std;

// 基本设置
const float SCR_WIDTH = 2400;
const float SCR_HEIGHT = 1800;
const GLfloat PI = 3.14159265358979323846f;
const int Y_SEGMENTS = 100;
const int X_SEGMENTS = 100;

// 缓冲区对象
unsigned int VBO[4], VAO[4], EBO[3];
unsigned int cubemap_texture, sky_vao, sky_vbo;

// 纹理对象
Textures texture[4], sky_texture;
Textures texture_snow[2];
Textures NBA[14];

// 纹理文件列表
std::vector<std::string> snow{
    "resources/model/snow/snow_01.jpg",
    "resources/model/snow/snow_02.jpg"
};

std::vector<std::string> court{
    "resources/texture/court/football.jpg",
    "resources/texture/court/laker.png",
    "resources/texture/court/76.png",
    "resources/texture/court/buxingzhe.png",
    "resources/texture/court/duxingxia.png",
    "resources/texture/court/huangfeng.png",
    "resources/texture/court/huixiong.png",
    "resources/texture/court/huosai.png",
    "resources/texture/court/juejin.png",
    "resources/texture/court/jueshi.png",
    "resources/texture/court/kaierteren.png",
    "resources/texture/court/kaituozhe.png",
    "resources/texture/court/king.png",
    "resources/texture/court/kuaichuan.png",
};

std::vector<std::string> faces{
    "resources/texture/jaj_snow/jajsnow1_rt.tga",
    "resources/texture/jaj_snow/jajsnow1_lf.tga",
    "resources/texture/jaj_snow/jajsnow1_up.tga",
    "resources/texture/jaj_snow/jajsnow1_dn.tga",
    "resources/texture/jaj_snow/jajsnow1_bk.tga",
    "resources/texture/jaj_snow/jajsnow1_ft.tga"
};

// 索引和状态变量
unsigned int snow_index = 0;
unsigned int penguin_index = 0;
int court_choice = 0;
bool day = true;
bool sp = true;
bool camera_move = false;
bool light_pole_switch = false;
int spotlight = -1;

// 时间和位置变量
float delta_time = 0.0f;
float last_frame = 0.0f;
float last_x = SCR_WIDTH / 2.0f;
float last_y = SCR_HEIGHT / 2.0f;
float penguin_x = 0.0f;
float penguin_y = 0.0f;
float penguin_z = 0.0f;
float delta = 0.0f;
float angle = 0.0f;

// 摄像机和光照
Camera camera(glm::vec3(0.0f, 2.5f, 6.0f));
glm::vec3 lightPos(0.0f, 0.0f, 0.0f);
glm::vec3 DirLight_shininess(2.0);
float DirLight_shininess2 = 1.0;

// 场景对象位置
glm::vec3 light_pole_pos[] = {
    glm::vec3(10.0f, -0.5f, 8.0f),
    glm::vec3(10.0f, -0.5f, -8.0f),
    glm::vec3(-10.0f, -0.5f, 8.0f),
    glm::vec3(-10.0f, -0.5f, -8.0f)
};

float light_pole_angle[] = {
    glm::radians(180.0f),
    glm::radians(180.0f),
    glm::radians(0.0f),
    glm::radians(0.0f)
};

glm::vec3 lighting_pole_pos[] = {
    glm::vec3(7.0f, 4.8f, 8.0f),
    glm::vec3(7.0f, 4.8f, -8.0f),
    glm::vec3(-7.0f, 4.8f, 8.0f),
    glm::vec3(-7.0f, 4.8f, -8.0f)
};

glm::vec3 grandstand_pos[] = {
    glm::vec3(15.0f, 0.0f, 8.0f),
    glm::vec3(15.0f, 0.0f, -8.0f),
    glm::vec3(-15.0f, 0.0f, 8.0f),
    glm::vec3(-15.0f, 0.0f, -8.0f)
};

float grandstand_angle[] = {
    glm::radians(-90.0f),
    glm::radians(-90.0f),
    glm::radians(90.0f),
    glm::radians(90.0f)
};

// 天空盒顶点数据
float skybox_vertices[] = {
    -1.0f,  1.0f, -1.0f,
    -1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f, -1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,

    -1.0f, -1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f, -1.0f,  1.0f,
    -1.0f, -1.0f,  1.0f,

    -1.0f,  1.0f, -1.0f,
     1.0f,  1.0f, -1.0f,
     1.0f,  1.0f,  1.0f,
     1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f,  1.0f,
    -1.0f,  1.0f, -1.0f,

    -1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f, -1.0f,
     1.0f, -1.0f, -1.0f,
    -1.0f, -1.0f,  1.0f,
     1.0f, -1.0f,  1.0f
};

// Shader对象
Shader shader, sky_shader, light_shader;

// 函数声明
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void cursor_position_callback(GLFWwindow* window, double x, double y);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void ProcessInput(GLFWwindow* window);

// 获取OpenGL信息
void get_OpenGL_info() {
    const GLubyte* name = glGetString(GL_VENDOR);
    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* glversion = glGetString(GL_VERSION);
    std::cout << "OpenGL company: " << name << std::endl;
    std::cout << "Renderer name: " << renderer << std::endl;
    std::cout << "OpenGL version: " << glversion << std::endl;
}

// 初始化OpenGL数据
void sendDataToOpenGL() {
    // 生成球体顶点和索引
    std::vector<float> sphereVertices;
    std::vector<int> sphereIndices;

    for (int y = 0; y <= Y_SEGMENTS; y++) {
        for (int x = 0; x <= X_SEGMENTS; x++) {
            float xSegment = (float)x / (float)X_SEGMENTS;
            float ySegment = (float)y / (float)Y_SEGMENTS;
            float xPos = std::cos(xSegment * 2.0f * PI) * std::sin(ySegment * PI);
            float yPos = std::cos(ySegment * PI);
            float zPos = std::sin(xSegment * 2.0f * PI) * std::sin(ySegment * PI);

            sphereVertices.push_back(xPos);
            sphereVertices.push_back(yPos);
            sphereVertices.push_back(zPos);
        }
    }

    for (int i = 0; i < Y_SEGMENTS; i++) {
        for (int j = 0; j < X_SEGMENTS; j++) {
            sphereIndices.push_back(i * (X_SEGMENTS + 1) + j);
            sphereIndices.push_back((i + 1) * (X_SEGMENTS + 1) + j);
            sphereIndices.push_back((i + 1) * (X_SEGMENTS + 1) + j + 1);

            sphereIndices.push_back(i * (X_SEGMENTS + 1) + j);
            sphereIndices.push_back((i + 1) * (X_SEGMENTS + 1) + j + 1);
            sphereIndices.push_back(i * (X_SEGMENTS + 1) + j + 1);
        }
    }

    // 四边形顶点数据
    float Quad[] = {
        0.54f,  0.29f, 0.0f,   0.0f,0.0f,1.0f, 1.0f,1.0f,
        0.54f, -0.29f, 0.0f,   0.0f,0.0f,1.0f, 1.0f,0.0f,
       -0.54f, -0.29f, 0.0f,   0.0f,0.0f,1.0f, 0.0f,0.0f,

        0.54f,  0.29f, 0.0f,   0.0f,0.0f,1.0f, 1.0f,1.0f,
       -0.54f, -0.29f, 0.0f,   0.0f,0.0f,1.0f, 0.0f,0.0f,
       -0.54f,  0.29f, 0.0f,   0.0f,0.0f,1.0f, 0.0f,1.0f
    };

    // 生成VAO, VBO, EBO
    glGenVertexArrays(4, VAO);
    glGenBuffers(4, VBO);
    glGenBuffers(3, EBO);

    // 加载纹理
    for (int i = 0; i < snow.size(); i++) {
        texture_snow[i].setupTexture(snow[i].c_str());
    }
    for (int i = 0; i < court.size(); i++) {
        NBA[i].setupTexture(court[i].c_str());
    }

    // 设置四边形数据
    glBindVertexArray(VAO[1]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
    glBufferData(GL_ARRAY_BUFFER, sizeof(Quad), Quad, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (char*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (char*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    // 设置光源球体数据
    glBindVertexArray(VAO[2]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[2]);
    glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float), &sphereVertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[2]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(int), &sphereIndices[0], GL_STATIC_DRAW);

    // 设置灯柱数据
    glBindVertexArray(VAO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
    glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float), &sphereVertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO[0]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(int), &sphereIndices[0], GL_STATIC_DRAW);

    // 设置天空盒数据
    glGenVertexArrays(1, &sky_vao);
    glGenBuffers(1, &sky_vbo);
    glBindVertexArray(sky_vao);
    glBindBuffer(GL_ARRAY_BUFFER, sky_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skybox_vertices), &skybox_vertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    // 加载立方体贴图
    glGenTextures(1, &cubemap_texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap_texture);
    int width, height, nrchannels;
    for (unsigned int i = 0; i < faces.size(); i++) {
        stbi_set_flip_vertically_on_load(false);
        unsigned char* data = stbi_load(faces[i].c_str(), &width, &height, &nrchannels, 0);
        if (data) {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else {
            std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }

    // 设置立方体贴图的纹理参数
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

}

// 初始化OpenGL
void initializedGL() {
    if (glewInit() != GLEW_OK) {
        std::cout << "GLEW not OK." << std::endl;
    }

    get_OpenGL_info();
    sendDataToOpenGL();
    shader.setupShader("./VertexShaderCode.glsl", "./FragmentShaderCode.glsl");
    sky_shader.setupShader("./skybox.vs", "./skybox.fs");
    light_shader.setupShader("./lamp.vertex", "./lamp.frag");

    glEnable(GL_DEPTH_TEST);
}

// 配置光照
void setupLighting() {
    shader.setVec3("viewPos", camera.position);
    shader.setFloat("material.shininess", 128.0f);

    // Directional light
    shader.setVec3("dirLight.direction", -0.0f, -1.0f, -0.0f);
    shader.setVec3("dirLight.ambient", 0.05f, 0.05f, 0.05f);
    shader.setVec3("dirLight.diffuse", 0.4f, 0.4f, 0.4f);
    shader.setVec3("dirLight.specular", 0.5f, 0.5f, 0.5f);
    shader.setVec3("DirLight_shininess", DirLight_shininess);
    shader.setFloat("DirLight_shininess2", DirLight_shininess2);

    // Point light
    shader.setVec3("pointLights[0].position", lightPos);
    shader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
    shader.setVec3("pointLights[0].diffuse", 0.8f, 0.8f, 0.8f);
    shader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
    shader.setFloat("pointLights[0].constant", 1.0f);
    shader.setFloat("pointLights[0].linear", 0.05f);
    shader.setFloat("pointLights[0].quadratic", 0.001f);

    // 灯柱光源
    for (int j = 0; j < 4; j++) {
        char uniformName[128];
        std::snprintf(uniformName, 128, "pointLights[%d].position", j + 1);
        shader.setVec3(uniformName, lighting_pole_pos[j]);

        if (light_pole_switch) {
            std::snprintf(uniformName, 128, "pointLights[%d].ambient", j + 1);
            shader.setVec3(uniformName, 0.05f, 0.05f, 0.05f);
            std::snprintf(uniformName, 128, "pointLights[%d].diffuse", j + 1);
            shader.setVec3(uniformName, 0.8f, 0.8f, 0.8f);
        }
        else {
            std::snprintf(uniformName, 128, "pointLights[%d].ambient", j + 1);
            shader.setVec3(uniformName, 0.0f, 0.0f, 0.0f);
            std::snprintf(uniformName, 128, "pointLights[%d].diffuse", j + 1);
            shader.setVec3(uniformName, 0.0f, 0.0f, 0.0f);
        }

        std::snprintf(uniformName, 128, "pointLights[%d].specular", j + 1);
        shader.setVec3(uniformName, 1.0f, 1.0f, 1.0f);
        std::snprintf(uniformName, 128, "pointLights[%d].constant", j + 1);
        shader.setFloat(uniformName, 1.0f);
        std::snprintf(uniformName, 128, "pointLights[%d].linear", j + 1);
        shader.setFloat(uniformName, 0.09f);
        std::snprintf(uniformName, 128, "pointLights[%d].quadratic", j + 1);
        shader.setFloat(uniformName, 0.032f);
    }

    // Spotlight
    shader.setVec3("spotLight.position", camera.position);
    shader.setVec3("spotLight.direction", camera.forward);
    shader.setVec3("spotLight.ambient", 0.0f, 0.0f, 0.0f);
    shader.setVec3("spotLight.diffuse", 1.0f, 1.0f, 1.0f);
    shader.setVec3("spotLight.specular", 1.0f, 1.0f, 1.0f);
    shader.setFloat("spotLight.constant", 1.0f);
    shader.setFloat("spotLight.linear", 0.09f);
    shader.setFloat("spotLight.quadratic", 0.005f);
    shader.setFloat("spotLight.cutOff", glm::cos(glm::radians(7.5f)));
    shader.setFloat("spotLight.outerCutOff", glm::cos(glm::radians(10.5f)));
    shader.setInt("spotlight", spotlight);
}

// 绘制场景
void paintGL() {
    glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // 设置变换矩阵
    glm::mat4 projection = glm::perspective(glm::radians(camera.zoom), SCR_WIDTH / SCR_HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = camera.GetViewMatrix();
    glm::mat4 model = glm::mat4(1.0f);

    // 设置着色器矩阵
    shader.use();
    shader.setMat4("projection", projection);
    shader.setMat4("view", view);
    shader.setMat4("model", model);

    light_shader.use();
    light_shader.setMat4("projection", projection);
    light_shader.setMat4("view", view);

    // 设置光照参数
    shader.use();
    setupLighting();

    // 1. 绘制天空盒
    glDepthFunc(GL_LEQUAL);
    sky_shader.use();
    sky_shader.setMat4("projection", projection);
    glm::mat4 sky_view = glm::mat4(glm::mat3(camera.GetViewMatrix()));
    sky_shader.setMat4("view", sky_view);
    sky_texture.bind(0);
    sky_shader.setInt("sky", 0);
    glBindVertexArray(sky_vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);
    sky_texture.unbind();

    // 2. 更新并绘制光源
    float a = 50.0f;
    float b = 20.0f;
    float speed = 0.2f;
    float t = glfwGetTime() * speed;
    float lightY = (sin(t) < 0) ? -b * sin(t) : b * sin(t);
    float lightX = a * cos(t);
    float lightZ = 0.0f;

    lightPos = glm::vec3(lightX, lightY, lightZ);

    if (day) DirLight_shininess = glm::vec3(2.0f);
    else DirLight_shininess = glm::vec3(0.5f);

    light_shader.use();
    model = glm::mat4(1.0f);
    model = glm::translate(model, lightPos);
    light_shader.setMat4("model", model);

    // 3. 绘制灯柱光源
    for (int j = 0; j < 4; j++) {
        model = glm::mat4(1.0f);
        model = glm::translate(model, lighting_pole_pos[j]);
        model = glm::scale(model, glm::vec3(0.2f));
        light_shader.setMat4("model", model);
        glBindVertexArray(VAO[0]);
        if (light_pole_switch) {
            glDrawElements(GL_TRIANGLES, X_SEGMENTS * Y_SEGMENTS * 6, GL_UNSIGNED_INT, 0);
        }
        glBindVertexArray(0);
    }

    // 4. 绘制球场
    shader.use();
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.5f, 0.0f));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(33.0f));
    shader.setMat4("model", model);
    NBA[court_choice].bind(0);
    shader.setInt("texture_diffuse0", 0);
    shader.setInt("texture_specular0", 0);
    glBindVertexArray(VAO[1]);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    NBA[court_choice].unbind();
}

// 绘制模型场景
void drawModels(Model& model1, Model& model2, Model& model3, Model& model4,
    Model& model5, Model& model6, Model& model7, Model& model8, Model& model9, Model& model10, Model& model11) {
    glm::mat4 model;

    // 绘制企鹅
    shader.use();
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 0.5f, 0.0f));
    model = glm::translate(model, glm::vec3(penguin_x, penguin_z, penguin_y));
    model = glm::rotate(model, glm::radians(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.8f));
    shader.setMat4("model", model);
    //设置企鹅纹理
    if (penguin_index == 0) model2.Draw(shader);
    else model3.Draw(shader);

    // 绘制地面
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(0.0f, 5.0f, 0.0f));
    model = glm::scale(model, glm::vec3(5.0));
    shader.setMat4("model", model);
    texture_snow[snow_index].bind(0);
    shader.setInt("texture_diffuse0", 0);
    shader.setInt("texture_specular0", 0);
    model1.Draw(shader);
    texture_snow[snow_index].unbind();

    // 绘制球场
    model = glm::mat4(1.0f);
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(0.001f));
    shader.setMat4("model", model);
    texture[1].bind(0);
    shader.setInt("texture_diffuse0", 0);
    shader.setInt("texture_specular0", 0);
    model4.Draw(shader);
    texture[1].unbind();

    // 绘制灯柱
    for (int j = 0; j < 4; j++) {
        model = glm::mat4(1.0f);
        model = glm::translate(model, light_pole_pos[j]);
        model = glm::rotate(model, light_pole_angle[j], glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::scale(model, glm::vec3(0.4f));
        shader.setMat4("model", model);
        model5.Draw(shader);
    }

    // 根据选择绘制月亮/太阳
    if (day) {
        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        model = glm::scale(model, glm::vec3(0.005f));
        shader.setMat4("model", model);
        model7.Draw(shader);
    }
    else {
        model = glm::mat4(1.0f);
        model = glm::translate(model, lightPos);
        model = glm::scale(model, glm::vec3(0.005f));
        shader.setMat4("model", model);
        model6.Draw(shader);
    }

    // 绘制马里奥雕像
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(4.0f, 6.0f, -20.0f));
    model = glm::scale(model, glm::vec3(0.1f));
    shader.setMat4("model", model);
    model8.Draw(shader);
    // 绘制皮卡丘
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-4.0f, 1.0f, -20.0f));;
    model = glm::rotate(model, glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::scale(model, glm::vec3(2.0f));
    shader.setMat4("model", model);
    model9.Draw(shader);

    //绘制坦克
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-0.0f, 1.0f, -5.0f));
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); 
    model = glm::scale(model, glm::vec3(1.5f));
    shader.setMat4("model", model);
    model10.Draw(shader);

    //绘制航舰
    model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(-4.0f, 1.0f, -50.0f));;
    model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(2.0f));
    shader.setMat4("model", model);
    model11.Draw(shader);
}

// 窗口大小回调
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

// 鼠标按钮回调
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (!sp && button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        spotlight = -spotlight;
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        camera.position = glm::vec3(6 * sin(glm::radians(angle)), 2.5f, 6 * cos(glm::radians(angle))) + glm::vec3(penguin_x, penguin_z, penguin_y);
        camera_move = !camera_move;
        sp = !sp;
    }
}

// 鼠标移动回调
void cursor_position_callback(GLFWwindow* window, double x, double y) {
    if (camera_move) {
        float x_offset = x - last_x;
        float y_offset = last_y - y;
        last_x = x;
        last_y = y;

        float sensitivity = 0.2;
        x_offset *= sensitivity;
        y_offset *= sensitivity;
        camera.Rotate(glm::vec3(x_offset, y_offset, 0));
    }
}

// 滚轮回调
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    camera.Zoom(yoffset);
}

// 处理输入
void ProcessInput(GLFWwindow* window) {
    float speed = camera.mouse_speed * delta_time;
    // 移动
    if (!sp) {
        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
            camera.Move(camera.forward * speed);
        if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS)
            camera.Move(-camera.forward * speed);
        if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS)
            camera.Move(-camera.right * speed);
        if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)
            camera.Move(camera.right * speed);
    }
}

// 键盘回调
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
    //光照控制
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        DirLight_shininess2 += 0.2;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
        DirLight_shininess2 = max(0.1f, DirLight_shininess2 - 0.2f);
    }

  // 地面和企鹅纹理选择 - 修改为单键循环切换
    if (glfwGetKey(window, GLFW_KEY_3) == GLFW_PRESS) {
        snow_index = 0;
    }
    if (glfwGetKey(window, GLFW_KEY_4) == GLFW_PRESS) {
        snow_index = 1;
    }
    if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS) {
        penguin_index = 0;
    }
    if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS) {
        penguin_index = 1;
    }
    //切换球场纹理
    if (glfwGetKey(window, GLFW_KEY_5) == GLFW_PRESS) {
        court_choice = (court_choice + 1) % court.size();
    }
    //球场路灯开关控制
    if (glfwGetKey(window, GLFW_KEY_6) == GLFW_PRESS) {
        light_pole_switch = !light_pole_switch;
    }
    //白天和黑夜模式切换（注：白天和黑夜分别有太阳模型和月球模型在上空移动，用于模拟太阳东升西落）
    if (glfwGetKey(window, GLFW_KEY_7) == GLFW_PRESS) {
        day = !day;
    }
    // 企鹅移动
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        delta = 0.5f;
        penguin_x += delta * sin(glm::radians(angle));
        penguin_y += delta * cos(glm::radians(angle));
        if (sp) {
            camera.position = glm::vec3(6 * sin(glm::radians(angle)), 2.5f, 6 * cos(glm::radians(angle))) +
                glm::vec3(penguin_x, penguin_z, penguin_y);
        }
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        delta = -0.5f;
        penguin_x += delta * sin(glm::radians(angle));
        penguin_y += delta * cos(glm::radians(angle));
        if (sp) {
            camera.position = glm::vec3(6 * sin(glm::radians(angle)), 2.5f, 6 * cos(glm::radians(angle))) +
                glm::vec3(penguin_x, penguin_z, penguin_y);
        }
    }

    // 企鹅旋转
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
        angle += 90.0f;
        if (sp) {
            camera.position = glm::vec3(6 * sin(glm::radians(angle)), 2.5f, 6 * cos(glm::radians(angle))) +
                glm::vec3(penguin_x, penguin_z, penguin_y);
            camera.UpdateCameraVectors(glm::vec3(0.0f, angle, 0.0f));
        }
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
        angle -= 90.0f;
        if (sp) {
            camera.position = glm::vec3(6 * sin(glm::radians(angle)), 2.5f, 6 * cos(glm::radians(angle))) +
                glm::vec3(penguin_x, penguin_z, penguin_y);
            camera.UpdateCameraVectors(glm::vec3(0.0f, angle, 0.0f));
        }
    }
}

int main(int argc, char* argv[]) {
    // 初始化GLFW
    if (!glfwInit()) {
        std::cout << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    // 配置GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // 创建窗口
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Assignment 2", NULL, NULL);
    if (!window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // 初始化OpenGL
    initializedGL();

    // 跳跃动画变量
    double jump_start_time = -1.0;
    double jump_end_time = -1.0;
    double jump_duration = 1.0;

    // 加载模型
    Model model1("./resources/model/snow/snow.obj");
    Model model2("./resources/model/penguin1/penguin.obj");
    Model model3("./resources/model/penguin2/penguin.obj");
    Model model4("./resources/model/Court with hoops/Court with hoops.obj");
    Model model5("./resources/model/Street Lamp/StreetLamp.obj");
    Model model6("./resources/model/moon/Moon.obj");
    Model model7("./resources/model/moon2/Moon.obj");
    Model model8("./resources/model/Mario/mario-sculpture.obj");
    Model model9("./resources/model/Pikachu/Pikachu.obj");
    Model model10("./resources/model/car/car.obj");
	Model model11("./resources/model/obj/obj.obj");

    // 主循环
// 修改主循环中的跳跃处理逻辑
    while (!glfwWindowShouldClose(window)) {
        // 计算每帧时间
        float current_frame = glfwGetTime();
        delta_time = current_frame - last_frame;
        last_frame = current_frame;
        ProcessInput(window);// 处理输入
        paintGL();// 渲染场景

        // 更新相机位置（第三人称视角）- 只跟随水平位置
        if (sp) {
            camera.position = glm::vec3(6 * sin(glm::radians(angle)), 2.5f, 6 * cos(glm::radians(angle))) + glm::vec3(penguin_x, 0.0f, penguin_y);
        }

        // 处理跳跃
        if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS && jump_start_time < 0.0) {
            jump_start_time = glfwGetTime();
            jump_end_time = jump_start_time + jump_duration;
        }

        // 更新跳跃状态 - 只更新企鹅的高度，不改变摄像机高度
        double current_time = glfwGetTime();
        if (current_time >= jump_start_time && current_time <= jump_end_time) {
            float jump_progress = (current_time - jump_start_time) / jump_duration;
            float jump_height = 1.0f;
            penguin_z = jump_height * sin(jump_progress * PI);
            // 不更新摄像机的位置，保持摄像机固定高度
        }
        else {
            jump_start_time = -1.0;
            jump_end_time = -1.0;
            penguin_z = 0.0f; // 确保跳跃结束后 penguin_z 归零
        }

        // 绘制模型
        drawModels(model1, model2, model3, model4, model5, model6, model7, model8, model9, model10, model11);

        // 交换缓冲区
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
