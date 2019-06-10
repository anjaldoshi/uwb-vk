#include "VkApp.h"
#include "PrintUtil.h"

VkApp::VkApp() : mWindow(nullptr) {}

void VkApp::run()
{
	initialize(WINDOW_TITLE);
	
	glfwSetTime(0.0);

	while (!glfwWindowShouldClose(mWindow)) {
		glfwPollEvents();
		handleInput();

		mCamera->updateViewMatrix();

		updateMVPBuffer(*mTestModelMVPBuffer, mTestModelXform, *mCamera);

		mRenderSystem.drawFrame();

		//update the frame timer
		mTime = glfwGetTime();
		mFrameTime = mTime - mPrevTime;		
		mPrevTime = mTime;
	}

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

	cameraControls();
}


void VkApp::cameraControls()
{
	float transDist = cCamTranslateSpeed * mFrameTime;

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