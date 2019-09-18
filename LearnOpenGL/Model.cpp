#include "Model.h"

#include <glad/glad.h>
#include "stb_image.h"

#include "Shader.h"
#include "helpers/Logger.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

unsigned int TextureFromFile(const std::string& path, const std::string& directory, bool gamma)
{
	std::string filename = directory + '/' + path;

	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
		{
			format = GL_RED;
		}
		else if (nrComponents == 3)
		{
			format = GL_RGB;
		}
		else if (nrComponents == 4)
		{
			format = GL_RGBA;
		}

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	}
	else
	{
		std::stringstream ss;
		ss << "Texture failed to load at path: " << path;
		Logger::LogError(ss.str());
	}

	stbi_image_free(data);
	return textureID;
}

Model::Model(const std::string& path)
{
	LoadModel(path);
}

void Model::Draw(Shader shader)
{
	for (Mesh& mesh : m_Meshes)
	{
		mesh.Draw(shader);
	}
}

void Model::LoadModel(const std::string path)
{
	Assimp::Importer importer;
	const aiScene* pScene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs);

	if (!pScene || pScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !pScene->mRootNode)
	{
		std::stringstream ss;
		ss << "ERROR::ASSIMP::" << importer.GetErrorString();
		Logger::LogError(ss.str());

		return;
	}

	m_Directory = path.substr(0, path.find_last_of('/'));

	ProcessNode(pScene->mRootNode, pScene);
}

void Model::ProcessNode(aiNode* pNode, const aiScene* pScene)
{
	// Process all the node's meshes (if any)
	for (unsigned int i = 0; i < pNode->mNumMeshes; i++)
	{
		aiMesh* pMesh = pScene->mMeshes[pNode->mMeshes[i]];
		m_Meshes.push_back(ProcessMesh(pMesh, pScene));
	}

	// Then do the same for each of its children
	for (unsigned int i = 0; i < pNode->mNumChildren; i++)
	{
		ProcessNode(pNode->mChildren[i], pScene);
	}
}

Mesh Model::ProcessMesh(aiMesh* pMesh, const aiScene* pScene)
{
	std::vector<Mesh::Vertex> vertices;
	std::vector<unsigned int> indices;
	std::vector<Mesh::Texture> textures;

	for (unsigned int i = 0; i < pMesh->mNumVertices; i++)
	{
		Mesh::Vertex vertex;
		// process vertex positions, normals and texture coordinates

		glm::vec3 vector;

		// positions
		vector.x = pMesh->mVertices[i].x;
		vector.y = pMesh->mVertices[i].y;
		vector.z = pMesh->mVertices[i].z;
		vertex.m_position = vector;

		// normals
		vector.x = pMesh->mNormals[i].x;
		vector.y = pMesh->mNormals[i].y;
		vector.z = pMesh->mNormals[i].z;
		vertex.m_normal = vector;

		// texture coordinates
		if (pMesh->mTextureCoords[0]) // check if the mesh contains texture coordinates
		{
			glm::vec2 vec;
			vec.x = pMesh->mTextureCoords[0][i].x;
			vec.y = pMesh->mTextureCoords[0][i].y;
			vertex.m_texCoords = vec;
		}
		else
		{
			vertex.m_texCoords = glm::vec2(0.0f, 0.0f);
		}
		
		vertices.push_back(vertex);
	}

	// process indices
	for (unsigned int i = 0; i < pMesh->mNumFaces; i++)
	{
		aiFace face = pMesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}
	
	// process material
	if (pMesh->mMaterialIndex >= 0)
	{
		aiMaterial* pMaterial = pScene->mMaterials[pMesh->mMaterialIndex];

		std::vector<Mesh::Texture> diffuseMaps = LoadMaterialTextures(pMaterial, aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

		std::vector<Mesh::Texture> specularMaps = LoadMaterialTextures(pMaterial, aiTextureType_SPECULAR, "texture_specular");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
	}

	return Mesh(vertices, indices, textures);
}

std::vector<Mesh::Texture> Model::LoadMaterialTextures(aiMaterial* pMaterial, aiTextureType type, const std::string& typeName)
{
	std::vector<Mesh::Texture> textures;
	for (unsigned int i = 0; i < pMaterial->GetTextureCount(type); i++)
	{
		aiString string;
		pMaterial->GetTexture(type, i, &string);

		Logger::LogWarning("Textures:");
		Logger::LogWarning(string.C_Str());
		
		bool skip = false;
		for (unsigned int j = 0; j < m_TexturesLoaded.size(); j++)
		{
			if (std::strcmp(m_TexturesLoaded[j].m_path.data(), string.C_Str()) == 0)
			{
				textures.push_back(m_TexturesLoaded[j]);
				skip = true;
				break;
			}
		}
		if (!skip)
		{
			// if texture hasn't been loaded already, load it.
			Mesh::Texture texture;
			texture.m_id = TextureFromFile(string.C_Str(), m_Directory);
			texture.m_type = typeName;
			texture.m_path = string.C_Str();
			textures.push_back(texture);
		}
	}

	return textures;
}
