#include "Renderer.h"

Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	waterQuad = Mesh::GenerateQuad();

	std::queue<Vector3> cameraTrack;
	camera = new Camera(0, 0, Vector3(0, 10, 10), cameraTrack);

	waterShader = new Shader("WaterVertexv2.glsl", "WaterFragmentv2.glsl");
	if (!waterShader->LoadSuccess()) {
		return;
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	init = true;
}

Renderer::~Renderer() {
	delete waterQuad;
	delete camera;
	delete waterShader;
}

void Renderer::RenderScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	BindShader(waterShader);
	
	UpdateShaderMatrices();
	waterQuad->Draw();
}
void Renderer::UpdateScene(float dt) {
	camera->UpdateCamera(dt);
	viewMatrix = camera->BuildViewMatrix();
	projMatrix = Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, 45.0f); // dont need to do every frame!
}