#include "Renderer.h"
#include "../nclgl/Light.h"
#include "../nclgl/Heightmap.h"
#include "../nclgl/Shader.h"
#include "../nclgl/Camera.h"

Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	quad = Mesh::GenerateQuad();
	window = &parent;
	
	heightMap = new HeightMap(TEXTUREDIR"noise.png");

	waterTex = SOIL_load_OGL_texture(TEXTUREDIR"water.TGA", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	waterDudvMap = SOIL_load_OGL_texture(TEXTUREDIR"waterDUDV.PNG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	waterBumpMap = SOIL_load_OGL_texture(TEXTUREDIR"waterNormal.PNG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	earthTex = SOIL_load_OGL_texture(TEXTUREDIR"Barren Reds.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	earthBump = SOIL_load_OGL_texture(TEXTUREDIR"Barren RedsDOT3.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	cubeMap = SOIL_load_OGL_cubemap(
		TEXTUREDIR"rusted_west.jpg", TEXTUREDIR"rusted_east.jpg",
		TEXTUREDIR"rusted_up.jpg", TEXTUREDIR"rusted_down.jpg",
		TEXTUREDIR"rusted_south.jpg", TEXTUREDIR"rusted_north.jpg",
		SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

	if (!earthTex || !earthBump || !cubeMap || !waterTex || !waterDudvMap || !waterBumpMap) return;

	SetTextureRepeating(earthTex, true);
	SetTextureRepeating(earthBump, true);
	SetTextureRepeating(waterTex, true);
	SetTextureRepeating(waterDudvMap, true);
	SetTextureRepeating(waterBumpMap, true);

	reflectShader = new Shader("reflectVertex.glsl", "reflectFragment.glsl");
	skyboxShader = new Shader("skyboxVertex.glsl", "skyboxFragment.glsl");
	lightShader = new Shader("PerPixelVertex.glsl", "PerPixelFragment.glsl");
	//waterShader = new Shader("WaterVertexv2.glsl", "WaterFragmentv2.glsl");
	waterShader = new Shader("WaterVertexv2.glsl", "WaterFragmentv2.glsl", "GeometryShader.glsl", "TCS.glsl", "TES.glsl");
	sceneShader = new Shader("vertexShader.glsl", "fragShader.glsl");

	if (!reflectShader->LoadSuccess() || !skyboxShader->LoadSuccess() || !lightShader->LoadSuccess() || !waterShader->LoadSuccess() || !sceneShader->LoadSuccess()) return;

	Vector3 heightmapSize = heightMap->GetHeightmapSize();

	camera = new Camera(-45.0f, 0.0f, heightmapSize * Vector3(0.5f, 2.0f, 0.5f));
	light = new Light(heightmapSize * Vector3(0.5f, 1.5f, 0.5f), Vector4(1, 1, 1, 1), heightmapSize.x);

	projMatrix = Matrix4::Perspective(1.0f, 5000.0f, (float)width / (float)height, 45.0f);

	waterQuad = Mesh::GenerateQuad();

	InitialiseReflectionAndRefraction();

	waveMovementFactor = 0; // init to 0


	// Palm tree!!!1
	ptMesh = Mesh::LoadFromMeshFile("palm_tree.msh");
	ptDiffuse = SOIL_load_OGL_texture(TEXTUREDIR"ptDiffuse.TGA", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	ptNormal = SOIL_load_OGL_texture(TEXTUREDIR"ptNormal.TGA", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	



	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	waterRotate = 0.0f;
	waterCycle = 0.0f;
	init = true;
}

Renderer ::~Renderer(void) {
	delete camera;
	delete heightMap;
	delete quad;
	delete reflectShader;
	delete skyboxShader;
	delete lightShader;
	delete light;

	delete waterQuad;
	delete waterShader;

	// Reflection and Refraction
	glDeleteFramebuffers(1, &reflectionFrameBuffer);
	glDeleteTextures(1, &reflectionTexture);
	glDeleteRenderbuffers(1, &reflectionDepthBuffer);
	glDeleteFramebuffers(1, &refractionFrameBuffer);
	glDeleteTextures(1, &refractionTexture);
	glDeleteTextures(1, &refractionDepthTexture);
}

void Renderer::UpdateScene(float dt) {
	camera->UpdateCamera(dt);
	viewMatrix = camera->BuildViewMatrix();
	waterRotate += dt * 2.0f; //2 degrees a second          REMOVE ALL INSTANCES OF MEEe!!!
	waterCycle += dt * 0.25f; //10 units a second
	waveMovementFactor += dt * WAVE_SPEED;
	waveMovementFactor = fmod(waveMovementFactor, 1.0f); // loop back to 1, using  fmod as % with floats
}

void Renderer::RenderScene() {




	// https://www.ultraengine.com/community/blogs/entry/1185-pass-the-tessellation-please-part-1/








	/*
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	DrawSkybox();
	DrawHeightmap();
	DrawWater();
	*/

	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	glEnable(GL_CLIP_DISTANCE0);

	float waterHeightOffset = 0.5f; // set as const and say WHY! (To reduce clipping fringing on edges, as the plane was being clipped exactly on water height)
	Vector4 reflectionClipPlane(0,1,0,-waterHeight + waterHeightOffset); // (0,1,0, -waterheight)
	Vector4 refractionClipPlane(0,-1,0,waterHeight); // (0,-1,0, waterheight)
	Vector4 standardClipPlane(0,0,0,0); // (0,0,0,0)

	BindReflectionFrameBuffer();
	float distance = 2 * (camera->GetPosition().y  - waterHeight); // CURRENTLY SET 125 = water size height
	camera->SetPosition(Vector3(camera->GetPosition().x, camera->GetPosition().y - distance, camera->GetPosition().z));
	camera->SetPitch(camera->GetPitch() * -1);
	viewMatrix = camera->BuildViewMatrix();
	DrawSkybox();
	//DrawHeightmap(reflectionClipPlane);
	DrawPT();
	camera->SetPosition(Vector3(camera->GetPosition().x, camera->GetPosition().y + distance, camera->GetPosition().z));
	camera->SetPitch(camera->GetPitch() * -1);
	viewMatrix = camera->BuildViewMatrix();

	BindRefractionFrameBuffer();
	DrawSkybox();
	//DrawHeightmap(refractionClipPlane);
	DrawPT();

	BindDefaultFramebuffer();

	DrawSkybox();
	//DrawHeightmap(standardClipPlane);
	DrawPT();
	BindShader(waterShader);
	modelMatrix = Matrix4::Translation(Vector3(2000,125,2000)) * Matrix4::Scale(Vector3(1000,1,1000));
	UpdateShaderMatrices();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, GetReflectionTexture());
	glUniform1i(glGetUniformLocation(waterShader->GetProgram(), "reflectionTexture"), 0);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, GetRefractionTexture());
	glUniform1i(glGetUniformLocation(waterShader->GetProgram(), "refractionTexture"), 1);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, waterDudvMap);
	glUniform1i(glGetUniformLocation(waterShader->GetProgram(), "dudvMap"), 2);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, waterBumpMap);
	glUniform1i(glGetUniformLocation(waterShader->GetProgram(), "normalMap"), 3);

	glActiveTexture(GL_TEXTURE4);
	glBindTexture(GL_TEXTURE_2D, GetRefractionDepthTexture());
	glUniform1i(glGetUniformLocation(waterShader->GetProgram(), "depthMap"), 4);

	glUniform1f(glGetUniformLocation(waterShader->GetProgram(), "waveMovementFactor"), waveMovementFactor);

	glUniform3fv(glGetUniformLocation(waterShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());
	
	glUniform3fv(glGetUniformLocation(waterShader->GetProgram(), "lightColour"), 1, (float*)&light->GetColour());
	glUniform3fv(glGetUniformLocation(waterShader->GetProgram(), "lightPos"), 1, (float*)&light->GetPosition());
	
	glPatchParameteri(GL_PATCH_VERTICES, 4);
	waterQuad->Draw();
}


void Renderer::DrawPT() {
	BindShader(lightShader);
	SetShaderLight(*light);
	glUniform3fv(glGetUniformLocation(lightShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());
	//glUniform4fv(glGetUniformLocation(lightShader->GetProgram(), "clipPlaneLevel"), 1, (float*)&fv);
	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ptDiffuse);
	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "bumpTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, ptNormal);

	modelMatrix.ToIdentity(); //New!
	modelMatrix = Matrix4::Translation(Vector3(900, 0, 900)) * Matrix4::Scale(Vector3(1, 1, 1));
	textureMatrix.ToIdentity(); //New!

	UpdateShaderMatrices();

	ptMesh->Draw();
}




void Renderer::DrawSkybox() {
	glDepthMask(GL_FALSE);
	
	BindShader(skyboxShader);
	UpdateShaderMatrices();
	
	quad->Draw();
	
	glDepthMask(GL_TRUE);
}

void Renderer::DrawHeightmap(Vector4 fv) {
	BindShader(lightShader);
	SetShaderLight(*light);
	glUniform3fv(glGetUniformLocation(lightShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());
	glUniform4fv(glGetUniformLocation(lightShader->GetProgram(), "clipPlaneLevel"), 1, (float*)&fv);
	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, earthTex);
	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "bumpTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, earthBump);

	modelMatrix.ToIdentity(); //New!
	textureMatrix.ToIdentity(); //New!

	UpdateShaderMatrices();

	heightMap->Draw();
}

void Renderer::DrawWater() {
	BindShader(reflectShader);
	
	glUniform3fv(glGetUniformLocation(reflectShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());
	
	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(), "cubeTex"), 2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, waterTex);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);

	Vector3 hSize = heightMap->GetHeightmapSize();
	
	modelMatrix = Matrix4::Translation(hSize * 0.5f) * Matrix4::Scale(hSize * 0.5f) * Matrix4::Rotation(90, Vector3(1, 0, 0));
	
	textureMatrix = Matrix4::Translation(Vector3(waterCycle, 0.0f, waterCycle)) * Matrix4::Scale(Vector3(10, 10, 10)) * Matrix4::Rotation(waterRotate, Vector3(0, 0, 1));
	
	UpdateShaderMatrices();
	// SetShaderLight (* light); //No lighting in this shader!
	quad->Draw();
}

/****  !!!! WATER !!!!   ****/

void Renderer::InitialiseReflectionAndRefraction() {
	// Reflection FBO
	reflectionFrameBuffer = GenerateFBO();
	reflectionTexture = CreateTextureAttachment(REFLECTION_HEIGHT, REFLECTION_WIDTH, reflectionFrameBuffer);
	reflectionDepthBuffer = CreateDepthBufferAttachment(REFLECTION_HEIGHT, REFLECTION_WIDTH, reflectionFrameBuffer);
	BindDefaultFramebuffer();

	// Refraction FBO
	refractionFrameBuffer = GenerateFBO();
	refractionTexture = CreateTextureAttachment(REFRACTION_HEIGHT, REFRACTION_WIDTH, refractionFrameBuffer);
	refractionDepthTexture = CreateDepthTextureAttachment(REFRACTION_HEIGHT, REFRACTION_WIDTH, refractionFrameBuffer);
	BindDefaultFramebuffer();
}

void Renderer::BindFramebuffer(GLuint frameBuffer, int height, int width) {
	glBindTexture(GL_TEXTURE_2D, 0); // unbind any current texture
	glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
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
	const GLenum bb = GL_COLOR_ATTACHMENT0;
	glDrawBuffers(1, &bb);
	//glDrawBuffer(fbo, , GL_COLOR_ATTACHMENT0);
	return fbo;
}

GLuint Renderer::CreateTextureAttachment(int height, int width, GLuint fbo) {
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0); // adding colour attachment to fbo
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	return texture;
}

GLuint Renderer::CreateDepthTextureAttachment(int height, int width, GLuint fbo) {
	GLuint texture;
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, width, height, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL);
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture, 0);// adding depth attachment to fbo
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	return texture;
}

GLuint Renderer::CreateDepthBufferAttachment(int height, int width, GLuint fbo) {
	GLuint depthBuffer;
	glGenRenderbuffers(1, &depthBuffer);
	glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
	return depthBuffer;
}