#pragma once

#include <string>
#include <iostream>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/vec3.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_inverse.hpp>

#include "RenderSystem.h"
#include "InputSystem.h"
#include "Renderable.h"
#include "Shader.h"
#include "Texture.h"
#include "Camera.h"
#include "Transform.h"

const int WIDTH = 1280;
const int HEIGHT = 720;
const bool WINDOWED = true;
const std::string WINDOW_TITLE = "ModelLoading";

const std::string CHALET_MODEL_PATH = "Resources/Meshes/chalet.mesh";
const std::string CHALET_TEXTURE_PATH = "Resources/Textures/chalet.jpg";

const std::string VERT_SHADER_PATH = "Resources/Shaders/textureMapping_vert.spv";
const std::string FRAG_SHADER_PATH = "Resources/Shaders/textureMapping_frag.spv";

//controls speeds
const float cCamTranslateSpeed = 20.0f;
const float cCamRotateSpeed = 100.0f;

class VkApp
{
private:
	GLFWwindow* mWindow;
	RenderSystem mRenderSystem;
	InputSystem mInputSystem;

	float mTime = 0.0f;
	float mPrevTime = 0.0f;
	float mFrameTime = 0.0f;

	std::unique_ptr<Camera> mCamera;

	std::shared_ptr<Renderable> mTestModel;
	std::shared_ptr<UBO> mTestModelMVPBuffer;
	Transform mTestModelXform;

	const float mModelTranslateSpeed = 5.0f;
	const float mModelRotateSpeed = 5.0f;

public:
	VkApp();
	void run();

private:
	void initialize(const std::string& appName);
	void shutdown();
	void createWindow();
	void handleInput();
	void cameraControls();
	void setupCamera();
	void createTestModel();
	void updateMVPBuffer(const UBO& mvpBuffer, const Transform& renderableXform, const Camera& cam);
};
