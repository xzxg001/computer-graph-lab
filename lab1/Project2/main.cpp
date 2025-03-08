#include "Dependencies/glew/glew.h"
#include "Dependencies/GLFW/glfw3.h"

#include "Dependencies/glm/glm.hpp"
#include "Dependencies/glm/gtc/matrix_transform.hpp"

#include <iostream>
#include <fstream>


GLuint programID;
//初始移动，缩放，旋转
float x_delta = 0.1f;
float x_press_num = 0;
float y_press_num = 0;
glm::vec3 size = glm::vec3(1.0f);//物体大小 用于进行缩放
float angle = 0.0f;//旋转角度


void get_OpenGL_info() {
    // OpenGL information
    const GLubyte* name = glGetString(GL_VENDOR);
    const GLubyte* renderer = glGetString(GL_RENDERER);
    const GLubyte* glversion = glGetString(GL_VERSION);
    std::cout << "OpenGL company: " << name << std::endl;
    std::cout << "Renderer name: " << renderer << std::endl;
    std::cout << "OpenGL version: " << glversion << std::endl;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

bool checkStatus(
    GLuint objectID,
    PFNGLGETSHADERIVPROC objectPropertyGetterFunc,
    PFNGLGETSHADERINFOLOGPROC getInfoLogFunc,
    GLenum statusType)
{
    GLint status;
    objectPropertyGetterFunc(objectID, statusType, &status);
    if (status != GL_TRUE)
    {
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

bool checkShaderStatus(GLuint shaderID) {
    return checkStatus(shaderID, glGetShaderiv, glGetShaderInfoLog, GL_COMPILE_STATUS);
}

bool checkProgramStatus(GLuint programID) {
    return checkStatus(programID, glGetProgramiv, glGetProgramInfoLog, GL_LINK_STATUS);
}

std::string readShaderCode(const char* fileName) {
    std::ifstream meInput(fileName);
    if (!meInput.good()) {
        std::cout << "File failed to load ... " << fileName << std::endl;
        exit(1);
    }
    return std::string(
        std::istreambuf_iterator<char>(meInput),
        std::istreambuf_iterator<char>()
    );
}

void installShaders() {
    GLuint vertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint fragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    const GLchar* adapter[1];
    //adapter[0] = vertexShaderCode;
    std::string temp = readShaderCode("VertexShaderCode.glsl");
    adapter[0] = temp.c_str();
    glShaderSource(vertexShaderID, 1, adapter, 0);
    //adapter[0] = fragmentShaderCode;
    temp = readShaderCode("FragmentShaderCode.glsl");
    adapter[0] = temp.c_str();
    glShaderSource(fragmentShaderID, 1, adapter, 0);

    glCompileShader(vertexShaderID);
    glCompileShader(fragmentShaderID);

    if (!checkShaderStatus(vertexShaderID) || !checkShaderStatus(fragmentShaderID))
        return;

    programID = glCreateProgram();
    glAttachShader(programID, vertexShaderID);
    glAttachShader(programID, fragmentShaderID);
    glLinkProgram(programID);

    if (!checkProgramStatus(programID))
        return;
    glUseProgram(programID);

}



void sendDataToOpenGL() {
    //const GLfloat background[] =
    //{
    //    -0.5f, -0.5f, +0.0f,
    //    +1.0f, +0.0f, +0.0f,

    //    +0.5f, -0.5f, +0.0f,
    //    +0.0f, +1.0f, +0.0f,

    //    -0.5f, +0.5f, +0.0f,
    //    +0.0f, +0.0f, +1.0f,

    //};
    //顶点数组对象及顶点缓冲对象
    GLuint vaoID;//create VAO id
    glGenVertexArrays(1, &vaoID);//create VAO
    glBindVertexArray(vaoID);  //first VAO
    //GLuint vboID;
    //glGenBuffers(1, &vboID);//create VBO id
    //glBindBuffer(GL_ARRAY_BUFFER, vboID);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(background), background, GL_STATIC_DRAW);//存储具体数据

    ////VBO将数据发送到硬件。将位置和颜色作为两个属性分别发送。
    //// 数据连接到绑定虚名
    //// vertex position
    //glEnableVertexAttribArray(0);
    //glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), 0);
    //// vertex color
    //glEnableVertexAttribArray(1);
    //glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (char*)(3 * sizeof(float)));

    const GLfloat square[] = {
        -0.5f,-0.5f,+0.0f,

        +0.5f,-0.5f,+0.0f,

        +0.5f,+0.5f,+0.0f,

        -0.5f,+0.5f,+0.0f, 
    };
    GLuint indices[] = {
        0,1,3,
        1,2,3
    };
    GLuint indexBufferID;
    glGenBuffers(1, &indexBufferID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), glGetUniformIndices, GL_STATIC_DRAW);

    //// with indexing (uncomment to use)
    //GLuint indices[] = { 0, 1, 2 };
    //// index buffer
    //GLuint indexBufferID;
    //glGenBuffers(1, &indexBufferID);
    //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
    //glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
}
void paintGL(void) {
    // always run
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);  //specify the background color
    glClear(GL_COLOR_BUFFER_BIT);

    glm::mat4 modelTransformMatrix = glm::mat4(1.0f);
    modelTransformMatrix = glm::translate(glm::mat4(1.0f),
        glm::vec3(x_delta * x_press_num, 0.0f, 0.0f));;
    GLint modelTransformMatrixUniformLocation =
        glGetUniformLocation(programID, "modelTransformMatrix");//获取指定程序对象的统一变量的位置
    glUniformMatrix4fv(modelTransformMatrixUniformLocation, 1,
        GL_FALSE, &modelTransformMatrix[0][0]);

    // glBindVertexArray();

    // without indexing
    glLineWidth(1.5f);
    glDrawArrays(GL_LINES, 0, 6);  //render primitives from array data
    //// with indexing (uncomment to use)
    //glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);
}
void initializedGL(void) {
    // run only once
    sendDataToOpenGL();//1.创建对象 2.向硬件发送数据
    installShaders();//加载着色器代码(glsl)
}
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    if (key == GLFW_KEY_A && action == GLFW_PRESS) {
        x_press_num -= 1;
    }
    if (key == GLFW_KEY_D && action == GLFW_PRESS) {
        x_press_num += 1;
    }
}

int main(int argc, char* argv[]) {
    GLFWwindow* window;

    /* Initialize the glfw */
    if (!glfwInit()) {
        std::cout << "Failed to initialize GLFW" << std::endl;
        return -1;
    }
    /* glfw: configure; necessary for MAC */
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);//set the major version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);//set the minor version
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);//set core mode

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    /* do not allow resizing */
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(1000, 1000, "Hello Triangle", NULL, NULL);// create a window
    if (!window) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();//if fail to create a window, terminate
        return -1;
    }

    //Make the window's context current 
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window, key_callback);

    /* Initialize the glew */
    if (GLEW_OK != glewInit()) {
        std::cout << "Failed to initialize GLEW" << std::endl;
        return -1;
    }
    get_OpenGL_info();
    initializedGL();

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window)) {
        /* Render here */
        paintGL();

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();
    }

    glfwTerminate();
    return 0;
}
