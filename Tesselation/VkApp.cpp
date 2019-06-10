#include "VkApp.h"


VkApp::VkApp() : mWindow(nullptr) {}

void VkApp::run()
{
	initialize();
	
	glfwSetTime(0.0);
	std::cout << "Starting loop..." << std::endl;
	//update loop
	while (!glfwWindowShouldClose(mWindow)) {
		glfwPollEvents();
		handleInput();

		updateMVPMatrices(mTestPlane, mTestPlaneXform);
		updateDemoOptions();

		mRenderSystem.drawFrame();

		//update the frame timer
		mTime = glfwGetTime();
		mFrameTime = mTime - mPrevTime;		
		mPrevTime = mTime;
	}

	std::cout << "---------------------------------------" << std::endl;
	shutdown();
}

void VkApp::initialize()
{
	glfwInit();
	createWindow();
	mInputSystem.initialize(mWindow);
	mRenderSystem.initialize(mWindow);


	createTesselatedPlane();
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

	//Camera Controls
	if (mInputSystem.isKeyDown(GLFW_KEY_UP)){
		mCamRotate.x += mCamRotateSpeed * (float)mFrameTime;
	}
	if (mInputSystem.isKeyDown(GLFW_KEY_DOWN)){
		mCamRotate.x -= mCamRotateSpeed * (float)mFrameTime;
	}
	if (mInputSystem.isKeyDown(GLFW_KEY_LEFT)) {
		mCamRotate.z += mCamRotateSpeed * (float)mFrameTime;
	}
	if (mInputSystem.isKeyDown(GLFW_KEY_RIGHT)) {
		mCamRotate.z -= mCamRotateSpeed * (float)mFrameTime;
	}
	//forward / back
	if (mInputSystem.isKeyDown(GLFW_KEY_W)) {
		mCamDist -= mCamTranslateSpeed * (float)mFrameTime;
	}
	else if (mInputSystem.isKeyDown(GLFW_KEY_S)) {
		mCamDist += mCamTranslateSpeed * (float)mFrameTime;
	}

	//Demo options manipulation
	if (mInputSystem.isKeyDown(GLFW_KEY_I)) {
		displacementMultiplier += optionsModSpeed * (float)mFrameTime;
	}
	else if (mInputSystem.isKeyDown(GLFW_KEY_K)) {
		displacementMultiplier -= optionsModSpeed * (float)mFrameTime;
	}
	if (mInputSystem.isKeyDown(GLFW_KEY_O)) {
		tessLevel += optionsModSpeed * (float)mFrameTime;
	}
	else if (mInputSystem.isKeyDown(GLFW_KEY_L)) {
		tessLevel -= optionsModSpeed * (float)mFrameTime;
	}
}

void VkApp::createTesselatedPlane()
{
	//start by creating the component resources
	std::shared_ptr<Mesh> mesh;
	mRenderSystem.createMesh(mesh, GROUND_MODEL_PATH);

	std::shared_ptr<Texture> displacementMap;
	mRenderSystem.createTexture(displacementMap, DISPLACEMENT_MAP_PATH);

	std::shared_ptr<Texture> texture;
	mRenderSystem.createTexture(texture, GROUND_TEXTURE_PATH);

	mRenderSystem.createUniformBuffer<MVPMatrices>(tessEvalBuffer);
	mRenderSystem.createUniformBuffer<float>(tessControlBuffer);
	
	ShaderSet planeShaderSet;
	mRenderSystem.createShader(planeShaderSet.vertShader, 
							   VERT_SHADER_PATH, 
							   VK_SHADER_STAGE_VERTEX_BIT);
	mRenderSystem.createShader(planeShaderSet.tessControlShader, 
							   TESS_CONTROL_SHADER_PATH,
							   VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
	mRenderSystem.createShader(planeShaderSet.tessEvalShader, 
							   TESS_EVAL_SHADER_PATH, 
						       VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
	mRenderSystem.createShader(planeShaderSet.fragShader, 
							   FRAG_SHADER_PATH, 
							   VK_SHADER_STAGE_FRAGMENT_BIT);



	//create a renderable and make the appropriate attachments
	//current restrictions: 
	//bindings are uniforms first then textures
	//uniforms and textures need to be added to the renderable in the same order as their associated bindings

	mRenderSystem.createRenderable(mTestPlane);


	mTestPlane->applyShaderSet(planeShaderSet);
	mTestPlane->addBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
						   VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT, 
						   0,	//binding number (i.e. "layout(binding = 0)" in glsl
						   1);	//resource quantity (if an array)
	mTestPlane->addBinding(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 
						   VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, 
						   1, 1);
	mTestPlane->addBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 
						   VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT, 
						   2, 1);
	mTestPlane->addBinding(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 
						   VK_SHADER_STAGE_FRAGMENT_BIT, 
						   3, 1);

	mTestPlane->setMesh(mesh);
	mTestPlane->addUniformBuffer(tessControlBuffer);
	mTestPlane->addUniformBuffer(tessEvalBuffer);
	mTestPlane->addTexture(displacementMap);
	mTestPlane->addTexture(texture);
	

	mTestPlaneXform.scale = glm::vec3(1.5f, 1.5f, 1.5f);
	mRenderSystem.instantiateRenderable(mTestPlane);
}

void VkApp::updateMVPMatrices(const std::shared_ptr<Renderable>& renderable, const xform& xform)
{
	float modelScale = 1.0f;
	float modelRotateZ = 90.0f;
	float translate = mCamDist;
	float rotateX = 0.0f;
	float rotateY = 90.0f;

	static auto startTime = std::chrono::high_resolution_clock::now();

	auto currentTime = std::chrono::high_resolution_clock::now();
	float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

	MVPMatrices mvp = {};
	mvp.model = glm::scale(glm::mat4(1.0f), xform.scale);
	mvp.model = glm::translate(mvp.model, xform.pos);
	mvp.model = glm::rotate(mvp.model, glm::radians(xform.rot.z), glm::vec3(0.0f, 0.0f, 1.0f));

	mvp.view = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -translate));
	mvp.view = glm::rotate(mvp.view, glm::radians(mCamRotate.x), glm::vec3(1.0f, 0.0f, 0.0f));
	mvp.view = glm::rotate(mvp.view, glm::radians(mCamRotate.y), glm::vec3(0.0f, 1.0f, 0.0f));
	mvp.view = glm::rotate(mvp.view, glm::radians(mCamRotate.z), glm::vec3(0.0f, 0.0f, 1.0f));

	mvp.proj = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);

	mvp.proj[1][1] *= -1;

	mvp.dispMod = displacementMultiplier;

	mRenderSystem.updateUniformBuffer<MVPMatrices>(*tessEvalBuffer, mvp);
}

void VkApp::updateDemoOptions()
{
	mRenderSystem.updateUniformBuffer<float>(*tessControlBuffer, tessLevel);
}
