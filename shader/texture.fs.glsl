#version 400

// generic shader
// gets texture
// sets texture as color

// first texture
uniform sampler2D firstTexture;

// get color from vertex shader
in vec2 tex;

// send color to screen
layout(location = 0) out vec4 finalcolor;

void main(void)
{
	// apply texture
	finalcolor = texture(firstTexture, tex);
}
