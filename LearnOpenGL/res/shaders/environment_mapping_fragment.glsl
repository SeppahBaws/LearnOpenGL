#version 450 core
out vec4 FragColor;

in vec3 Position;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 cameraPos;
uniform samplerCube skybox;
uniform sampler2D boxTexture;

void main()
{
	const float ratio = 1.00 / 1.52;
	vec3 I = normalize(Position - cameraPos);
	vec3 R = refract(I, normalize(Normal), ratio);
	FragColor = vec4(texture(skybox, R).rgb, 1.0);
}