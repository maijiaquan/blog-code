#include "imgui.h"
#include "imgui_impl_glfw_gl3.h"
#include <stdio.h>
#include <GL/glew.h> //  使用gl3w，glad也行，注意要在项目工程中添加gl3w.c（或者glad.c/使用glad）
#include <GLFW/glfw3.h>
GLFWwindow *window;

#include <iostream>
#include <common/shader.hpp>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <common/texture.hpp>
#include <common/objloader.hpp>
// #include <common/controls.hpp>

glm::mat4 viewMouseMatrix;
glm::mat4 GetView();


// Initial position : on +Z
glm::vec3 position = glm::vec3(0, 0, 5);
// Initial horizontal angle : toward -Z
float horizontalAngle = 3.14f;
// Initial vertical angle : none
float verticalAngle = 0.0f;
// Initial Field of View
float initialFoV = 45.0f;

float speed = 3.0f; // 3 units / second
float mouseSpeed = 0.005f;

bool showMonitor = true;
bool canMouseRotate = false;
bool canSelfRotate = false;
bool canRotate45 = false;

void ShowMonitor(bool *p_open);
void ShowMonitor(bool *p_open)
{
    const float DISTANCE = 10.0f;
    static int corner = 0;
    ImVec2 window_pos = ImVec2((corner & 1) ? ImGui::GetIO().DisplaySize.x - DISTANCE : DISTANCE, (corner & 2) ? ImGui::GetIO().DisplaySize.y - DISTANCE : DISTANCE);
    ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
    if (corner != -1)
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
    ImGui::SetNextWindowBgAlpha(0.3f); // Transparent background
    if (ImGui::Begin("Example: Simple Overlay", p_open, (corner != -1 ? ImGuiWindowFlags_NoMove : 0) | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav))
    {
        ImGui::Text("Simple overlay\n"
                    "in the corner of the screen.\n"
                    "(right-click to change position)");
        ImGui::Separator();
        ImGui::Text("Angle : (%.1f,%.1f)", horizontalAngle, verticalAngle);

        if (ImGui::IsMousePosValid())
        {
            ImGui::Text("Mouse Position: (%.1f,%.1f)", ImGui::GetIO().MousePos.x, ImGui::GetIO().MousePos.y);
        }
        else
            ImGui::Text("Mouse Position: <invalid>");
        if (ImGui::BeginPopupContextWindow())
        {
            if (ImGui::MenuItem("Custom", NULL, corner == -1))
                corner = -1;
            if (ImGui::MenuItem("Top-left", NULL, corner == 0))
                corner = 0;
            if (ImGui::MenuItem("Top-right", NULL, corner == 1))
                corner = 1;
            if (ImGui::MenuItem("Bottom-left", NULL, corner == 2))
                corner = 2;
            if (ImGui::MenuItem("Bottom-right", NULL, corner == 3))
                corner = 3;
            if (p_open && ImGui::MenuItem("Close"))
                *p_open = false;
            ImGui::EndPopup();
        }
        ImGui::End();
    }
}

glm::mat4 GetView()
{
    static double lastTime = glfwGetTime();
    double currentTime = glfwGetTime();
    float deltaTime = float(currentTime - lastTime);

    // Get mouse position
    if (canMouseRotate)
    {
        horizontalAngle -= mouseSpeed * ImGui::GetIO().MouseDelta.x;
        verticalAngle -= mouseSpeed * ImGui::GetIO().MouseDelta.y;
    }

    // Direction : Spherical coordinates to Cartesian coordinates conversion
    glm::vec3 direction(
        cos(verticalAngle) * sin(horizontalAngle),
        sin(verticalAngle),
        cos(verticalAngle) * cos(horizontalAngle));

    // Right vector
    glm::vec3 right = glm::vec3(
        sin(horizontalAngle - 3.14f / 2.0f),
        0,
        cos(horizontalAngle - 3.14f / 2.0f));

    // Up vector
    glm::vec3 up = glm::cross(right, direction);

    // Move forward
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        position += direction * deltaTime * speed;
    }
    // Move backward
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        position -= direction * deltaTime * speed;
    }
    // Strafe right
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        position += right * deltaTime * speed;
    }
    // Strafe left
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        position -= right * deltaTime * speed;
    }

    float FoV = initialFoV ; 
    
    glm::mat4 ViewMatrix = glm::lookAt(
        position,             // Camera is here
        position + direction, // and looks here : at the same position, plus "direction"
        up                    // Head is up (set to 0,-1,0 to look upside-down)
    );

    lastTime = currentTime;
    return ViewMatrix;
}

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
    // GLFWwindow *window = glfwCreateWindow(Window_width, Window_height, "ImGui Triangle", NULL, NULL);
    window = glfwCreateWindow(Window_width, Window_height, "ImGui Triangle", NULL, NULL);
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

    // Load the texture using any two methods
    GLuint Texture = loadBMP_custom("uvtemplate.bmp");
    // GLuint Texture = loadDDS("uvtemplate.DDS");

    // Get a handle for our "myTextureSampler" uniform
    GLuint TextureID = glGetUniformLocation(programID, "myTextureSampler");

    // Read our .obj file
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals; // Won't be used at the moment.
    bool res = loadOBJ("cube.obj", vertices, uvs, normals);

    //定义顶点缓冲，并将顶点缓冲传给OpenGL
    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    // 把我们的顶点数组复制到一个顶点缓冲中，供OpenGL使用
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);

    GLuint uvbuffer;
    glGenBuffers(1, &uvbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
    //glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data), g_uv_buffer_data, GL_STATIC_DRAW);
    glBufferData(GL_ARRAY_BUFFER, uvs.size() * sizeof(glm::vec2), &uvs[0], GL_STATIC_DRAW);


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
        glm::mat4 RotationMatrix45 = canRotate45 ? glm::rotate((float)3.14f / 4, glm::vec3(1, 0, 0)) : glm::mat4(1.0);
        glm::mat4 RotationMatrix = canSelfRotate ? glm::rotate((float)currentTime, rotationAxis) : glm::mat4(1.0);

        //-----
        // Compute the MVP matrix from keyboard and mouse input
        // computeMatricesFromInputs();
        //glm::mat4 ProjectionMatrix = getProjectionMatrix();
        //glm::mat4 ViewMatrix = getViewMatrix();
        glm::mat4 ModelMatrix = glm::mat4(1.0);
        // glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;
        //----

        // Camera matrix
        glm::mat4 View = glm::lookAt(pos, lookAtPos, up);
        // View = View * RotationMatrix * RotationMatrix45;

        glm::mat4 Model = glm::mat4(1.0f);
        Model = View * RotationMatrix * RotationMatrix45;
        // glm::mat4 MVP = Projection * View * Model;
        glm::mat4 ViewMatrix = GetView();

        glm::mat4 MVP = Projection * ViewMatrix * Model;

        // 创建ImGui
        glfwPollEvents();
        ImGui_ImplGlfwGL3_NewFrame();
        ImGui::Begin("Panel", &ImGui, ImGuiWindowFlags_MenuBar);
        ShowMonitor(&showMonitor);
        ImGui::RadioButton("perspective camera", &isOrthoCamera, 0);
        ImGui::SameLine();
        ImGui::RadioButton("ortho camera", &isOrthoCamera, 1);

        //UI组件
        ImGui::Checkbox("Enable Mouse Rotate", &canMouseRotate);
        ImGui::Checkbox("Enable Cube Rotate", &canSelfRotate);
        ImGui::Checkbox("Enable Rotate 45", &canRotate45);
        ImGui::SliderFloat("viewField", &viewField, 0.0f, 90.0f, "viewField = %.3f");
        ImGui::SliderFloat("radius", &radius, 0.0f, 20.0f, "radius = %.3f");
        ImGui::SliderFloat("horizontalAngle", &horizontalAngle, -10.0f, 10.0f, "horizontalAngle = %.3f");
        ImGui::SliderFloat("verticalAngle", &verticalAngle, -10.0f, 10.0f, "verticalAngle = %.3f");

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