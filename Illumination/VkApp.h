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
const std::string WINDOW_TITLE = "Illumination";

//resource paths
const std::string BOX_DIFFUSE_PATH		= "Resources/Textures/UrbanTexturePack/Brick_OldDestroyed/Brick_OldDestroyed_1k_d.tga";
const std::string BOX_NORMAL_PATH		= "Resources/Textures/UrbanTexturePack/Brick_OldDestroyed/Brick_OldDestroyed_1k_n.tga";
const std::string BOX_SPECULAR_PATH		= "Resources/Textures/UrbanTexturePack/Brick_OldDestroyed/Brick_OldDestroyed_1k_s.tga";
const std::string BOX_MODEL_PATH		= "Resources/Meshes/cube.mesh";
const std::string BOX_VERT_SHADER_PATH  = "Resources/Shaders/multipleLights_vert.spv";
const std::string BOX_FRAG_SHADER_PATH  = "Resources/Shaders/multipleLights_frag.spv";

const std::string GROUND_DIFFUSE_PATH = "Resources/Textures/UrbanTexturePack/Ground_Dirt/Ground_Dirt_1k_d.tga";
const std::string GROUND_NORMAL_PATH = "Resources/Textures/UrbanTexturePack/Ground_Dirt/Ground_Dirt_1k_n.tga";
const std::string GROUND_SPECULAR_PATH = "Resources/Textures/UrbanTexturePack/Ground_Dirt/Ground_Dirt_1k_s.tga";
const std::string GROUND_MESH_PATH = "Resources/Meshes/ground.mesh";

//light indicator
const std::string LIGHT_MODEL_PATH		 = "Resources/Meshes/cube.mesh";
const std::string LIGHT_VERT_SHADER_PATH = "Resources/Shaders/lightObj_vert.spv";
const std::string LIGHT_FRAG_SHADER_PATH = "Resources/Shaders/lightObj_frag.spv";

//controls speeds
const float cCamTranslateSpeed = 20.0f;
const float cCamRotateSpeed = 100.0f;
const float cLightTranslateSpeed = 5.0f;

struct Material
{

	float shininess;
};
 
class VkApp
{
private:
	GLFWwindow* mWindow;
	RenderSystem mRenderSystem;
	InputSystem mInputSystem;
	float mElapsedTime = 0.0;
	float mPrevTime = 0.0;
	float mFrameTime = 0.0;

	std::vector<VkClearValue> clearColors = { {0.176f, 0.11f,  0.114f, 1.0f},
											  {0.937f, 0.749f, 0.376f, 1.0f},
											  {0.788f, 0.2f,   0.125f, 1.0f},
											  {0.0f,   0.0f,   0.0f,   1.0f} };	
	int clearColorIndex = 0;

	std::unique_ptr<Camera> mCamera;
	
	//Lights UBO
	std::shared_ptr<UBO> mLightUBOBuffer;
	LightUBO mLightUBO;

	std::shared_ptr<Renderable> mWall;
	std::shared_ptr<UBO> mWallMVPBuffer;
	Transform mWallXForm;

	std::shared_ptr<Renderable> mGround;
	std::shared_ptr<UBO> mGroundMVPBuffer;
	Transform mGroundXForm;

	std::shared_ptr<Renderable> mLightIndicators[MAX_LIGHTS];
	std::shared_ptr<UBO> mLightIndicatorMVPBuffer[MAX_LIGHTS];
	Transform mLightIndicatorXForm[MAX_LIGHTS];
	std::shared_ptr<UBO> mLightIndicatorLightBuffer[MAX_LIGHTS];
	//implied mLightUBO.lights[index]


	bool mLightOrbit = true;
	uint32_t mSelectedLight = 0;
	uint32_t mTotalLights = 0;
public:
	VkApp();
	void run();

private:
	void initialize(const std::string& appName);
	void shutdown();
	void createWindow();
	void handleInput();
	void cameraControls();
	void lightControls();

	void setupCamera();
	void setupLights();

	void createLightIndicator(uint32_t lightIndex);
	void createWall();
	void createGround();
	void updateMVPBuffer(const UBO& mvpBuffer, 
						const Renderable& renderable, 
						const Transform& renderableXform, 
						const Camera& cam);
};
