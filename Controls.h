#pragma once

#ifndef CONTROLS_H
#define CONTROLS_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
//#include <GL/freeglut.h>
#include <assimp/Importer.hpp>      
#include <assimp/scene.h>         
#include <assimp/postprocess.h>

#include "Camera.h"
#include "ObjectRender.h"

#include <iostream>
#include <vector>


bool rightMouseButtonPressed = false;

// controls for mouse
void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
	static float lastX = SCREEN_WIDTH / 2.0f;
	static float lastY = SCREEN_HEIGHT / 2.0f;


	ImGuiIO& io = ImGui::GetIO();
	if (rightMouseButtonPressed && !io.WantCaptureMouse) {
		float xoffset = xpos - lastX;
		float yoffset = lastY - ypos; // inverting y-offset

		const float sensitivity = 0.07f; // sensivity
		xoffset *= sensitivity;
		yoffset *= sensitivity;

		yaw += xoffset;
		pitch += yoffset;

		// clamp the pitch to avoid camera flipping
		if (pitch > 89.0f) pitch = 89.0f;
		if (pitch < -89.0f) pitch = -89.0f;

		// camera front vector
		glm::vec3 front;
		front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
		front.y = sin(glm::radians(pitch));
		front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
		cameraFront = glm::normalize(front);
	}

	lastX = xpos;
	lastY = ypos;
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
	//ImGuiIO& io = ImGui::GetIO();

	if (button == GLFW_MOUSE_BUTTON_RIGHT) {
		if (action == GLFW_PRESS) {
			rightMouseButtonPressed = true;
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED); // hiding cursor when pressed
		} else if (action == GLFW_RELEASE && ImGuiKey_MouseLeft) {
			rightMouseButtonPressed = false;
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL); // restore cursor visibility
		}
	}
}

void processInput(GLFWwindow* window) {
	float cameraSpeed = 2.5f * deltaTime;
	float cameraAcceleration = 5.6f * deltaTime;

	// default controls
	ImGuiIO& io = ImGui::GetIO();
	if (!io.WantCaptureMouse) {
		if (keys[GLFW_KEY_W]) {
			if (keys[GLFW_KEY_LEFT_SHIFT]) {
				cameraPos += cameraAcceleration * cameraFront;
			}
			else {
				cameraPos += cameraSpeed * cameraFront;
			}
		}
		if (keys[GLFW_KEY_S])
			cameraPos -= cameraSpeed * cameraFront;
		if (keys[GLFW_KEY_A])
			cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
		if (keys[GLFW_KEY_D])
			cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
		if (keys[GLFW_KEY_SPACE])
			cameraPos += cameraUp * cameraSpeed;
		if (keys[GLFW_KEY_LEFT_CONTROL])
			cameraPos -= cameraUp * cameraSpeed;
		/*
		if (keys[GLFW_KEY_LEFT_ALT])
			cameraPos -= cameraUp * cameraSpeed;
		*/

		// for arrows
		if (keys[GLFW_KEY_UP]) {
			if (keys[GLFW_KEY_RIGHT_SHIFT]) {
				cameraPos += cameraAcceleration * cameraFront;
			}
			else {
				cameraPos += cameraSpeed * cameraFront;

			}
		}
		if (keys[GLFW_KEY_DOWN])
			cameraPos -= cameraSpeed * cameraFront;
		if (keys[GLFW_KEY_LEFT])
			cameraPos -= glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
		if (keys[GLFW_KEY_RIGHT])
			cameraPos += glm::normalize(glm::cross(cameraFront, cameraUp)) * cameraSpeed;
		if (keys[GLFW_KEY_RIGHT_CONTROL])
			cameraPos -= cameraUp * cameraSpeed;
	}
 }

void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);

	if (action == GLFW_PRESS)
		keys[key] = true;
	else if (action == GLFW_RELEASE)
		keys[key] = false;
}

#endif // Controls.h