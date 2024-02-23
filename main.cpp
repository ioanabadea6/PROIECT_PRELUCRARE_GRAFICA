#if defined (__APPLE__)
#define GLFW_INCLUDE_GLCOREARB
#define GL_SILENCE_DEPRECATION
#else
#define GLEW_STATIC
#include <GL/glew.h>
#endif

#include <GLFW/glfw3.h>

#include <glm/glm.hpp> //core glm functionality
#include <glm/gtc/matrix_transform.hpp> //glm extension for generating common transformation matrices
#include <glm/gtc/matrix_inverse.hpp> //glm extension for computing inverse matrices
#include <glm/gtc/type_ptr.hpp> //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"
#include "SkyBox.hpp"
#include <iostream>

// window
gps::Window myWindow;

// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;
glm::mat4 lightRotation;

// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;

// shader uniform locations
GLint modelLoc;
GLint viewLoc;
GLint projectionLoc;
GLint normalMatrixLoc;
GLint lightDirLoc;
GLint lightColorLoc;

// camera
gps::Camera myCamera(
    glm::vec3(2.5f, 2.5f, 3.0f),
    glm::vec3(0.0f, 0.0f, -5.0f),
    glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraSpeed = 0.2f;

GLboolean pressedKeys[1024];

GLint fog;

float fogDensity = 0.0f;
bool punctiform = false;

// models
gps::Model3D scena;
gps::SkyBox mySkyBox;

GLfloat angle;
GLfloat angle2;

// shaders
gps::Shader myBasicShader;
gps::Shader lightShader;
gps::Shader depthMapShader;
gps::Shader skyboxShader;

int glWindowWidth = 1920;
int glWindowHeight = 1017;

const unsigned int SHADOW_WIDTH = 1920;
const unsigned int SHADOW_HEIGHT = 1017;

float x_curent = glWindowWidth;
float y_curent = glWindowHeight;
bool prima_miscare = true;
float yaw = -90.0f;
float pitch = 0.0f;

glm::mat4 modelAvion;
glm::mat3 normalMatrixAvion;
GLint avionLoc;
GLint normalMatrixLocAvion;
float deplasare;
gps::Model3D avion;
bool miscare_avion = false;


//street light
glm::vec3 pozitieLumina1;
glm::vec3 pozitieLumina2;
glm::vec3 culoareLumina1;
glm::vec3 culoareLumina2;

//spotlight
glm::vec3 spotLight2;
glm::vec3 target2;
float lumina = 0.0f;
GLint luminaLoc;
GLint luminLoc2;
GLint pozitieLumina1Loc;
GLint pozitieLumina2Loc;
GLint culoareLumina1Loc;
GLint culoareLumina2Loc;

//punctiform
glm::vec3 lightPunctiform1;
glm::vec3 lightPunctiform2;
GLint lightPunctiform1Loc;
GLint lightPunctiform2Loc;

GLuint shadowMapFBO;
GLuint depthMapTexture;

std::vector<const GLchar*> faces;


GLenum glCheckError_(const char* file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        std::string error;
        switch (errorCode) {
        case GL_INVALID_ENUM:
            error = "INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            error = "INVALID_VALUE";
            break;
        case GL_INVALID_OPERATION:
            error = "INVALID_OPERATION";
            break;
        case GL_OUT_OF_MEMORY:
            error = "OUT_OF_MEMORY";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            error = "INVALID_FRAMEBUFFER_OPERATION";
            break;
        }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}
#define glCheckError() glCheckError_(__FILE__, __LINE__)

void windowResizeCallback(GLFWwindow* window, int width, int height) {
    fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
    glfwGetFramebufferSize(myWindow.getWindow(), &width, &height);
    myBasicShader.useShaderProgram();
    projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 1000.0f);
    projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    glViewport(0, 0, width, height);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        }
        else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (prima_miscare) {
        x_curent = xpos;
        y_curent = ypos;
        prima_miscare = false;
    }

    float x_deplasare = xpos - x_curent;
    float y_deplasare = y_curent - ypos; 
    x_curent = xpos;
    y_curent = ypos;

    x_deplasare *= 0.15f;
    y_deplasare *= 0.15f;

    yaw += x_deplasare;
    pitch += y_deplasare;

    myCamera.rotate(pitch, yaw);
    myBasicShader.useShaderProgram();
    view = myCamera.getViewMatrix();
    viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
}

void processMovement() {
    if (pressedKeys[GLFW_KEY_W]) {
        myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for scena
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_S]) {
        myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for scena
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_A]) {
        myCamera.move(gps::MOVE_LEFT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for scena
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_D]) {
        myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
        //update view matrix
        view = myCamera.getViewMatrix();
        myBasicShader.useShaderProgram();
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
        // compute normal matrix for scena
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_Q]) {
        angle -= 1.0f;
        // update model matrix for scena
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        // update normal matrix for scena
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    if (pressedKeys[GLFW_KEY_E]) {
        angle += 1.0f;
        // update model matrix for scena
        model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0, 1, 0));
        // update normal matrix for scena
        normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    }

    //vizualizare in mod solid
    if (pressedKeys[GLFW_KEY_Z]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    //vizulaizare in mod wireframe
    if (pressedKeys[GLFW_KEY_X]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    //vizualizare in mod polygonal
    if (pressedKeys[GLFW_KEY_C]) {
        glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    }

    //vizulaizare in mod smooth
    if (pressedKeys[GLFW_KEY_V]) {
        glShadeModel(GL_SMOOTH);
    }

    //ceata
    if (pressedKeys[GLFW_KEY_1]) {
        fogDensity += 0.001f;
        myBasicShader.useShaderProgram();
        glUniform1fv(fog, 1, &fogDensity);
    }

    if (pressedKeys[GLFW_KEY_2]) {
        fogDensity -= 0.001f;
        myBasicShader.useShaderProgram();
        glUniform1fv(fog, 1, &fogDensity);
    }

    //lumina
    if (pressedKeys[GLFW_KEY_3]) {
        lumina = 1.0f;
        myBasicShader.useShaderProgram();
        glUniform1fv(luminLoc2, 1, &lumina);
    }

    if (pressedKeys[GLFW_KEY_4]) {
        lumina = 0.0f;
        myBasicShader.useShaderProgram();
        glUniform1fv(luminLoc2, 1, &lumina);
    }

}

void initOpenGLWindow() {
    myWindow.Create(1024, 768, "Proiect PG");
}

void setWindowCallbacks() {
    glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
}

void initOpenGLState() {
    glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
    glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glEnable(GL_FRAMEBUFFER_SRGB);
    glEnable(GL_DEPTH_TEST); // enable depth-testing
    glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
    glEnable(GL_CULL_FACE); // cull face
    glCullFace(GL_BACK); // cull back face
    glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}

void initModels() {
    scena.LoadModel("models/obiecte/a9.obj");
    avion.LoadModel("models/obiecte/avion5.obj");
}

void initShaders() {
    myBasicShader.loadShader(
        "shaders/basic.vert",
        "shaders/basic.frag");
    depthMapShader.loadShader(
        "shaders/depthMapShader.vert",
       "shaders/depthMapShader.frag");
    lightShader.loadShader(
        "shaders/lightCube.vert",
        "shaders/lightCube.frag");
}

void initSkybox() {
    faces.push_back("skybox/right.tga");
    faces.push_back("skybox/left.tga");
    faces.push_back("skybox/top.tga");
    faces.push_back("skybox/bottom.tga");
    faces.push_back("skybox/back.tga");
    faces.push_back("skybox/front.tga");
    mySkyBox.Load(faces);
    skyboxShader.loadShader("shaders/skyboxShader.vert", "shaders/skyboxShader.frag");
    skyboxShader.useShaderProgram();
    view = myCamera.getViewMatrix();
    viewLoc = glGetUniformLocation(skyboxShader.shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    projection = glm::perspective(glm::radians(45.0f),
        (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
        0.1f, 2000.0f);
    projectionLoc = glGetUniformLocation(skyboxShader.shaderProgram, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
}

void initUniforms() {
    myBasicShader.useShaderProgram();

    // create model matrix for scena
    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    //model = glm::scale(model, glm::vec3(0.5f, 0.5f, 0.5f));
    modelLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");

    // get view matrix for current camera
    view = myCamera.getViewMatrix();
    viewLoc = glGetUniformLocation(myBasicShader.shaderProgram, "view");
    // send view matrix to shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // compute normal matrix for scena
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    normalMatrixLoc = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

    // create projection matrix
    projection = glm::perspective(glm::radians(45.0f),
        (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
        0.1f, 2000.0f);
    projectionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "projection");
    // send projection matrix to shader
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    //set the light direction (direction towards the light)
    lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
    lightDirLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightDir");
    // send light dir to shader
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

    //set light color
    lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
    lightColorLoc = glGetUniformLocation(myBasicShader.shaderProgram, "lightColor");
    // send light color to shader
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));

    //ceata
    fog = glGetUniformLocation(myBasicShader.shaderProgram, "fogDensity");
    glUniform1fv(fog, 1, &fogDensity);

    //lumina
    spotLight2 = glm::vec3(0.0f, 0.0f, 0.0f);
    luminaLoc = glGetUniformLocation(myBasicShader.shaderProgram, "spotLight2");
    glUniform3fv(luminaLoc, 1, glm::value_ptr(spotLight2));

    target2=glm::vec3(3.0f, 0.0f, 0.0f);
    luminaLoc = glGetUniformLocation(myBasicShader.shaderProgram, "target2");
    glUniform3fv(luminaLoc, 1, glm::value_ptr(target2));  

   /* x= 4.86064f
    y= -27.0232f
    z= 4.79666f*/
    lightPunctiform1 = glm::vec3(4.86064f, 4.79666f, 27.0232f);
    lightPunctiform1Loc = glGetUniformLocation(myBasicShader.shaderProgram, "lightPunctiform1");
    glUniform3fv(lightPunctiform1Loc, 1, glm::value_ptr(lightPunctiform1));

    //x=17.2005 m
    //y= -20.6029 m
    //z= 4.57344 m
    lightPunctiform2 = glm::vec3(17.2005f, 4.57344f, 20.6029f);
    lightPunctiform2Loc = glGetUniformLocation(myBasicShader.shaderProgram, "lightPunctiform2");
    glUniform3fv(lightPunctiform1Loc, 1, glm::value_ptr(lightPunctiform1));

    luminLoc2 = glGetUniformLocation(myBasicShader.shaderProgram, "lumina");
    glUniform1fv(luminLoc2, 1, &lumina);

    //avion
    modelAvion = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
    avionLoc = glGetUniformLocation(myBasicShader.shaderProgram, "model");
    normalMatrixAvion = glm::mat3(glm::inverseTranspose(view * model));
    normalMatrixLocAvion = glGetUniformLocation(myBasicShader.shaderProgram, "normalMatrix");

    //lightShader.useShaderProgram();
    //glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
}

void initFBO() {
    glGenFramebuffers(1, &shadowMapFBO);
    glGenTextures(1, &depthMapTexture);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
        SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture,
        0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void renderscena(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();

    //send scena model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));


    //send scena normal matrix data to shader
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));

    // draw scena
    //scena.Draw(shader);
    scena.Draw(shader);
}

void animatie_avion(gps::Shader shader) {
    shader.useShaderProgram();
    deplasare -= 2.0f;
   /* x= -44.9432 m
    y= -6.30391 m
    z= 15.4871 m*/
    modelAvion = glm::translate(glm::mat4(1.0f), glm::vec3(-44.9432, -6.30391f, 15.4871f));
    modelAvion = glm::rotate(modelAvion, glm::radians(deplasare), glm::vec3(0.0f, 1.0f, 0.0f));
    modelAvion = glm::translate(modelAvion, glm::vec3(44.9432f, 6.30391f, -15.4871f));
    glUniformMatrix4fv(avionLoc, 1, GL_FALSE, glm::value_ptr(modelAvion));
    normalMatrixAvion = glm::mat3(glm::inverseTranspose(view * model));
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    avion.Draw(shader);
}

void renderScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    animatie_avion(myBasicShader);

    renderscena(myBasicShader);
    
}

void cleanup() {
    myWindow.Delete();
    //cleanup code for your own data
}

int main(int argc, const char* argv[]) {

    try {
        initOpenGLWindow();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    initOpenGLState();
    initModels();
    initShaders();
    initUniforms();
    setWindowCallbacks();

    glCheckError();
    // application loop
    glfwSetInputMode(myWindow.getWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    while (!glfwWindowShouldClose(myWindow.getWindow())) {
        processMovement();
        renderScene();

        glfwPollEvents();
        glfwSwapBuffers(myWindow.getWindow());

        glCheckError();
    }

    cleanup();

    return EXIT_SUCCESS;
}


