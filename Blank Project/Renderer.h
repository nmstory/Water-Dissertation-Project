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
	Mesh* waterQuad;
	Camera* camera;
	Shader* waterShader;
};