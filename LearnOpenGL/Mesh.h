#pragma once

#include <string>
#include <vector>

#include <glm/glm.hpp>

class Shader;

struct Vertex
{
	glm::vec3 m_position;
	glm::vec3 m_normal;
	glm::vec2 m_texCoords;
};

struct Texture
{
	unsigned int m_id;
	std::string m_type;
};

class Mesh
{
public:
	Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices, std::vector<Texture> textures);

	void Draw(Shader shader);

private:
	void SetupMesh();
	
private:
	// Mesh data
	std::vector<Vertex> m_Vertices;
	std::vector<unsigned int> m_Indices;
	std::vector<Texture> m_Textures;

	// Render data
	unsigned int VAO, VBO, EBO;
};
