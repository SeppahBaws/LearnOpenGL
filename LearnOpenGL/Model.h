#pragma once

#include <vector>
#include <assimp/scene.h>

#include "Mesh.h"

unsigned int TextureFromFile(const std::string& path, const std::string& directory, bool gamma = false);

class Model
{
public:
	Model(const std::string& path);

	void Draw(Shader shader);

private:
	void LoadModel(const std::string path);
	void ProcessNode(aiNode* pNode, const aiScene* pScene);
	Mesh ProcessMesh(aiMesh* pMesh, const aiScene* pScene);
	std::vector<Mesh::Texture> LoadMaterialTextures(aiMaterial* pMaterial, aiTextureType type, const std::string& typeName);
	
private:
	std::vector<Mesh> m_Meshes;
	std::string m_Directory;
	std::vector<Mesh::Texture> m_TexturesLoaded;
};
