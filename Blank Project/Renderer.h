#pragma once

#include "../nclgl/OGLRenderer.h"
#include "../nclgl/Window.h"
#include "../nclgl/Mesh.h"
#include "../nclgl/Camera.h"
#include "../nclgl/Shader.h"

class Renderer : public OGLRenderer {
public:
	Renderer(Window& parent);
	~Renderer();

	void RenderScene() override;
	void UpdateScene(float dt) override;

	Camera* GetCamera() { return camera; }

private:
	// Framebuffer methods
	void BindFramebuffer(GLuint frameBuffer, int height, int width);
	void BindDefaultFramebuffer();

	// FBO and Attachment methods
	GLuint GenerateFBO();
	GLuint CreateTextureAttachment(int height, int width);
	GLuint CreateDepthTextureAttachment(int height, int width);
	GLuint CreateDepthBufferAttachment(int height, int width);

	// Misc.
	Mesh* waterQuad;
	Camera* camera;
	Shader* waterShader;
	Window* window;

	// Reflction and Refraction
	void InitialiseReflectionAndRefraction();

	void BindReflectionFrameBuffer() { BindFramebuffer(reflectionFrameBuffer, REFLECTION_HEIGHT, REFLECTION_WIDTH); }
	void BindRefractionFrameBuffer() { BindFramebuffer(refractionFrameBuffer, REFRACTION_HEIGHT, REFRACTION_WIDTH); }
	
	GLuint GetReflectionTexture() { return reflectionTexture; }
	GLuint GetRefractionTexture() { return refractionTexture; }
	GLuint GetRefractionDepthTexture() { return refractionDepthTexture; }
	
	GLuint reflectionFrameBuffer;
	GLuint reflectionTexture;
	GLuint reflectionDepthBuffer;
	GLuint refractionFrameBuffer;
	GLuint refractionTexture;
	GLuint refractionDepthTexture;

	const int REFLECTION_WIDTH = 320;
	const int REFLECTION_HEIGHT = 180;
	const int REFRACTION_WIDTH = 1280;
	const int REFRACTION_HEIGHT = 720;
};