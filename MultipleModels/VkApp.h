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
#include "Lighting.h"
#include "Transform.h"

const int WIDTH = 1280;
const int HEIGHT = 720;
const bool WINDOWED = true;
const std::string WINDOW_TITLE = "MultipleModels";

const std::string CHALET_MODEL_PATH = "Resources/Meshes/chalet.mesh";
const std::string CHALET_TEXTURE_PATH = "Resources/Textures/chalet.jpg";

const std::string GROUND_MODEL_PATH = "Resources/Meshes/plane.mesh";
const std::string GROUND_TEXTURE_PATH = "Resources/Textures/grass.jpg";

const std::string TEA_MODEL_PATH = "Resources/Meshes/teapot.mesh";
const std::string TEA_TEXTURE_PATH = "Resources/Textures/teapot.jpg";

const std::string VERT_SHADER_PATH = "Resources/Shaders/textureMapping_vert.spv";
const std::string FRAG_SHADER_PATH = "Resources/Shaders/textureMapping_frag.spv";

//controls speeds
const float cCamTranslateSpeed = 20.0f;
const float cCamRotateSpeed = 100.0f;
const float cLightTranslateSpeed = 5.0f;

class VkApp
{
private:
	GLFWwindow* mWindow;
	RenderSystem mRenderSystem;
	InputSystem mInputSystem;
	float mTime = 0.0f;
	float mPrevTime = 0.0f;
	float mFrameTime = 0.0f;

	std::vector<VkClearValue> clearColors = {   {0.937f, 0.749f, 0.376f, 1.0f},
												{0.788f, 0.2f,   0.125f, 1.0f},
												{0.176f, 0.11f,  0.114f, 1.0f},
												{0.0f,   0.0f,   0.0f,   1.0f } };		
	int clearColorIndex = 0;

	std::unique_ptr<Camera> mCamera;

	//Models
	std::shared_ptr<Renderable> mTestModel;
	std::shared_ptr<UBO> mTestModelMVPBuffer;
	Transform mTestModelXform;

	std::shared_ptr<Renderable> mTestPlane;
	std::shared_ptr<UBO> mTestPlaneMVPBuffer;
	Transform mTestPlaneXform;

	std::shared_ptr<Renderable> mTeapot;
	std::shared_ptr<UBO> mTeapotMVPBuffer;
	Transform mTeapotXform;

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
	void createTestPlane();
	void createTeapot();
	void updateMVPBuffer(const UBO& mvpBuffer, const Transform& renderableXform, const Camera& cam);
};
