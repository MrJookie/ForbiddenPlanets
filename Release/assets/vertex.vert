#version 130

in vec4 vert;

/*
uniform mat4 proj;
uniform mat4 model;
*/

in vec2 texcoord;
out vec2 texCoord;

void main()
{
	texCoord = texcoord;
    gl_Position = gl_ModelViewProjectionMatrix*vert;
}
