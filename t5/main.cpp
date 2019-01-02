#include "imgui.h"
#include "imgui_impl_glfw_gl3.h"
#include <stdio.h>
#include <GL/glew.h> //  使用gl3w，glad也行，注意要在项目工程中添加gl3w.c（或者glad.c/使用glad）
#include <GLFW/glfw3.h>
#include <iostream>
#include <common/shader.hpp>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <common/texture.hpp>

void window_size_callback(GLFWwindow *window, int width, int height);

// 设置窗口大小
const unsigned int Window_width = 1600;
const unsigned int Window_height = 1200;

int main()
{
    // 实例化GLFW窗口
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //下面这条语句是为了适应苹果系统
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // 创建一个窗口对象，这个窗口对象存放了所有和窗口相关的数据，而且会被GLFW的其他函数频繁地用到。
    // 此外增加 if (window == NULL) 判断窗口是否创建成功
    GLFWwindow *window = glfwCreateWindow(Window_width, Window_height, "ImGui Triangle", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, window_size_callback);
    glfwSwapInterval(1);

    //初始化glew
    glewInit();

    //创建并绑定ImGui
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void)io;
    ImGui_ImplGlfwGL3_Init(window, true);
    ImGui::StyleColorsDark();

    //初始化各种数据
    bool ImGui = true;
    bool show_demo_window = true;
    int isOrthoCamera = 0;
    float radius = 3.0f;

    // 开启深度测试
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it closer to the camera than the former one
    glDepthFunc(GL_LESS);
    //创建一个VAO，并将它设为当前对象
    GLuint VertexArrayID;
    glGenVertexArrays(1, &VertexArrayID);
    //绑定顶点数组对象
    glBindVertexArray(VertexArrayID);

    // 加载shader文件，创建并编译GLSL程序
    GLuint programID = LoadShaders("TransformVertexShader.vertexshader", "TextureFragmentShader.fragmentshader");
    ImVec4 v1 = ImVec4(-0.25f, -0.25f, 0.0f, 1.00f);
    ImVec4 v2 = ImVec4(0.25f, -0.25f, 0.0f, 1.00f);
    ImVec4 v3 = ImVec4(0.0f, 0.25f, 0.0f, 1.00f);
    // ImVec4 camPos = ImVec4(4.0f, 3.0f, 3.0f, 1.00f);
    ImVec4 camPos = ImVec4(radius, 0.0f, radius, 1.00f);

    float viewField = 90.0f;
    //定义顶点缓冲，并将顶点缓冲传给OpenGL
    GLuint vertexbuffer;

    // Load the texture using any two methods
    GLuint Texture = loadBMP_custom("uvtemplate.bmp");
    // GLuint Texture = loadDDS("uvtemplate.DDS");

    // Get a handle for our "myTextureSampler" uniform
    GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");

    static const GLfloat g_vertex_buffer_data[] = {
        -1.0f, -1.0f, -1.0f, // triangle 1 : begin
        -1.0f, -1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f, // triangle 1 : end
        1.0f, 1.0f, -1.0f, // triangle 2 : begin
        -1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f, // triangle 2 : end
        1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, -1.0f,
        1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 1.0f};

    // Two UV coordinatesfor each vertex. They were created with Blender.
    static const GLfloat g_uv_buffer_data[] = {
        0.000059f, 1.0f - 0.000004f,
        0.000103f, 1.0f - 0.336048f,
        0.335973f, 1.0f - 0.335903f,
        1.000023f, 1.0f - 0.000013f,
        0.667979f, 1.0f - 0.335851f,
        0.999958f, 1.0f - 0.336064f,
        0.667979f, 1.0f - 0.335851f,
        0.336024f, 1.0f - 0.671877f,
        0.667969f, 1.0f - 0.671889f,
        1.000023f, 1.0f - 0.000013f,
        0.668104f, 1.0f - 0.000013f,
        0.667979f, 1.0f - 0.335851f,
        0.000059f, 1.0f - 0.000004f,
        0.335973f, 1.0f - 0.335903f,
        0.336098f, 1.0f - 0.000071f,
        0.667979f, 1.0f - 0.335851f,
        0.335973f, 1.0f - 0.335903f,
        0.336024f, 1.0f - 0.671877f,
        1.000004f, 1.0f - 0.671847f,
        0.999958f, 1.0f - 0.336064f,
        0.667979f, 1.0f - 0.335851f,
        0.668104f, 1.0f - 0.000013f,
        0.335973f, 1.0f - 0.335903f,
        0.667979f, 1.0f - 0.335851f,
        0.335973f, 1.0f - 0.335903f,
        0.668104f, 1.0f - 0.000013f,
        0.336098f, 1.0f - 0.000071f,
        0.000103f, 1.0f - 0.336048f,
        0.000004f, 1.0f - 0.671870f,
        0.336024f, 1.0f - 0.671877f,
        0.000103f, 1.0f - 0.336048f,
        0.336024f, 1.0f - 0.671877f,
        0.335973f, 1.0f - 0.335903f,
        0.667969f, 1.0f - 0.671889f,
        1.000004f, 1.0f - 0.671847f,
        0.667979f, 1.0f - 0.335851f};

    glGenBuffers(1, &vertexbuffer);
    // 把我们的顶点数组复制到一个顶点缓冲中，供OpenGL使用
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

    GLuint uvbuffer;
    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data), g_uv_buffer_data, GL_STATIC_DRAW);


    glm::mat4 RotationMatrix45 = glm::rotate((float)3.14f/4, glm::vec3(1, 0, 0));

    // 渲染循环
    while (!glfwWindowShouldClose(window))
    {
        double currentTime = glfwGetTime();


        // Projection matrix : 45?Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
        glm::mat4 Projection = isOrthoCamera ? glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.0f, 10.0f) : glm::perspective(glm::radians(viewField), 4.0f / 3.0f, 0.1f, 100.0f);

        glm::vec3 pos = glm::vec3(radius, 0, radius); // Camera is at (4,3,3), in World Space
        glm::vec3 lookAtPos = glm::vec3(0, 0, 0);
        glm::vec3 up = glm::vec3(0, 1, 0);

        glm::vec3 rotationAxis = glm::vec3(0, 1, 0);
        glm::mat4 RotationMatrix = glm::rotate((float)currentTime, rotationAxis);

        // Camera matrix
        glm::mat4 View = glm::lookAt(pos, lookAtPos, up);
        View = View * RotationMatrix * RotationMatrix45;
        glm::mat4 Model = glm::mat4(1.0f);
        glm::mat4 MVP = Projection * View  * Model; 

        // 创建ImGui
        glfwPollEvents();
        ImGui_ImplGlfwGL3_NewFrame();
        ImGui::Begin("Panel", &ImGui, ImGuiWindowFlags_MenuBar);
        ImGui::RadioButton("perspective camera", &isOrthoCamera, 0);
        ImGui::SameLine();
        ImGui::RadioButton("ortho camera", &isOrthoCamera, 1);

        ImGui::SliderFloat("viewField", &viewField, 0.0f, 90.0f, "viewField = %.3f");
        ImGui::SliderFloat("radius", &radius, 0.0f, 20.0f, "radius = %.3f");

        ImGui::End();

        if (show_demo_window)
        {
            ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver); // Normally user code doesn't need/want to call this because positions are saved in .ini file anyway. Here we just want to make the demo initial state a bit more friendly!
            ImGui::ShowDemoWindow(&show_demo_window);
        }
        // 渲染窗口颜色
        int view_width, view_height;
        glfwGetFramebufferSize(window, &view_width, &view_height);
        glViewport(0, 0, view_width, view_height);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        ImGui::Render();
        ImGui_ImplGlfwGL3_RenderDrawData(ImGui::GetDrawData());

        glUseProgram(programID);

        // Get a handle for our "MVP" uniform
        GLuint MatrixID = glGetUniformLocation(programID, "MVP");

        // Send our transformation to the currently bound shader,
        // in the "MVP" uniform
        glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

        // Bind our texture in Texture Unit 0
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, Texture);
        // Set our "myTextureSampler" sampler to use Texture Unit 0
        glUniform1i(TextureID, 0);

        // 1rst attribute buffer : vertices
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
        //设定顶点属性指针
        glVertexAttribPointer(
            0,        // attribute 0. No particular reason for 0, but must match the layout in the shader.
            3,        // size
            GL_FLOAT, // type
            GL_FALSE, // normalized?
            0,        // stride
            (void *)0 // array buffer offset
        );

        // 2nd attribute buffer : UVs
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
        glVertexAttribPointer(
            1,        // attribute. No particular reason for 1, but must match the layout in the shader.
            2,        // size : U+V => 2
            GL_FLOAT, // type
            GL_FALSE, // normalized?
            0,        // stride
            (void *)0 // array buffer offset
        );
        // 画三角形
        glDrawArrays(GL_TRIANGLES, 0, 12 * 3); // 3 indices starting at 0 -> 1 triangle

        glDisableVertexAttribArray(0);

        // 双缓冲。前缓冲保存着最终输出的图像，它会在屏幕上显示；而所有的的渲染指令都会在后缓冲上绘制。
        glfwSwapBuffers(window);
    }

    // 释放VAO、VBO、EBO资源
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &uvbuffer);
    glDeleteProgram(programID);
    glDeleteTextures(1, &Texture);
    glDeleteVertexArrays(1, &VertexArrayID);
    // 释放ImGui资源
    ImGui_ImplGlfwGL3_Shutdown();
    ImGui::DestroyContext();

    // 清除所有申请的glfw资源
    glfwTerminate();
    return 0;
}

void window_size_callback(GLFWwindow *window, int width, int height)
{
    glViewport(0, 0, width, height);
}