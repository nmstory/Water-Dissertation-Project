#pragma once
#include "../nclgl/OGLRenderer.h"
#include "../nclgl/SceneNode.h"
#include "../nclgl/Frustum.h"
#include "../nclgl/MeshMaterial.h"
#include <algorithm>
#include <vector>

class HeightMap;
class Camera;
class OGLRenderer;

class Renderer : public OGLRenderer {
public:
	Renderer(Window& parent);
	~Renderer(void);
	void RenderScene() override;
	void UpdateScene(float dt) override;

	// Auxilliary methods
	Camera* GetCamera() { return camera; }

protected:
	// Rendering functions
	void DrawShadowScene();
	void DrawShadowObjects();
	void DrawSkybox();
	void DrawHeightmap();
	void DrawWater();

	// Scene Node functions 
	void BuildNodeLists(SceneNode* from);
	void SortNodeLists();
	void ClearNodeLists();
	void DrawNodes();
	void DrawNode(SceneNode* n);

	// Lighting
	Light* light;
	GLuint bumpMapDirt;
	GLuint bumpMapRock;

	// Scene Management
	SceneNode* root;
	Frustum frameFrustum;
	std::vector <SceneNode*> transparentNodeList;
	std::vector <SceneNode*> nodeList;

	// Heightmap
	HeightMap* heightMap;
	Shader* heightMapShader;
	GLuint	terrainTexRock;
	GLuint	terrainTexDirt;

	// Skybox
	GLuint cubeMap;
	Shader* skyboxShader;
	Mesh* quad;

	// Water
	Mesh* waterPlane;
	GLuint waterDiffuseTex;
	GLuint waterBumpTex;
	Shader* waterShader;
	float sceneTime;

	// Shadows
	GLuint shadowFBO;
	GLuint shadowTex;
	Shader* shadowShader;
	std::vector<Mesh*> sceneMeshes; // Put both these within a struct - SEE EXTRA project
	std::vector<Matrix4> sceneTransforms;

	// Palm Trees
	Shader* ptShader;
	MeshMaterial* ptMat;
	std::vector<GLuint> ptMatTextures;

	// Misc.
	Camera* camera;
	Window* window;
};