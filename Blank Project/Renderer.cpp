#include "Renderer.h"

Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	this->window = &parent;
	
	waterQuad = Mesh::GenerateQuad();

	std::queue<Vector3> cameraTrack;
	camera = new Camera(0, 0, Vector3(0, 10, 10), cameraTrack);

	waterShader = new Shader("WaterVertexv2.glsl", "WaterFragmentv2.glsl");
	if (!waterShader->LoadSuccess()) {
		return;
	}

	InitialiseReflectionAndRefraction();

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	init = true;
}

Renderer::~Renderer() {
	delete waterQuad;
	delete camera;
	delete waterShader;

	// Reflection and Refraction
	glDeleteFramebuffers(1, &reflectionFrameBuffer);
	glDeleteTextures(1, &reflectionTexture);
	glDeleteRenderbuffers(1, &reflectionDepthBuffer);
	glDeleteFramebuffers(1, &refractionFrameBuffer);
	glDeleteTextures(1, &refractionTexture);
	glDeleteTextures(1, &refractionDepthTexture);
}

void Renderer::RenderScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	BindReflectionFrameBuffer();
	// render the scene here
	BindDefaultFramebuffer();



	// Render water
	BindShader(waterShader);
	UpdateShaderMatrices();
	waterQuad->Draw();
}
void Renderer::UpdateScene(float dt) {
	camera->UpdateCamera(dt);
	viewMatrix = camera->BuildViewMatrix();
	projMatrix = Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, 45.0f); // dont need to do every frame!
}

void Renderer::InitialiseReflectionAndRefraction() {
	// Reflection FBO
	reflectionFrameBuffer = GenerateFBO();
	reflectionTexture = CreateTextureAttachment(REFLECTION_WIDTH, REFLECTION_HEIGHT);
	reflectionDepthBuffer = CreateDepthBufferAttachment(REFLECTION_WIDTH, REFLECTION_HEIGHT);
	BindDefaultFramebuffer();

	// Refraction FBO
	refractionFrameBuffer = GenerateFBO();
	refractionTexture = CreateTextureAttachment(REFRACTION_WIDTH, REFRACTION_HEIGHT);
	refractionDepthTexture = CreateDepthTextureAttachment(REFRACTION_WIDTH, REFRACTION_HEIGHT);
	BindDefaultFramebuffer();
}

void Renderer::BindFramebuffer(GLuint frameBuffer, int height, int width) {
	glBindTexture(GL_TEXTURE_2D, 0); // unbind any current texture
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glViewport(0, 0, width, height);
}

void Renderer::BindDefaultFramebuffer() {
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glViewport(0, 0, window->GetScreenSize().x, window->GetScreenSize().y);
}


GLuint Renderer::GenerateFBO() {
	GLuint fbo;
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);
	//glDrawBuffer(fbo, , GL_COLOR_ATTACHMENT0);
}

GLuint Renderer::CreateTextureAttachment(int height, int width) {
	GLuint texture;
	glGenTextures(GL_TEXTURE_2D, &texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture, 0); // adding colour attachment to fbo
	return texture;
}

GLuint Renderer::CreateDepthTextureAttachment(int height, int width) {
	GLuint texture;
	glGenTextures(GL_TEXTURE_2D, &texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture, 0);// adding depth attachment to fbo
	return texture;
}

GLuint Renderer::CreateDepthBufferAttachment(int height, int width) {
	GLuint depthBuffer;
	glGenRenderbuffers(GL_RENDERBUFFER, &depthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
	return depthBuffer;
}