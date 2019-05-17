#pragma once
#include <glad/glad.h>

#include <string>

enum class TextureMode
{
	JPG,
	PNG
};

class Texture
{
public:
	Texture(const std::string& texture, TextureMode mode);

	unsigned int GetId() const { return m_Id; }

	void Use(GLenum texture);

private:
	unsigned int m_Id;
};
