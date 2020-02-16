#version 450 core
out vec4 FragColor;

uniform vec3 cameraPos;
uniform samplerCube skybox;
uniform sampler2D boxTexture;

in VS_OUT
{
	vec3 Position;
	vec3 Normal;
	vec2 TexCoords;
} fs_in;

void main()
{
	const float ratio = 1.00 / 1.52;
	vec3 I = normalize(fs_in.Position - cameraPos);
	vec3 R = refract(I, normalize(fs_in.Normal), ratio);
	FragColor = vec4(texture(skybox, R).rgb, 1.0);
}