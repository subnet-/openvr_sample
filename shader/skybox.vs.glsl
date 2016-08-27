#version 400

uniform mat4 projection_matrix;
uniform mat4 model_matrix;
uniform mat4 view_matrix;

// get position from vertex array object
layout(location = 0) in vec3 pos;

// send color to fragment shader
out vec3 vcolor;

void main(void)
{
    // calculate position in model view projection space

	vec4 position = projection_matrix * view_matrix * vec4(pos, 1);
    gl_Position = position.xyww;

    // set local position as color
    vcolor = pos;
}
