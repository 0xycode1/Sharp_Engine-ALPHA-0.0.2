// Sharp Engine

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
//#include <GL/freeglut.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>         
#include <assimp/postprocess.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Camera.h"
#include "ObjectRender.h"
#include "Controls.h"

#include <iostream>
#include <vector>
#include <string>
#include <cmath>

//#include <windows.h>
//#include <commdlg.h>

/* // Change SCREEN PREFERENCES in 'Camera.h'
const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;
*/



glm::vec3 modelColor(1.0f, 1.0f, 1.0f);


float modelScaleX = 1.0f;
float modelScaleY = 1.0f;
float modelScaleZ = 1.0f;


static float rotationX = 0.0f;
static float rotationY = 0.0f;
static float rotationZ = 0.0f;





int main(void) {
    std::cout << "[ Sharp Engine ]" << std::endl;


    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW." << std::endl;

        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Sharp Engine", NULL, NULL);

    if (!window) {
        std::cerr << "Failed to create GLFW window." << std::endl;
        glfwTerminate();

        return -1;
    }




    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, keyCallback);
    // mouse enable
    glfwSetCursorPosCallback(window, mouseCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);


    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD" << std::endl;

        return -1;
    }

    glEnable(GL_DEPTH_TEST);



    // Setup model preferences
    setupShader();
    //loadModel("ShadowSonic/Shadow.obj"); // Path to your object (1)
    loadModel("psychopomp/C'venash.obj");
    //loadModel("xyz/XYZ.obj");
    setupModel();



    if (vertices.empty() || indices.empty()) {
        std::cerr << "ERROR: No vertices or indices loaded." << std::endl;

        return -1;
    }

    // Getting coordinates of uniform-variables
    modelLoc = glGetUniformLocation(shader_program, "model");
    viewLoc = glGetUniformLocation(shader_program, "view");
    projLoc = glGetUniformLocation(shader_program, "projection");
    glUniform1i(glGetUniformLocation(shader_program, "ourTexture"), 0); // Associate the texture sampler with texture unit 0


    //std::cout << std::endl;
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");


    std::cout << std::endl;
    std::cout << "{TextureIDs}" << std::endl;
    std::cout << std::endl;

    //Mesh currentMesh;

    for (Mesh& mesh : meshes) {
        for (unsigned int i = 0; i < mesh.textureIds.size(); i++) { // i <= textureIds.size()
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, mesh.textureIds[i]);

            std::cout << "TextureID " << i << ": " << mesh.textureIds[i] << std::endl;
        }
    }



    std::cout << std::endl;
    std::cout << "Running..." << std::endl;



    while (!glfwWindowShouldClose(window)) {
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        processInput(window);

        // Clear buffer
        glClearColor(0.25f, 0.25f, 0.25f, 1.0f); // screen color

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(shader_program);

        ImGui_ImplGlfw_NewFrame();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui::NewFrame();





        // Model matrix (object matrix)
        glm::mat4 model = glm::mat4(1.0f);

        // Real-time render
        static float animationTime = 0.0f;
        animationTime += deltaTime;

        // Model preferences
        model = glm::scale(model, glm::vec3(modelScaleX, modelScaleY, modelScaleZ)); // model scale (X, Y, Z)

        //model = glm::rotate(model, animationTime * glm::radians(45.0f), glm::vec3(rotationX, rotationY, rotationZ)); // axis of model rotation (X, Y, Z)
        if (rotationX != 0.0f || rotationY != 0.0f || rotationZ != 0.0f) {
            model = glm::rotate(model, animationTime * glm::radians(45.0f), glm::vec3(rotationX, rotationY, rotationZ)); // axis of model rotation (X, Y, Z)
        }

        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        // View matrix
        glm::mat4 view = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        // Projection matrix
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCREEN_WIDTH / (float)SCREEN_HEIGHT, 0.1f, 100.0f);
        glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));




        for (Mesh& mesh : meshes) {
            for (size_t i = 0; i < mesh.textureIds.size(); i++) { // i <= mesh.textureIds.size()
                glActiveTexture(GL_TEXTURE0 + i);

                glBindTexture(GL_TEXTURE_2D, mesh.textureIds[i]);
                //glUniform1i(glGetUniformLocation(shader_program, "TextureIndex"), i);
                std::string samplerName = "ourTextures[" + std::to_string(i) + "]";
                glUniform1i(glGetUniformLocation(shader_program, samplerName.c_str()), i);


                //std::cout << "Texture " << i << ": ID " << mesh.textureIds[i] << std::endl;
            }

            glUniform1i(glGetUniformLocation(shader_program, "TextureIndex"), 0);

            glBindVertexArray(mesh.VAO); // (vao) //// (mesh.VAO)
            glDrawElements(GL_TRIANGLES, mesh.indices.size(), GL_UNSIGNED_INT, 0);
        }


        /*
        //for (size_t i = 0; i <= meshes[i].textureIds.size(); i++) {
        for (size_t i = 0; i < textureIds.size(); i++) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, textureIds[i]);

            //glUniform1i(glGetUniformLocation(shader_program, "TextureIndex"), i);
            glUniform1i(glGetUniformLocation(shader_program, ("ourTextures[" + std::to_string(i) + "]").c_str()), i);
        }

        //glBindVertexArray(0);

        for (Mesh& mesh : meshes) {
            //setupMesh(mesh);
            glBindVertexArray(mesh.VAO); // (vao) //// (mesh.VAO)
        }
        */

        bool drawObject = true;
        if (drawObject)
            glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);






        // settings buttons
        ImGui::Begin("Settings");
        ImGui::Text("[CHECK CODE FOR DETAILS]");
        ImGui::Text("");
        //ImGui::Checkbox("DrawObject", &drawObject);
        // rotation button
        ImGui::Text("Rotation");
        ImGui::SliderFloat("X-axis(R)", &rotationX, 0, 1, "%1.0f");
        ImGui::SliderFloat("Y-axis(R)", &rotationY, 0, 1, "%1.0f");
        ImGui::SliderFloat("Z-axis(R)", &rotationZ, 0, 1, "%1.0f");
        // size
        ImGui::Text("");
        ImGui::Text("Size");
        ImGui::SliderFloat("X-axis(S)", &modelScaleX, 0.1f, 10.0f);
        ImGui::SliderFloat("Y-axis(S)", &modelScaleY, 0.1f, 10.0f);
        ImGui::SliderFloat("Z-axis(S)", &modelScaleZ, 0.1f, 10.0f);
        // color (for opengl objects)
        ImGui::ColorEdit3("[GL]Color", glm::value_ptr(modelColor));
        // load objects
        ImGui::Text("");
        if (ImGui::Button("Load Object")) {
            setupShader();
            loadModel("quake_ranger/player.obj"); // "psychopomp/C'venash.obj" "HeatherMason/Heather.obj"  "Alice/Alice.obj"

            std::cout << std::endl;
            std::cout << "{TextureIDs}" << std::endl;
            std::cout << std::endl;

            //Mesh currentMesh;

            for (Mesh& mesh : meshes) {
                for (unsigned int i = 0; i < mesh.textureIds.size(); i++) { // i <= textureIds.size()
                    glActiveTexture(GL_TEXTURE0 + i);
                    glBindTexture(GL_TEXTURE_2D, mesh.textureIds[i]);

                    std::cout << "TextureID " << i << ": " << mesh.textureIds[i] << std::endl;
                }
            }

            setupModel();
        }
        ImGui::Text("{HAVEN`T DONE YET!}");
        ImGui::Text("");
        if (ImGui::Button("Refresh")) {
            meshes.clear();
            vertices.clear();
            indices.clear();
            textureIds.clear();

            setupShader();
            loadModel("psychopomp/C'venash.obj");
            setupModel();




            for (Mesh& mesh : meshes) {
                for (unsigned int i = 0; i < mesh.textureIds.size(); i++) { // i <= textureIds.size()
                    glActiveTexture(GL_TEXTURE0 + i);
                    glBindTexture(GL_TEXTURE_2D, mesh.textureIds[i]);

                    std::cout << "TextureID " << i << ": " << mesh.textureIds[i] << std::endl;
                }
            }
        }



        ImGui::Text("");
        if (ImGui::Button("Clear buffers")) {
            meshes.clear();
            vertices.clear();
            indices.clear();
            textureIds.clear();
        }
        ImGui::Text("");




        /*
        std::string OpenFileDialog() {
            OPENFILENAME ofn;       // structure for open object
            char szFile[260];      // buffer

            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.lpstrFilter = "Model Files\0*.obj;*.fbx;*.dae\0All Files\0*.*\0";
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = sizeof(szFile);
            ofn.lpstrTitle = "Select a model file";
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

            // dialog window
            if (GetOpenFileName(&ofn)) {
                return std::string(ofn.lpstrFile);
            }
            return "";
        }

        if (ImGui::Button("Load Object")) {
            std::string filename = OpenFileDialog();

            if (!filename.empty()) {
                loadModel(filename);
            }
        }
        */



        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());


        glfwSwapBuffers(window);
        glfwPollEvents();
    }


    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // Clear resources
    for (Mesh& mesh : meshes) {
        glDeleteVertexArrays(1, &mesh.VAO);
        glfwTerminate();
    }



    std::cout << std::endl;
    int maxTextures;
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &maxTextures);
    std::cout << "MAX textures: " << maxTextures << std::endl;



    std::cout << std::endl;
    std::cout << "End." << std::endl;

    return 0;
}