#include "VkApp.h"
#include "PrintUtil.h"

VkApp::VkApp() : mWindow(nullptr) {}

void VkApp::run()
{
	initialize(WINDOW_TITLE);
	
	glfwSetTime(0.0);
	std::cout << "Starting loop..." << std::endl;
	//update loop
	while (!glfwWindowShouldClose(mWindow)) {
		glfwPollEvents();
		handleInput();

		mCamera->updateViewMatrix();

		updateMVPBuffer(*mTestModelMVPBuffer, mTestModelXform, *mCamera);
		updateMVPBuffer(*mTestPlaneMVPBuffer, mTestPlaneXform, *mCamera);
		updateMVPBuffer(*mTeapotMVPBuffer, mTeapotXform, *mCamera);

		mRenderSystem.drawFrame();

		//update the frame timer
		mTime = glfwGetTime();
		mFrameTime = mTime - mPrevTime;		
		mPrevTime = mTime;
	}

	std::cout << "---------------------------------------" << std::endl;
	shutdown();
}

void VkApp::initialize(const std::string& appName)
{
	glfwInit();
	createWindow();
	mRenderSystem.initialize(mWindow, appName);
	mInputSystem.initialize(mWindow);

	setupCamera();

	createTestModel();
	mTestModelXform.scale = glm::vec3(2.0f);
	mTestModelXform.rotation *= glm::angleAxis(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

	createTeapot();
	mTeapotXform.scale = glm::vec3(0.025f);
	mTeapotXform.position = glm::vec3(0.0f, 1.0f, 5.0f);
	mTeapotXform.rotation *= glm::angleAxis(glm::radians(180.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	
	createTestPlane();
	mTestPlaneXform.scale = glm::vec3(10.0f);
	mTestPlaneXform.rotation *= glm::angleAxis(glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
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

	//change the clear color
	if (mInputSystem.isKeyPressed(GLFW_KEY_B)) {
		mRenderSystem.setClearColor(clearColors[clearColorIndex]);
		clearColorIndex++;
		if (clearColorIndex >= clearColors.size()) clearColorIndex = 0;
	}

	//close the window
	if (mInputSystem.isKeyPressed(GLFW_KEY_ESCAPE))
		glfwSetWindowShouldClose(mWindow, GLFW_TRUE);

	//print out FPS
	if (mInputSystem.isKeyPressed(GLFW_KEY_F))
		std::cout << "frameTime: " << mFrameTime * 1000.0 << " ms ( " << (1.0 / mFrameTime) << " fps)" << std::endl;


	//Model Controls
	if (mInputSystem.isKeyDown(GLFW_KEY_J)) {
		mTestModelXform.position -= glm::vec3(mModelTranslateSpeed * mFrameTime, 0.0f, 0.0f);
	}
	if (mInputSystem.isKeyDown(GLFW_KEY_L)) {
		mTestModelXform.position += glm::vec3(mModelTranslateSpeed * mFrameTime, 0.0f, 0.0f);
	}
	if (mInputSystem.isKeyDown(GLFW_KEY_I)) {
		mTestModelXform.position += glm::vec3(0.0f, 0.0f, mModelTranslateSpeed * mFrameTime);
	}
	if (mInputSystem.isKeyDown(GLFW_KEY_K)) {
		mTestModelXform.position -= glm::vec3(0.0f, 0.0f, mModelTranslateSpeed * mFrameTime);
	}
	if (mInputSystem.isKeyDown(GLFW_KEY_U)) {
		mTestModelXform.rotation *= glm::angleAxis(mModelRotateSpeed * mFrameTime, mCamera->right);
	}
	if (mInputSystem.isKeyDown(GLFW_KEY_O)) {
		mTestModelXform.rotation *= glm::angleAxis(-mModelRotateSpeed * mFrameTime, mCamera->right);
	}

	cameraControls();
}


void VkApp::cameraControls()
{
	float transDist = cCamTranslateSpeed * mFrameTime;
	float rotAmount = glm::radians(cCamRotateSpeed * mFrameTime);

	const float cursorSensitivity = 3.5f;
	glm::vec3 deltaCursor = mInputSystem.getMouseDelta();
	glm::quat pitch = glm::angleAxis(-deltaCursor.x * cursorSensitivity * mFrameTime, mCamera->up);	//rotation is ccw around axis

	glm::quat yaw = glm::angleAxis(-deltaCursor.y * cursorSensitivity * mFrameTime, mCamera->right);

	mCamera->rotation *= pitch;

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
	if (mInputSystem.isKeyDown(GLFW_KEY_Q)) {
		mCamera->position -= mCamera->up * transDist;
	}
	if (mInputSystem.isKeyDown(GLFW_KEY_E)) { 
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

void VkApp::setupCamera()
{
	mCamera = std::make_unique<Camera>(Camera(WIDTH, HEIGHT));
	mCamera->position = glm::vec3(0.0f, 3.0f, 10.0f);
	std::cout << "Camera Setup Done" << std::endl;
}

void VkApp::createTestModel() {

	std::shared_ptr<Mesh> mTestModelMesh;
	mRenderSystem.createMesh(mTestModelMesh, CHALET_MODEL_PATH, false);

	std::shared_ptr<Texture> mTestModelTex;
	mRenderSystem.createTexture(mTestModelTex, CHALET_TEXTURE_PATH);

	ShaderSet mTestModelShaders;
	mRenderSystem.createShader(mTestModelShaders.vertShader, VERT_SHADER_PATH, VK_SHADER_STAGE_VERTEX_BIT);
	mRenderSystem.createShader(mTestModelShaders.fragShader, FRAG_SHADER_PATH, VK_SHADER_STAGE_FRAGMENT_BIT);

	mRenderSystem.createUniformBuffer<MVPMatrices>(mTestModelMVPBuffer, 1);

	mRenderSystem.createRenderable(mTestModel);

	mTestModel->applyShaderSet(mTestModelShaders);
	mTestModel->addShaderBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0, 1);				//MVP
	mTestModel->addShaderBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, 1);	//texture map

	mTestModel->setMesh(mTestModelMesh);

	mTestModel->bindUniformBuffer(mTestModelMVPBuffer, 0);
	mTestModel->bindTexture(mTestModelTex, 1);

	mRenderSystem.instantiateRenderable(mTestModel);
}

void VkApp::createTestPlane() {

	std::shared_ptr<Mesh> mTestPlaneMesh;
	mRenderSystem.createMesh(mTestPlaneMesh, GROUND_MODEL_PATH, false);

	std::shared_ptr<Texture> mTestPlaneTex;
	mRenderSystem.createTexture(mTestPlaneTex, GROUND_TEXTURE_PATH);

	ShaderSet mTestPlaneShaders;
	mRenderSystem.createShader(mTestPlaneShaders.vertShader, VERT_SHADER_PATH, VK_SHADER_STAGE_VERTEX_BIT);
	mRenderSystem.createShader(mTestPlaneShaders.fragShader, FRAG_SHADER_PATH, VK_SHADER_STAGE_FRAGMENT_BIT);

	mRenderSystem.createUniformBuffer<MVPMatrices>(mTestPlaneMVPBuffer, 1);

	mRenderSystem.createRenderable(mTestPlane);

	mTestPlane->applyShaderSet(mTestPlaneShaders);
	mTestPlane->addShaderBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0, 1);				//MVP
	mTestPlane->addShaderBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, 1);	//texture map

	mTestPlane->setMesh(mTestPlaneMesh);

	mTestPlane->bindUniformBuffer(mTestPlaneMVPBuffer, 0);
	mTestPlane->bindTexture(mTestPlaneTex, 1);

	mRenderSystem.instantiateRenderable(mTestPlane);
}

void VkApp::createTeapot() {

	std::shared_ptr<Mesh> mTeapotMesh;
	mRenderSystem.createMesh(mTeapotMesh, TEA_MODEL_PATH, false);

	std::shared_ptr<Texture> mTepotTex;
	mRenderSystem.createTexture(mTepotTex, TEA_TEXTURE_PATH);

	ShaderSet mTeapotShaders;
	mRenderSystem.createShader(mTeapotShaders.vertShader, VERT_SHADER_PATH, VK_SHADER_STAGE_VERTEX_BIT);
	mRenderSystem.createShader(mTeapotShaders.fragShader, FRAG_SHADER_PATH, VK_SHADER_STAGE_FRAGMENT_BIT);

	mRenderSystem.createUniformBuffer<MVPMatrices>(mTeapotMVPBuffer, 1);

	mRenderSystem.createRenderable(mTeapot);

	mTeapot->applyShaderSet(mTeapotShaders);
	mTeapot->addShaderBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT, 0, 1);				//MVP
	mTeapot->addShaderBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT, 1, 1);	//texture map

	mTeapot->setMesh(mTeapotMesh);

	mTeapot->bindUniformBuffer(mTeapotMVPBuffer, 0);
	mTeapot->bindTexture(mTepotTex, 1);

	mRenderSystem.instantiateRenderable(mTeapot);
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