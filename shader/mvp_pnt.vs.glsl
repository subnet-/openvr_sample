#version 400

// generic shader
// gets model_matrix, view_matrix and projection matrix as uniforms
// gets positions, normals and texture coordinates as layout in
// sends texcooords and normals to fragment shader

// matrices
uniform mat4 model_matrix;
uniform mat4 view_matrix;
uniform mat4 projection_matrix;

// vertex array object
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texcoord;

// send normals and texcoords to fragment shader
out vec3 nor;
out vec2 tex;

void main(void)
{
    // calculate position in model view projection space
    gl_Position = projection_matrix * view_matrix * model_matrix * vec4(position, 1);

	// calculate normal matrix
	mat3 normal_matrix   = mat3(inverse(transpose(view_matrix * model_matrix)));

	// send normals to fragment shader
	nor = (normal_matrix * normal);

    // send texcoords to fragment shader
    tex = texcoord;
}
