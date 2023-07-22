#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/shader_s.h>
#include <learnopengl/CameraFPS.h>
#include "scene.h"
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


glm::vec3 eye(0.0f, 0.0f, 8.0f);
glm::vec3 at(0.0f, 0.0f, 0.0f);
glm::vec3 up(0.0f, 1.0f, 0.0f);

int main()
{
    Scene scene;
    Material m1(glm::vec3(0.9f, 0.3f, 0.1f), 0.75f, 0.5f, 3.8f, 0.2f, 0.0f, 0.0f);
    Material m2(glm::vec3(0.9f), 0.75f, 0.5f, 3.8f, 0.2f, 0.0f, 0.0f);
    Model model1("D:\\3DResources\\chorus.obj");
    //Model model2("D:\\3DResources\\plane.obj");
    Object chorus1(&model1, m1, glm::vec3(-0.5f, 0.0f, 0.0f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
    Object chorus2(&model1, m2, glm::vec3(0.5f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f));
    //Object plane(&model1, m2, glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(2.0f));
    //Object plane2(&model2, m2, glm::vec3(0.0f, -2.0f, 0.0f), glm::vec3(90.0f, 0.0f, 0.0f), glm::vec3(2.0f));
    Light pointLight(glm::vec3(-5.0f, 5.0f, 5.0f));
    //Light pointLight2(glm::vec3(5.0f, 5.0f, 5.0f));
    scene.addObject(chorus1);
    scene.addObject(chorus2);
    //scene.addObject(plane);
    //scene.addObject(plane2);
    scene.addLight(pointLight);
    //scene.addLight(pointLight2);
    vector<Triangle> ttt;
    vector<Object_encoded> ooo;
    vector<Light> lll;
    vector<BVHnode> tree;
    vector<BVHnode_encoded> tree2;
    scene.generateData(ttt, ooo, tree, lll);
    converTree(tree, tree2);

    //int x = 0;
    //for (Triangle t : ttt) {
    //    cout << x++ << endl;
    //    cout << "p1: " << t.p1.x << " " << t.p1.y << " " << t.p1.z << endl;
    //    cout << "p2: " << t.p2.x << " " << t.p2.y << " " << t.p2.z << endl;
    //    cout << "p3: " << t.p3.x << " " << t.p3.y << " " << t.p3.z << endl;
    //}

    //int z = 0;
    //for (BVHnode_encoded node : tree2) {
    //    cout << "node " << z++ << "-----------------------------" << endl;
    //    cout << "triangle: " << node.triangles.x << " " << node.triangles.y << endl;
    //    cout << "children: " << node.children.x << " " << node.children.y << endl;
    //    cout << "AA: " << node.AA.x << " " << node.AA.y << " " << node.AA.z << endl;
    //    cout << "BB: " << node.BB.x << " " << node.BB.y << " " << node.BB.z << endl;
    //}

    int y = 0;
    for (Object_encoded o : ooo) {
        cout << "object " << y++ << "------------------------------" << endl;
        cout << "numT: " << o.numT.x << " " << o.numT.y << " " << o.numT.z << endl;
        cout << "color: " << o.color.x << " " << o.color.y << " " << o.color.z << endl;
        cout << "scale: " << o.scale.x << " " << o.scale.y << " " << o.scale.z << endl;
    }

    //for (Light l : lll) {
    //    cout << "position: " << l.position.x << " " << l.position.y << " " << l.position.z << endl;
    //    cout << "color: " << l.color.x << " " << l.color.y << " " << l.color.z << endl;
    //}

    //return 0;

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


    glm::vec3 bcolor(0.5f, 0.5f, 0.5f);
    glm::vec3 light1(-10, 10, 10);
    glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
    float shadowbias = 0.0001f;
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

    GLuint tbo_t, tbo_o, tbo_l, tbo_tree;
    glGenBuffers(1, &tbo_t);
    glBindBuffer(GL_TEXTURE_BUFFER, tbo_t);
    glBufferData(GL_TEXTURE_BUFFER, ttt.size() * sizeof(Triangle), &ttt[0], GL_STATIC_DRAW);
    glGenBuffers(1, &tbo_o);
    glBindBuffer(GL_TEXTURE_BUFFER, tbo_o);
    glBufferData(GL_TEXTURE_BUFFER, ooo.size() * sizeof(Object_encoded), &ooo[0], GL_STATIC_DRAW);
    glGenBuffers(1, &tbo_l);
    glBindBuffer(GL_TEXTURE_BUFFER, tbo_l);
    glBufferData(GL_TEXTURE_BUFFER, lll.size() * sizeof(Light), &lll[0], GL_STATIC_DRAW);
    glGenBuffers(1, &tbo_tree);
    glBindBuffer(GL_TEXTURE_BUFFER, tbo_tree);
    glBufferData(GL_TEXTURE_BUFFER, tree2.size() * sizeof(BVHnode_encoded), &tree2[0], GL_STATIC_DRAW);

    GLuint tex_t, tex_o, tex_l, tex_tree;
    glGenTextures(1, &tex_t);
    glGenTextures(1, &tex_o);
    glGenTextures(1, &tex_l);
    glGenTextures(1, &tex_tree);
    

    // build and compile shader program
    Shader shader("raytracing.vert", "raytracing.frag");
    shader.use();
    shader.setInt("triangles", 0);
    shader.setInt("objects", 1);
    shader.setInt("lights", 2);
    shader.setInt("trees", 3);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_BUFFER, tex_t);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, tbo_t);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_BUFFER, tex_o);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, tbo_o);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_BUFFER, tex_l);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, tbo_l);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_BUFFER, tex_tree);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGB32F, tbo_tree);
    

    

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
        shader.setInt2("resolution", SCR_WIDTH, SCR_HEIGHT);
        shader.setFloat3("eye", eye.x, eye.y, eye.z);
        shader.setFloat3("up", up.x, up.y, up.z);
        shader.setFloat3("at", at.x, at.y, at.z);
        shader.setFloat("angle", angle);
        shader.setFloat("shadowbias", shadowbias);
        shader.setFloat("hither", hither);
        shader.setFloat3("bcolor", bcolor.x, bcolor.y, bcolor.z);
        shader.setInt("triangleNum", ttt.size());
        shader.setInt("objectNum", ooo.size());
        shader.setInt("lightNum", lll.size());
        
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
