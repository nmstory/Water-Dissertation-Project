#include "Renderer.h"
#include "../nclgl/camera.h"
#include "../nclgl/HeightMap.h"

#define SHADOWSIZE 2048

Renderer::Renderer(Window& parent) : OGLRenderer(parent) {
	window = &parent;

	//heightMap = new HeightMap(TEXTUREDIR"noise.png");
	quad = Mesh::GenerateQuad();
	

	heightMap = new HeightMap(TEXTUREDIR"tenerifeINVERT.png");
	sceneMeshes.push_back(heightMap);
	/*
	cubeMap = SOIL_load_OGL_cubemap(
		TEXTUREDIR"sky_west.jpg", TEXTUREDIR"sky_east.jpg",
		TEXTUREDIR"sky_up.jpg", TEXTUREDIR"sky_down.jpg",
		TEXTUREDIR"sky_south.jpg", TEXTUREDIR"sky_north.jpg",
		SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);*/
	
	cubeMap = SOIL_load_OGL_cubemap(
		TEXTUREDIR"SKY_WEST1.BMP", TEXTUREDIR"SKY_EAST1.BMP",
		TEXTUREDIR"SKY_UP1.BMP", TEXTUREDIR"SKY_DOWN1.BMP",
		TEXTUREDIR"SKY_SOUTH1.BMP", TEXTUREDIR"SKY_NORTH1.BMP",
		SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);

	

	if (!heightMap || !cubeMap) {
		return;
	}

	Vector3 heightmapSize = heightMap->GetHeightmapSize();
	//light = new Light(heightmapSize * Vector3(0.5f, 5.0f, 0.5f), Vector4(0.9, 0.9, 0.9, 1), 15000);
	light = new Light(heightmapSize * Vector3(0.5f, 5.0f, 0.5f), Vector4(1, 0.98431373, 0.91372549, 0.9), 20000);
	waterPlane = Mesh::GenerateWaterPlane(heightmapSize * Vector3(0.5, 0.2, 0.5), 5000);

	std::queue<Vector3> cameraTrack; // temp!!
	cameraTrack.emplace(Vector3(0, 1, 1));
	camera = new Camera(-40, 270, heightmapSize * Vector3(0.5, 2, 0.5), cameraTrack);
	//camera = new Camera(-40, 270, Vector3(0,0,0), cameraTrack);

	heightMapShader = new Shader("heightmapVertex.glsl", "heightmapFragment.glsl");
	skyboxShader = new Shader("skyboxVertex.glsl", "skyboxFragment.glsl");
	waterShader = new Shader("WaterVertex.glsl", "WaterFrag.glsl", "GeometryShader.glsl", "TCS.glsl", "TES.glsl");
	shadowShader = new Shader("shadowVert.glsl", "shadowFrag.glsl");
	ptShader = new Shader("PalmTreeVertex.glsl", "PalmTreeFragment.glsl");

	if (!heightMapShader->LoadSuccess() || !skyboxShader->LoadSuccess() || !waterShader->LoadSuccess() || !shadowShader->LoadSuccess() || !ptShader->LoadSuccess()) {
		return;
	}

	//terrainTex = SOIL_load_OGL_texture(TEXTUREDIR"Barren Reds.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	terrainTexDirt = SOIL_load_OGL_texture(TEXTUREDIR"Dirt_DIFF.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	terrainTexRock = SOIL_load_OGL_texture(TEXTUREDIR"Rock_04_DIFF.PNG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	bumpMapDirt = SOIL_load_OGL_texture(TEXTUREDIR"Dirt_NRM.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	bumpMapRock = SOIL_load_OGL_texture(TEXTUREDIR"Rock_04_NRM.PNG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	
	//waterDiffuseTex = SOIL_load_OGL_texture(TEXTUREDIR"Water_002_COLOR.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	//waterBumpTex = SOIL_load_OGL_texture(TEXTUREDIR"Water_002_NORM.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	waterDiffuseTex = SOIL_load_OGL_texture(TEXTUREDIR"water.TGA", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	waterBumpTex = SOIL_load_OGL_texture(TEXTUREDIR"waterbump.PNG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	if (!terrainTexDirt || !terrainTexRock || !bumpMapDirt || !bumpMapRock || !waterDiffuseTex || !waterBumpTex) {
		return;
	}

	SetTextureRepeating(terrainTexDirt, true);
	SetTextureRepeating(terrainTexRock, true);
	SetTextureRepeating(bumpMapDirt, true);
	SetTextureRepeating(bumpMapRock, true);

	projMatrix = Matrix4::Perspective(1.0f, 10000.0f, (float)width / (float)height, 45.0f);

	// SceneNode
	root = new SceneNode();
	SceneNode* heightMapNode = new SceneNode();
	heightMapNode->SetShader(heightMapShader);
	heightMapNode->SetMesh(heightMap);
	heightMapNode->AddTexture("diffuseTexDirt", terrainTexDirt);
	heightMapNode->AddTexture("diffuseTexRock", terrainTexRock);
	heightMapNode->AddTexture("bumpTexDirt", bumpMapDirt);
	heightMapNode->AddTexture("bumpTexRock", bumpMapRock);
	heightMapNode->SetLight(light);
	heightMapNode->SetBoundingRadius(10000);
	root->AddChild(heightMapNode);

	/****  Shadows  ****/
	glGenTextures(1, &shadowTex);
	glBindTexture(GL_TEXTURE_2D, shadowTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// GL_DEPTH_COMPONENT gives us 32bits of precision for depth buffer, rather than splitting (fancy word for it) to give some bits to stencil
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOWSIZE, SHADOWSIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glBindTexture(GL_TEXTURE_2D, 0);

	// Code to create an FBO and it's depth attachment
	glGenFramebuffers(1, &shadowFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowTex, 0);
	glDrawBuffer(GL_NONE); // GL_NONE here means we don't actually need any colour information for our shadow map pass - so we dont have a colour attachment at all
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//sceneMeshes.emplace_back(Mesh::LoadFromMeshFile("palm_tree.msh"));
	//ptMat = new MeshMaterial("palm_tree.mat");
	sceneMeshes.push_back(Mesh::LoadFromMeshFile("palm_tree.msh"));
	ptMat = new MeshMaterial("palm_tree.mat");

	auto matEntry = ptMat->GetMaterialForLayer(0);

	const std::string* filename = nullptr;
	matEntry->GetEntry("Diffuse", &filename);

	string path = TEXTUREDIR + *filename;
	GLuint texID = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
	ptMatTextures.emplace_back(texID);

	filename = nullptr;
	matEntry->GetEntry("Bump", &filename);

	path = TEXTUREDIR + *filename;
	texID = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
	ptMatTextures.emplace_back(texID);

	sceneTransforms.resize(2);
	sceneTransforms[1] = Matrix4::Translation(Vector3(8000, 200, 8000));


	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_BACK);

	// Which to remove?
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

	sceneTime = 0;
	init = true;
}

Renderer::~Renderer(void) {
	// check we definitely delete everything!
	delete heightMap;
	delete camera;
	delete heightMapShader;
	delete quad;
	delete light;
}

void Renderer::UpdateScene(float dt) {
	camera->UpdateCamera(dt);
	viewMatrix = camera->BuildViewMatrix();
	sceneTime += dt;
}

void Renderer::BuildNodeLists(SceneNode* from) {
	/*
	if (frameFrustum.InsideFrustum(*from)) {
		Vector3 dir = from->GetWorldTransform().GetPositionVector() - camera->GetPosition();
		from->SetCameraDistance(Vector3::Dot(dir, dir));

		if (from->GetColour().w < 1.0f) {
			transparentNodeList.push_back(from);
		}
		else {
			nodeList.push_back(from);
		}
	}*/

	for (std::vector<SceneNode*>::const_iterator i = from->GetChildIteratorStart(); i != from->GetChildIteratorEnd(); ++i) {
		nodeList.push_back(*i);
		BuildNodeLists(*i);
	}
}

void Renderer::SortNodeLists() {
	// Once the node lists have been built, they now need to be sort

	std::sort(transparentNodeList.rbegin(), transparentNodeList.rend(), SceneNode::CompareByCameraDistance); // rbegin and rend means reverse
	std::sort(nodeList.begin(), nodeList.end(), SceneNode::CompareByCameraDistance);
}

void Renderer::ClearNodeLists() {
	transparentNodeList.clear();
	nodeList.clear();
}

/*
	Drawing scene graph using two for loops - opaque nodes first, then transparent nodes.
	Since we used the reverse iterators to sort the transprent nodes, they'll be drawn from furthers to closest just by ranging over the vector, same as the opaque nodes.
*/
void Renderer::DrawNodes() {
	for (const auto& i : nodeList) {
		DrawNode(i);
	}

	for (const auto& i : transparentNodeList) {
		DrawNode(i);
	}
}

void Renderer::DrawNode(SceneNode* n) {
	BindShader(n->GetShader());
	if (n->GetMesh()) {
		viewMatrix = camera->BuildViewMatrix();
		projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width / (float)height, 45.0f);

		if (n->GetShader()) BindShader(n->GetShader());
		if (n->GetLight()) SetShaderLight(*n->GetLight());

		glUniform1i(glGetUniformLocation(n->GetShader()->GetProgram(), "shadowTex"), 2);
		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, shadowTex);

		UpdateShaderMatrices();

		n->Draw(*this);

		/*
		Matrix4 model = n->GetWorldTransform() * Matrix4::Scale(n->GetModelScale());
		glUniformMatrix4fv(glGetUniformLocation(heightMapShader->GetProgram(), "modelMatrix"), 1, false, model.values);

		glUniform4fv(glGetUniformLocation(heightMapShader->GetProgram(), "nodeColour"), 1, (float*)&n->GetColour());

		GLuint texture = n->GetTexture();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);

		glUniform1i(glGetUniformLocation(heightMapShader->GetProgram(), "useTexture"), texture);

		n->Draw(*this);*/
	}
}

void Renderer::RenderScene() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	
	DrawSkybox(); // Need to draw skybox first!


	//DrawShadowScene();
	//DrawShadowObjects();

	
	BuildNodeLists(root);
	DrawNodes();
	ClearNodeLists();
	
	//DrawHeightmap();
	DrawWater();
}

void Renderer::DrawSkybox() {
	glDepthMask(GL_FALSE);

	BindShader(skyboxShader);
	UpdateShaderMatrices();

	quad->Draw();

	glDepthMask(GL_TRUE);
}
/*
void Renderer::DrawHeightmap() {
	// Heightmap
	BindShader(heightMapShader);
	
	glUniform1i(glGetUniformLocation(heightMapShader->GetProgram(), "diffuseTexDirt"), 0);
	glUniform1i(glGetUniformLocation(heightMapShader->GetProgram(), "diffuseTexRock"), 1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, terrainTexDirt);
	glGenerateMipmap(GL_TEXTURE_2D); // could do this without binding by calling glGenerateTextureMipMap (maybe in renderer constructor?) https://www.khronos.org/registry/OpenGL-Refpages/gl4/html/glGenerateMipmap.xhtml
	glActiveTexture(GL_TEXTURE0 + 1);
	glBindTexture(GL_TEXTURE_2D, terrainTexRock);

	// Lighting
	glUniform1i(glGetUniformLocation(heightMapShader->GetProgram(), "bumpTexDirt"), 2);
	glUniform1i(glGetUniformLocation(heightMapShader->GetProgram(), "bumpTexRock"), 3);
	glActiveTexture(GL_TEXTURE0 + 2);
	glBindTexture(GL_TEXTURE_2D, bumpMapDirt);
	glActiveTexture(GL_TEXTURE0 + 3);
	glBindTexture(GL_TEXTURE_2D, bumpMapRock);

	glUniform3fv(glGetUniformLocation(heightMapShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());
	
	// Extra
	SetShaderLight(*light);
	UpdateShaderMatrices();
	heightMap->Draw();
}*/

void Renderer::DrawWater() {
	/*
	GLint MaxPatchVertices = 0;
	glGetIntegerv(GL_MAX_PATCH_VERTICES, &MaxPatchVertices);
	std::cout << "The maximum supported patch vertices: " << MaxPatchVertices << std::endl;
	*/
	BindShader(waterShader);

	//glPatchParameteri(GL_PATCH_VERTICES, 4);
	//glDrawArrays(GL_PATCHES, 0, 8);
	//glDrawArraysInstanced(GL_PATCHES, 0, 6, 64 * 64);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	SetShaderLight(*light);
	glUniform1f(glGetUniformLocation(waterShader->GetProgram(), "time"), sceneTime);

	glUniform1i(glGetUniformLocation(waterShader->GetProgram(), "screenWidth"), window->GetScreenSize().x);
	glUniform1i(glGetUniformLocation(waterShader->GetProgram(), "screenHeight"), window->GetScreenSize().y);

	glUniform1i(glGetUniformLocation(waterShader->GetProgram(), "diffuseWater"), window->GetScreenSize().x);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, waterDiffuseTex);
	glUniform1i(glGetUniformLocation(waterShader->GetProgram(), "bumpWater"), window->GetScreenSize().y);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, waterBumpTex);

	glPatchParameteri(GL_PATCH_VERTICES, 4);


	UpdateShaderMatrices();
	waterPlane->Draw();

	// Turn off wireframe mode
	//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);


	/*
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
	quad->Draw();
	*/
}
	

// SetShaderLight (* light); //No lighting in this shader!

void Renderer::DrawShadowScene() {
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glClear(GL_DEPTH_BUFFER_BIT);

	glViewport(0, 0, SHADOWSIZE, SHADOWSIZE);

	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	BindShader(shadowShader);

	viewMatrix = Matrix4::BuildViewMatrix(light->GetPosition(), Vector3(0, 0, 0));
	//projMatrix = Matrix4::Perspective(1, 100, 1, 45);
	projMatrix = Matrix4::Perspective(1, 15000,(float) width/ (float)height, 45);
	shadowMatrix = projMatrix * viewMatrix; //used later

	Matrix4 tempmm = modelMatrix;
	for (int i = 0; i < 2; ++i) {
		modelMatrix = sceneTransforms[i];
		UpdateShaderMatrices();
		sceneMeshes[i]->Draw();
	}
	modelMatrix = tempmm;

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glViewport(0, 0, width, height);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer::DrawShadowObjects() {
	// i.e. palm trees
	BindShader(ptShader);
	SetShaderLight(*light);
	viewMatrix = camera->BuildViewMatrix();
	projMatrix = Matrix4::Perspective(1.0f, 15000.0f, (float)width / (float)height, 45.0f);

	glUniform1i(glGetUniformLocation(ptShader->GetProgram(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(ptShader->GetProgram(), "bumpTex"), 1);
	glUniform1i(glGetUniformLocation(ptShader->GetProgram(), "shadowTex"), 2);

	glUniform3fv(glGetUniformLocation(ptShader->GetProgram(), "cameraPos"), 3, (float*)&camera->GetPosition());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ptMatTextures[0]);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, ptMatTextures[1]);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, shadowTex);

	Matrix4 tempmm = modelMatrix;
	for (int i = 0; i < 2; ++i) {
		modelMatrix = sceneTransforms[i];
		UpdateShaderMatrices();
		sceneMeshes[i]->Draw();
	}
	modelMatrix = tempmm;
}