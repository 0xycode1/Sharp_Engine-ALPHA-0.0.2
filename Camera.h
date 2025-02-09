#pragma once

#ifndef CAMERA_H
#define CAMERA_H

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <assimp/Importer.hpp>      
#include <assimp/scene.h>         
#include <assimp/postprocess.h>

#include <iostream>
#include <vector>
#include <cmath>

const int SCREEN_WIDTH = 1280;
const int SCREEN_HEIGHT = 720;

glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

float deltaTime = 0.0f;
float lastFrame = 0.0f;


bool keys[1024] = { false };

unsigned int vao, shader_program;
int modelLoc, viewLoc, projLoc;

float yaw = -89.0f; // oY
float pitch = 0.0f; // oX
float lastX = SCREEN_WIDTH / 2.0f;
float lastY = SCREEN_HEIGHT / 2.0f;
bool firstMouse = true;

#endif // Camera.h