#version 330 core

in vec2 TexCoord;
in float AO;
in float Skylight;

out vec4 FragColor;

// texture samplers
uniform sampler2D texture1;

void main()
{	
	vec4 textureColor = texture(texture1, TexCoord);
	textureColor.rgb *= AO * Skylight;  
	FragColor = textureColor;
}