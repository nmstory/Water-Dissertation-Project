#pragma once
#include "../nclgl/OGLRenderer.h"
class Camera;
class Shader;
class HeightMap;

class Renderer : public OGLRenderer {
public:
	Renderer(Window & parent);
	 ~Renderer(void);
	void RenderScene() override;
	void UpdateScene(float dt) override;
	
protected:
	void DrawHeightmap(Vector4 fv);
	void DrawWater();
	void DrawSkybox();
	void DrawAllObjects();
	
	Shader* lightShader;
	Shader* reflectShader;
	Shader* skyboxShader;
	Shader* sceneShader;
	
	HeightMap* heightMap;
	Mesh*  quad;
	
	Light* light;
	Camera* camera;
	
	GLuint cubeMap;
	GLuint waterTex;
	GLuint earthTex;
	GLuint earthBump;
	
	float waterRotate;
	float waterCycle;

	/****  !!!! WATER !!!!   ****/

	// Framebuffer methods
	void BindFramebuffer(GLuint frameBuffer, int height, int width);
	void BindDefaultFramebuffer();

	// FBO and Attachment methods
	GLuint GenerateFBO();
	GLuint CreateTextureAttachment(int height, int width, GLuint fbo);
	GLuint CreateDepthTextureAttachment(int height, int width, GLuint fbo);
	GLuint CreateDepthBufferAttachment(int height, int width, GLuint fbo);

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
	/*
	const int REFLECTION_WIDTH = 320;
	const int REFLECTION_HEIGHT = 180;
	const int REFRACTION_WIDTH = 1280;
	const int REFRACTION_HEIGHT = 720;
	*/

	const int REFLECTION_WIDTH = 1920;
	const int REFLECTION_HEIGHT = 1080;
	const int REFRACTION_WIDTH = 1920;
	const int REFRACTION_HEIGHT = 1080;

	// Clipping plane levels
	int waterHeight = 100; // set using getheightmapsize!!

	GLuint waterDudvMap;
	GLuint waterBumpMap;


	const float WAVE_SPEED = 0.1f;
	float waveMovementFactor;

	// Misc.
	Mesh* waterQuad;
	Shader* waterShader;
	Window* window;


	// PALM TREE TEST
	void DrawPT();
	Mesh* ptMesh;
	GLuint ptDiffuse;
	GLuint ptNormal;
};