#include "VkApp.h"
#include "PrintUtil.h"

VkApp::VkApp() : mWindow(nullptr) {}

void VkApp::run()
{
	initialize("PlanarReflection");
	
	glfwSetTime(0.0);
	std::cout << "Starting loop..." << std::endl;
	//update loop
	while (!glfwWindowShouldClose(mWindow)) {
		glfwPollEvents();
		handleInput();

		updateReflectionMVP(*mCamera, mLightUBO.lights[0]);
		mCamera->updateViewMatrix();
		
		//light buffers
		mLightUBO.viewPos = glm::vec4(mCamera->position, 1.0);
		mRenderSystem.updateUniformBuffer<LightUBO>(*mLightUBOBuffer, mLightUBO, 0);
		
		//update transform buffers
		updateMVPBuffer(*mCubeMVPBuffer, mCubeXForm, *mCamera);
		updateMirrorMVPBuffer(*mMirrorMVPBuffer, mMirrorXForm, *mCamera);
		
		//update light indicators
		for (uint32_t lightIndex = 0; lightIndex < mTotalLights; lightIndex++) {
			mLightIndicatorXForm[lightIndex].position = mLightUBO.lights[lightIndex].position;

			updateMVPBuffer(*mLightIndicatorMVPBuffer[lightIndex], mLightIndicatorXForm[lightIndex], *mCamera);
			mRenderSystem.updateUniformBuffer<Light>(*mLightIndicatorLightBuffer[lightIndex], mLightUBO.lights[lightIndex], 0);
		}
		

		mRenderSystem.drawFrame();

		//update the frame timer
		mElapsedTime = (float)glfwGetTime();
		mFrameTime = mElapsedTime - mPrevTime;		
		mPrevTime = mElapsedTime;
	}

	std::cout << "---------------------------------" << std::endl;
	shutdown();
}

void VkApp::initialize(const std::string& appName)
{
	glfwInit();
	createWindow();
	mRenderSystem.initialize(mWindow, appName);
	mInputSystem.initialize(mWindow);
	
	setupCamera();
	setupLights();
	
	createCube();
	mCubeXForm.scale = glm::vec3(4.0f);
	mCubeXForm.position += glm::vec3(0.0f, 2.0f, 0.0f);

	createMirror();
	mMirrorXForm.scale = glm::vec3(40.0f);
	mMirrorXForm.rotation *= glm::angleAxis(glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	
	for(uint32_t lightIndex = 0; lightIndex < mTotalLights; lightIndex++)
		createLightIndicator(lightIndex);
}

void VkApp::shutdown()
{
	mRenderSystem.cleanup();
	glfwDestroyWindow(mWindow);
	glfwTerminate();
}

void VkApp::createWindow()
{ 
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	GLFWmonitor* monitor = (WINDOWED ? nullptr : glfwGetPrimaryMonitor());
	mWindow = glfwCreateWindow(WIDTH, HEIGHT, WINDOW_TITLE.c_str(), monitor, nullptr);

	if (mWindow == nullptr) throw std::runtime_error("Window creation failed!");
}

void VkApp::handleInput()
{
	mInputSystem.update();

	//close the window
	if (mInputSystem.isKeyPressed(GLFW_KEY_ESCAPE))
		glfwSetWindowShouldClose(mWindow, GLFW_TRUE);

	//change the clear color
	if (mInputSystem.isKeyPressed(GLFW_KEY_B)) {
		mRenderSystem.setClearColor(clearColors[clearColorIndex]);
		clearColorIndex++;
		if (clearColorIndex >= clearColors.size()) clearColorIndex = 0;
	}

	//print out FPS
	if (mInputSystem.isKeyPressed(GLFW_KEY_F))
		std::cout << "frameTime: " << mFrameTime * 1000.0 << " ms ( " << (1.0 / mFrameTime) << " fps)" << std::endl;

	if (mInputSystem.isKeyPressed(GLFW_KEY_L))
		mLightOrbit = !mLightOrbit;

	cameraControls();
	lightControls();
}

void VkApp::cameraControls()
{
	float transDist = cCamTranslateSpeed * mFrameTime;
	float rotAmount = glm::radians(cCamRotateSpeed * mFrameTime);

	const float cursorSensitivity = 3.5f;
	glm::vec3 deltaCursor = mInputSystem.getMouseDelta();
	glm::quat pitch = glm::angleAxis(-deltaCursor.x * cursorSensitivity * mFrameTime, mCamera->up);	//rotation is ccw around axis
	
	glm::quat yaw = glm::angleAxis(-deltaCursor.y * cursorSensitivity * mFrameTime, mCamera->right);

	mCamera->rotation *= yaw;
	mCamera->rotation = glm::normalize(mCamera->rotation);

	if (mInputSystem.isKeyDown(GLFW_KEY_W)) {
		mCamera->position += mCamera->forward * transDist;
	}
	if (mInputSystem.isKeyDown(GLFW_KEY_S)) {
		mCamera->position -= mCamera->forward * transDist;
	}
	if (mInputSystem.isKeyDown(GLFW_KEY_A)) {		
		mCamera->position -= mCamera->right * transDist;
	}
	if (mInputSystem.isKeyDown(GLFW_KEY_D)) {
		mCamera->position += mCamera->right * transDist;
	}
	if (mInputSystem.isKeyDown(GLFW_KEY_Q)) { //down
		mCamera->position -= mCamera->up * transDist;
	}
	if (mInputSystem.isKeyDown(GLFW_KEY_E)) { //up
		mCamera->position += mCamera->up * transDist;
	}


	if (mInputSystem.isKeyPressed(GLFW_KEY_C)) {
		std::cout << "cam vectors:" << std::endl;
		std::cout << "position: " << mCamera->position << std::endl;
		std::cout << "forward:  " << mCamera->forward << std::endl;
		std::cout << "right:    " << mCamera->right << std::endl;
		std::cout << "up:       " << mCamera->up << std::endl;
	}
}

void VkApp::lightControls()
{
	float orbitRadius = 10.0f;
	float orbitSpeed = 0.5f;
	float wobbleSpeed = 4.0f;
	float orbitWobble = 5.0f;
	float offConst = 0.5f;
	float transDist = cLightTranslateSpeed * mFrameTime;
	
	//in orbit mode, all of the lights orbit around the origin
	if (mLightOrbit) {
		mLightUBO.lights[0].position = glm::vec4(orbitRadius * glm::sin(orbitSpeed * mElapsedTime),
												orbitWobble * glm::sin(orbitSpeed * mElapsedTime) + 8.0f,
												orbitRadius * glm::cos(orbitSpeed * mElapsedTime), 1.0f);

		//point at the center
		mLightUBO.lights[0].direction = -glm::normalize(mLightUBO.lights[0].position);

		for (uint32_t lightIndex = 1; lightIndex < mTotalLights; lightIndex++) {
			glm::quat rot = glm::angleAxis(glm::radians(360.0f / mTotalLights * lightIndex), glm::vec3(0.0f, 1.0f, 0.0f));
			mLightUBO.lights[lightIndex].position = rot * mLightUBO.lights[0].position;
		}
	}
	else {	//if we're not in orbit mode, we can manipulate individual lights
		//move the selected light
		
		if (mInputSystem.isKeyDown(GLFW_KEY_UP)) {	//negative z is away from pov
			mLightUBO.lights[mSelectedLight].position += glm::vec4(0.0f, 0.0f, transDist, 1.0f);
		}
		if (mInputSystem.isKeyDown(GLFW_KEY_DOWN)) {
			mLightUBO.lights[mSelectedLight].position -= glm::vec4(0.0f, 0.0f, transDist, 1.0f);
		}
		if (mInputSystem.isKeyDown(GLFW_KEY_LEFT)) {	//negative z is away from pov
			mLightUBO.lights[mSelectedLight].position -= glm::vec4(transDist, 0.0f, 0.0f, 1.0f);
		}
		if (mInputSystem.isKeyDown(GLFW_KEY_RIGHT)) {
			mLightUBO.lights[mSelectedLight].position += glm::vec4(transDist, 0.0f, 0.0f, 1.0f);
		}
		if (mInputSystem.isKeyDown(GLFW_KEY_LEFT_BRACKET)) {	//negative z is away from pov
			mLightUBO.lights[mSelectedLight].position -= glm::vec4(0.0f, transDist, 0.0f, 1.0f);
		}
		if (mInputSystem.isKeyDown(GLFW_KEY_RIGHT_BRACKET)) {
			mLightUBO.lights[mSelectedLight].position += glm::vec4(0.0f, transDist, 0.0f, 1.0f);
		}

		//turn the selected light on/off
		if (mInputSystem.isKeyPressed(GLFW_KEY_O)) {
			mLightUBO.lights[mSelectedLight].isEnabled = !mLightUBO.lights[mSelectedLight].isEnabled;
			std::cout << "Light " << mSelectedLight << ": " << ((mLightUBO.lights[mSelectedLight].isEnabled) ? "on" : "off") << std::endl;
		}

		if (mInputSystem.isKeyPressed(GLFW_KEY_U)) {
			std::cout << "light position: " << mLightUBO.lights[mSelectedLight].position << std::endl;
		}
	}


	if (mInputSystem.isKeyDown(GLFW_KEY_G)) {
		mMirrorXForm.position += glm::vec3(0.0, 1.0, 0.0) * transDist;
	}

	if (mInputSystem.isKeyDown(GLFW_KEY_H)) {
		mMirrorXForm.position -= glm::vec3(0.0, 1.0, 0.0) * transDist;
	}

}

void VkApp::setupCamera()
{
	mCamera = std::make_unique<Camera>(Camera(WIDTH, HEIGHT));
	mCamera->position = glm::vec3(0.0f, 3.0f, 20.0f);
}

void VkApp::setupLights()
{
	mRenderSystem.createUniformBuffer<LightUBO>(mLightUBOBuffer, 1);
	mTotalLights = 1;

	//setup spotlight
	mLightUBO.lights[0].isEnabled	= true;
	mLightUBO.lights[0].lightType   = LightType::Spot;
	mLightUBO.lights[0].ambient     = glm::vec4(0.2, 0.2, 0.2, 1.0);
	mLightUBO.lights[0].diffuse     = glm::vec4(4.0f, 4.0f, 4.0f, 1.0f);		//red light
	mLightUBO.lights[0].specular    = glm::vec4(4.0f, 4.0f, 4.0f, 1.0f);		
	mLightUBO.lights[0].cutOff		= glm::cos(glm::radians(45.0f));			//12.5f
	mLightUBO.lights[0].outerCutOff = glm::cos(glm::radians(50.0f));			//15.0f
	mLightUBO.lights[0].constant = 0.25f;// 1.0f
	mLightUBO.lights[0].linear = 0.0f;// 0.09f;
	mLightUBO.lights[0].quadratic = 0.0f;// 0.032f;

	mRenderSystem.createUniformBuffer<glm::mat4>(mReflectionMVPBuffer, 1);
}

void VkApp::createLightIndicator(uint32_t lightIndex)
{
	//Resources
	std::shared_ptr<Mesh> lightMesh;
	mRenderSystem.createMesh(lightMesh, LIGHT_MODEL_PATH, false);
	
	ShaderSet lightIndicatorShaderSet;
	mRenderSystem.createShader(lightIndicatorShaderSet.vertShader, LIGHT_VERT_SHADER_PATH, VK_SHADER_STAGE_VERTEX_BIT);
	mRenderSystem.createShader(lightIndicatorShaderSet.fragShader, LIGHT_FRAG_SHADER_PATH, VK_SHADER_STAGE_FRAGMENT_BIT);

	mRenderSystem.createUniformBuffer<MVPMatrices>(mLightIndicatorMVPBuffer[lightIndex], 1);
	mRenderSystem.createUniformBuffer<Light>(mLightIndicatorLightBuffer[lightIndex], 1);


	mRenderSystem.createRenderable(mLightIndicators[lightIndex]);

	mLightIndicators[lightIndex]->applyShaderSet(lightIndicatorShaderSet);
	mLightIndicators[lightIndex]->addShaderBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0, 1);
	mLightIndicators[lightIndex]->addShaderBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, 1);


	mLightIndicators[lightIndex]->setMesh(lightMesh);
	mLightIndicators[lightIndex]->bindUniformBuffer(mLightIndicatorMVPBuffer[lightIndex], 0);
	mLightIndicators[lightIndex]->bindUniformBuffer(mLightIndicatorLightBuffer[lightIndex], 1);

	mLightIndicatorXForm[lightIndex].scale = glm::vec3(0.1f);

	std::cout << "Instantiating light #" << lightIndex << std::endl;
	mRenderSystem.instantiateRenderable(mLightIndicators[lightIndex]);
}

void VkApp::createCube()
{
	//start by creating the component resources
	std::shared_ptr<Mesh> cubeMesh;
	mRenderSystem.createMesh(cubeMesh, BOX_MODEL_PATH, true);

	std::shared_ptr<Texture> boxDiffuseMap;
	mRenderSystem.createTexture(boxDiffuseMap, BOX_DIFFUSE_PATH);

	std::shared_ptr<Texture> boxNormalMap;
	mRenderSystem.createTexture(boxNormalMap, BOX_NORMAL_PATH);

	std::shared_ptr<Texture> boxSpecularMap;
	mRenderSystem.createTexture(boxSpecularMap, BOX_SPECULAR_PATH);

	ShaderSet boxShaderSet;
	mRenderSystem.createShader(boxShaderSet.vertShader, BOX_VERT_SHADER_PATH, VK_SHADER_STAGE_VERTEX_BIT);
	mRenderSystem.createShader(boxShaderSet.fragShader, BOX_FRAG_SHADER_PATH, VK_SHADER_STAGE_FRAGMENT_BIT);
	
	mRenderSystem.createUniformBuffer<MVPMatrices>(mCubeMVPBuffer, 1);

	//create a renderable and make the appropriate attachments
	mRenderSystem.createRenderable(mCube);

	//setup the shaders and note the bindings they will use
	mCube->applyShaderSet(boxShaderSet);	
	mCube->addShaderBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0, 1);				//MVP
	mCube->addShaderBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, 1);				//lights
	mCube->addShaderBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2, 1);		//diffuse map
	mCube->addShaderBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 3, 1);		//normal map
	mCube->addShaderBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 4, 1);		//specular map

	//set the mesh we will use
	mCube->setMesh(cubeMesh);

	//bind resources
	mCube->bindUniformBuffer(mCubeMVPBuffer, 0);
	mCube->bindUniformBuffer(mLightUBOBuffer, 1);
	mCube->bindTexture(boxDiffuseMap, 2);
	mCube->bindTexture(boxNormalMap, 3);
	mCube->bindTexture(boxSpecularMap, 4);

	//finally, instantiate
	mRenderSystem.instantiateRenderable(mCube);
}

void VkApp::createMirror()
{
	//start by creating the component resources
	std::shared_ptr<Mesh> mirrorMesh;
	mRenderSystem.createMesh(mirrorMesh, MIRROR_MESH_PATH, false);

	std::shared_ptr<Texture> mirrorTexture;
	mRenderSystem.createTexture(mirrorTexture, MIRROR_TEXTURE_PATH);
	

	ShaderSet mirrorShaderSet;
	mRenderSystem.createShader(mirrorShaderSet.vertShader, MIRROR_VERT_SHADER_PATH, VK_SHADER_STAGE_VERTEX_BIT);
	mRenderSystem.createShader(mirrorShaderSet.fragShader, MIRROR_FRAG_SHADER_PATH, VK_SHADER_STAGE_FRAGMENT_BIT);

	mRenderSystem.createUniformBuffer<MVPMatrices>(mMirrorMVPBuffer, 1);

	//create a renderable and make the appropriate attachments
	mRenderSystem.createRenderable(mMirror);

	//setup the shaders and note the bindings they will use
	//Current restriction: one resource per binding (no arrays right now) 
	mMirror->applyShaderSet(mirrorShaderSet);
	mMirror->addShaderBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0, 1);				//MVP

	mMirror->addShaderBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, 1);		//reflection map
	mMirror->addShaderBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 2, 1);		//Texture map

	mMirror->setMesh(mirrorMesh);

	//bind resources
	mMirror->bindUniformBuffer(mMirrorMVPBuffer, 0);
	mMirror->bindReflectionMap(mRenderSystem.getReflectionMap(), 1);
	mMirror->bindTexture(mirrorTexture, 2);

	//instantiate (flush bindings, create pipeline)
	std::cout << "Instantianting a Mirror" << std::endl;
	mRenderSystem.instantiateRenderable(mMirror);
}

void VkApp::updateMVPBuffer(const UBO& mvpBuffer,   
							const Transform& renderableXForm, 
							const Camera& camera)
{
	MVPMatrices mvp = {};	
	mvp.model = renderableXForm.getModelMatrix();
	mvp.view = camera.viewMat;
	mvp.projection = camera.projMat;
	mvp.normalMat = glm::transpose(glm::inverse(mvp.view * mvp.model));

	mRenderSystem.updateUniformBuffer<MVPMatrices>(mvpBuffer, mvp, 0);
}

void VkApp::updateMirrorMVPBuffer(const UBO& mvpBuffer,
	const Transform& renderableXForm,
	const Camera& camera)
{
	MirrorMVPMatrices mvp = {};
	mvp.model = camera.viewMat * renderableXForm.getModelMatrix();
	mvp.projection = camera.projMat;

	mRenderSystem.updateUniformBuffer<MirrorMVPMatrices>(mvpBuffer, mvp, 0);
}

void VkApp::updateReflectionMVP(const Camera& camera, const Light & light)
{
	//To correct clip space (vulkan has inverted y, 1/2 z)
	/*const glm::mat4 clipFix(1.0f, 0.0f, 0.0f, 0.0f,
							0.0f, -1.0f, 0.0f, 0.0f,
							0.0f, 0.0f, 0.5f, 0.0f,
							0.0f, 0.0f, 0.5f, 1.0f);*/

	offscreenUBO refUBO = {};
	//glm::mat4 viewMatrix = glm::translate(glm::mat4(1.0f), -camera.position);

	refUBO.model = camera.viewMat * glm::translate(glm::mat4(1.0f), camera.position);
	refUBO.model = glm::scale(refUBO.model, glm::vec3(1.0f, -1.0f, 1.0f));
	refUBO.model = glm::translate(refUBO.model, mCubeXForm.position);

	refUBO.projection = camera.projMat;
	refUBO.lightPos = light.position;

	mRenderSystem.setOffscreenMVPBuffer(refUBO);
}
