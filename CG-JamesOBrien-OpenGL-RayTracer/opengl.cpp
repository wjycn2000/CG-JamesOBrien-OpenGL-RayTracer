#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/shader_s.h>
#include <learnopengl/CameraFPS.h>
#include "model.h"
#include "raytrace.h"
#include <Windows.h>

#include <iostream>

const float MAX_FPS = 60.0f;

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

// screen settings
const unsigned int SCR_WIDTH = 512;
const unsigned int SCR_HEIGHT = 512;


//Mouses
float lastX = SCR_WIDTH/2, lastY = SCR_HEIGHT/2;
bool firstMouse = true;

//timing
float deltaTime = 0.0f; // 当前帧与上一帧的时间差
float lastFrame = 0.0f; // 上一帧的时间


glm::vec3 eye(0.0f, 0.5f, 8.0f);
glm::vec3 at(0.0f, 0.0f, 0.0f);
glm::vec3 up(0.0f, 1.0f, 0.0f);

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
     
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OpenGL Realtime Ray Tracing", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    
    float canvas[] = {
        -1, -1, 0,
        1, -1, 0,
        -1, 1, 0,
        1, 1, 0,
        -1, 1, 0,
        1, -1, 0
    };

    Material m;
    Model model("D:\\3DResources\\scene1.obj", m);
    vector<Triangle> ts;
    model.genTriangles(ts);

    glm::vec3 bcolor(0.5f, 0.5f, 0.5f);
    glm::vec3 light1(-10, 10, 10);
    glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
    glm::vec3 color = m.color;
    float shadowbias = 0.00001f;
    float angle = 60.0f;
    float hither = 1.0f;


    //buffer data
    unsigned int VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(canvas), canvas, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    GLuint tbo;
    glGenBuffers(1, &tbo);
    glBindBuffer(GL_TEXTURE_BUFFER, tbo);
    glBufferData(GL_TEXTURE_BUFFER, ts.size() * sizeof(Triangle), &ts[0], GL_STATIC_DRAW);
    GLuint tex;
    glGenTextures(1, &tex);
    

    // build and compile shader program
    Shader shader("D:\\OpenGL\\shaders\\raytracing.vs", "D:\\OpenGL\\shaders\\raytracing.fs");
    shader.use();

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_BUFFER, tex);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, tbo);
    

    

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // per-frame time logic
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        double diff = 1.0 / MAX_FPS - deltaTime;

        if (diff > 0) {
            Sleep(diff * 1000);
        }
        
        
        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        

        shader.use();
        shader.setFloat3("material1", color.x, color.y, color.z);
        shader.setFloat3("material2", m.kd, m.ks, m.shine);
        shader.setFloat3("material3", m.km, m.t, m.ior);
        shader.setInt2("resolution", SCR_WIDTH, SCR_HEIGHT);
        shader.setFloat3("eye", eye.x, eye.y, eye.z);
        shader.setFloat3("up", up.x, up.y, up.z);
        shader.setFloat3("at", at.x, at.y, at.z);
        shader.setFloat("angle", angle);
        shader.setFloat("shadowbias", shadowbias);
        shader.setFloat("hither", hither);
        shader.setFloat3("bcolor", bcolor.x, bcolor.y, bcolor.z);
        shader.setInt("triangleNum", ts.size());
        shader.setFloat3("light1", light1.x, light1.y, light1.z);
        shader.setFloat3("lightColor", lightColor.x, lightColor.y, lightColor.z);
        
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6);


        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glBindVertexArray(0);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
        eye.y += 0.1f;
    }
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        eye.y -= 0.1f;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        eye.x -= 0.1f;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        eye.x += 0.1f;
}   
    

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
