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

glm::mat4 viewMouseMatrix;
glm::mat4 GetView();

// Initial cameraPosition : on +Z
glm::vec3 cameraPosition = glm::vec3(-8.9, -2.5, 13.3);
// Initial horizontal angle : toward -Z
float horizontalAngle = 2.526f;
// Initial vertical angle : none
float verticalAngle = 0.15f;
// Initial Field of View
float initialFoV = 45.0f;

// float ambientMaterial = 0.3;
float shininessMaterial = 32;
// float specularMaterial = 0.5;

float speed = 3.0f; // 3 units / second
float mouseSpeed = 0.005f;

bool showMonitor = true;
bool canMouseRotate = false;
bool canSelfRotate = false;
bool canRotate45 = false;
static int item_current = 0;

enum MATERIAL
{
    EMERALD,
    PEARL,
    BRONZE,
    GOLD,
    GREEN_RUBBER,
    RED_PLASTIC
};
struct MaterialStruct
{
    ImVec4 ambient;
    ImVec4 diffuse;
    ImVec4 specular;
    float shininess;
};
MaterialStruct materialArray[6];

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
        ImGui::Separator();
        ImGui::Text("Angle : (%.1f,%.1f)", horizontalAngle, verticalAngle);
        ImGui::Text("item : (%.1f)", (float)item_current);
        ImGui::Text("Camera position: (%.1f,%.1f,%.1f)", cameraPosition.x, cameraPosition.y, cameraPosition.z);

        if (ImGui::IsMousePosValid())
        {
            ImGui::Text("Mouse position: (%.1f,%.1f)", ImGui::GetIO().MousePos.x, ImGui::GetIO().MousePos.y);
        }
        else
            ImGui::Text("Mouse position: <invalid>");
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
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
    {
        cameraPosition += up * deltaTime * speed;
    }

    // Move forward
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        cameraPosition -= up * deltaTime * speed;
    }
    // Move forward
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        cameraPosition += direction * deltaTime * speed;
    }
    // Move backward
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        cameraPosition -= direction * deltaTime * speed;
    }
    // Strafe right
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        cameraPosition += right * deltaTime * speed;
    }
    // Strafe left
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        cameraPosition -= right * deltaTime * speed;
    }

    float FoV = initialFoV;

    glm::mat4 ViewMatrix = glm::lookAt(
        cameraPosition,             // Camera is here
        cameraPosition + direction, // and looks here : at the same cameraPosition, plus "direction"
        up                          // Head is up (set to 0,-1,0 to look upside-down)
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
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

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
    float viewField = 45.0f;
    ImVec4 camPos = ImVec4(radius, 0.0f, radius, 1.00f);
    glm::vec3 lightPos(1.2f, 0.8f, 2.0f);
    ImVec4 cubeColor = ImVec4(1.0f, 0.5f, 0.31f, 1.00f);
    ImVec4 lightColor = ImVec4(1.0f, 1.0f, 1.0f, 1.00f);

    ImVec4 ambientMaterial = ImVec4(1.0f, 0.5f, 0.31f, 1.00f);
    ImVec4 diffuseMaterial = ImVec4(1.0f, 0.5f, 0.31f, 1.00f);
    ImVec4 specularMaterial = ImVec4(0.5f, 0.5f, 0.5f, 1.00f);

    enum MATERIAL
    {
        EMERALD,
        PEARL,
        BRONZE,
        GOLD,
        GREEN_RUBBER,
        RED_PLASTIC
    };

    materialArray[EMERALD].ambient = ImVec4(0.0215, 0.1745, 0.0215, 1.0);
    materialArray[EMERALD].diffuse = ImVec4(0.07568, 0.61424, 0.07568, 1.0);
    materialArray[EMERALD].specular = ImVec4(0.633, 0.727811, 0.633, 1.0);
    materialArray[EMERALD].shininess = 76.8;

    materialArray[PEARL].ambient = ImVec4(0.25, 0.20725, 0.20725, 1.0);
    materialArray[PEARL].diffuse = ImVec4(1, 0.829, 0.829, 1.0);
    materialArray[PEARL].specular = ImVec4(0.296648, 0.296648, 0.296648, 1.0);
    materialArray[PEARL].shininess = 11.26;

    materialArray[BRONZE].ambient = ImVec4(0.2125, 0.1275, 0.054, 1.0);
    materialArray[BRONZE].diffuse = ImVec4(0.714, 0.4284, 0.18144, 1.0);
    materialArray[BRONZE].specular = ImVec4(0.393548, 0.271906, 0.166721, 1.0);
    materialArray[BRONZE].shininess = 25.6;

    materialArray[GOLD].ambient = ImVec4(0.24725, 0.1995, 0.0745, 1.0);
    materialArray[GOLD].diffuse = ImVec4(0.75164, 0.60648, 0.22648, 1.0);
    materialArray[GOLD].specular = ImVec4(0.628281, 0.555802, 0.366065, 1.0);
    materialArray[GOLD].shininess = 51.2;

    materialArray[GREEN_RUBBER].ambient = ImVec4(0.0, 0.05, 0, 1.0);
    materialArray[GREEN_RUBBER].diffuse = ImVec4(0.4, 0.5, 0.4, 1.0);
    materialArray[GREEN_RUBBER].specular = ImVec4(0.04, 0.7, 0.04, 1.0);
    materialArray[GREEN_RUBBER].shininess = 10;

    materialArray[RED_PLASTIC].ambient = ImVec4(0.0, 0.0, 0.0, 1.0);
    materialArray[RED_PLASTIC].diffuse = ImVec4(0.5, 0.0, 0.0, 1.0);
    materialArray[RED_PLASTIC].specular = ImVec4(0.7, 0.6, 0.6, 1.0);
    materialArray[RED_PLASTIC].shininess = 32;

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    // 加载shader文件，创建并编译GLSL程序
    GLuint cubeProgramID = LoadShaders("basic_lighting.vs", "basic_lighting.fs");
    GLuint lampProgramID = LoadShaders("lamp.vs", "lamp.fs");

    // 读取obj文件
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;
    loadOBJ("cube.obj", vertices, uvs, normals);

    GLuint cubeVAO;
    glGenVertexArrays(1, &cubeVAO);
    glBindVertexArray(cubeVAO);

    GLuint VBO;
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), &vertices[0], GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    //add normal
    GLuint normalbuffer;
    glGenBuffers(1, &normalbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), &normals[0], GL_STATIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void *)0);
    glEnableVertexAttribArray(1);

    GLuint lightVAO;
    glGenVertexArrays(1, &lightVAO);
    glBindVertexArray(lightVAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);

    // 渲染循环
    while (!glfwWindowShouldClose(window))
    {
        double currentTime = glfwGetTime();

        glm::mat4 View = GetView();

        for (int i = 0; i < IM_ARRAYSIZE(io.KeysDown); i++)
        {
            if (ImGui::IsKeyPressed(i))
            {
                if (i == 81) //Q
                {
                    canMouseRotate = true;
                }
                if (i == 69) //E
                {
                    canMouseRotate = false;
                }
                break;
            }
        }

        glm::mat4 Projection = isOrthoCamera ? glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.0f, 10.0f) : glm::perspective(glm::radians(viewField), 4.0f / 3.0f, 0.1f, 100.0f);

        glm::vec3 rotationAxis = glm::vec3(0, 1, 0);
        glm::mat4 RotationMatrix45 = canRotate45 ? glm::rotate((float)3.14f / 4, glm::vec3(1, 0, 0)) : glm::mat4(1.0);
        glm::mat4 RotationMatrix = canSelfRotate ? glm::rotate((float)currentTime, rotationAxis) : glm::mat4(1.0);
        glm::mat4 Model = glm::mat4(1.0f);
        Model = Model * RotationMatrix * RotationMatrix45;

        glm::mat4 MVP = Projection * View * Model;

        // 创建ImGui
        glfwPollEvents();
        ImGui_ImplGlfwGL3_NewFrame();
        ImGui::Begin("Panel", &ImGui, ImGuiWindowFlags_MenuBar);

        //UI组件
        ShowMonitor(&showMonitor);
        ImGui::RadioButton("perspective camera", &isOrthoCamera, 0);
        ImGui::SameLine();
        ImGui::RadioButton("ortho camera", &isOrthoCamera, 1);
        ImGui::Checkbox("Enable Mouse Rotate", &canMouseRotate);
        ImGui::Checkbox("Enable Cube Rotate", &canSelfRotate);
        ImGui::Checkbox("Enable Rotate 45", &canRotate45);
        ImGui::SliderFloat("viewField", &viewField, 0.0f, 90.0f, "viewField = %.3f");
        ImGui::SliderFloat("radius", &radius, 0.0f, 20.0f, "radius = %.3f");
        ImGui::SliderFloat("horizontalAngle", &horizontalAngle, -10.0f, 10.0f, "horizontalAngle = %.3f");
        ImGui::SliderFloat("verticalAngle", &verticalAngle, -10.0f, 10.0f, "verticalAngle = %.3f");

        ImGui::Separator();

        // ImGui::SliderFloat("ambientMaterial", &ambientMaterial, 0.0f, 1.0f, "ambientMaterial = %.3f");
        // ImGui::SliderFloat("specularMaterial", &specularMaterial, 0.0f, 1.0f, "specularMaterial = %.3f");

        ImVec4 curr_ambient = materialArray[item_current].ambient;
        ImVec4 curr_diffuse = materialArray[item_current].diffuse;
        ImVec4 curr_specular = materialArray[item_current].specular;
        float curr_shininess = materialArray[item_current].shininess;

        ImGui::ColorEdit3("light color", (float *)&lightColor);

        ImGui::ColorEdit3("ambient", (float *)&curr_ambient);
        ImGui::ColorEdit3("diffuse", (float *)&curr_diffuse);
        ImGui::ColorEdit3("specular", (float *)&curr_specular);
        ImGui::SliderFloat("shininess", &curr_shininess, 0.0f, 256.0f, "shininessMaterial = %.3f");

        {
            const char *items[] = {
                "Emerald",
                "Pearl",
                "Bronze",
                "Gold",
                "Green rubber",
                "Red Plastic",
            };
            ImGui::Combo("Material", &item_current, items, IM_ARRAYSIZE(items));
            ImGui::SameLine();
        }
        ImGui::End();

        if (show_demo_window)
        {
            ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiCond_FirstUseEver);
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

        // glUseProgram(programID);

        //cube
        glUseProgram(cubeProgramID);
        setVec3(cubeProgramID, "objectColor", cubeColor.x, cubeColor.y, cubeColor.z);
        setVec3(cubeProgramID, "lightColor", lightColor.x, lightColor.y, lightColor.z);
        setVec3(cubeProgramID, "lightPos", lightPos.x, lightPos.y, lightPos.z);
        setVec3(cubeProgramID, "viewPos", cameraPosition.x, cameraPosition.y, cameraPosition.z);

        // setFloat(cubeProgramID, "shininessMaterial", shininessMaterial);
        // setFloat(cubeProgramID, "ambientMaterial", ambientMaterial);
        // setFloat(cubeProgramID, "specularMaterial", specularMaterial);

        setVec3(cubeProgramID, "material.ambient", curr_ambient.x, curr_ambient.y, curr_ambient.z);
        setVec3(cubeProgramID, "material.diffuse", curr_diffuse.x, curr_diffuse.y, curr_diffuse.z);
        setVec3(cubeProgramID, "material.specular", curr_specular.x, curr_specular.y, curr_specular.z);
        setFloat(cubeProgramID, "material.shininess", curr_shininess);

        setMat4(cubeProgramID, "projection", Projection);
        setMat4(cubeProgramID, "view", View);
        setMat4(cubeProgramID, "model", Model);

        glBindVertexArray(cubeVAO);
        glDrawArrays(GL_TRIANGLES, 0, 12 * 3); // 3 indices starting at 0 -> 1 triangle

        //lamp
        glUseProgram(lampProgramID);
        setMat4(lampProgramID, "projection", Projection);
        setMat4(lampProgramID, "view", View);
        Model = glm::mat4(1.0f);
        Model = glm::translate(Model, lightPos);
        Model = glm::scale(Model, glm::vec3(0.2f));
        setMat4(lampProgramID, "model", Model);

        glBindVertexArray(lightVAO);
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glfwSwapBuffers(window);
    }

    // 释放VAO、VBO、EBO资源
    glDeleteBuffers(1, &VBO);
    // glDeleteProgram(programID);
    glDeleteVertexArrays(1, &cubeVAO);
    glDeleteVertexArrays(1, &lightVAO);

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