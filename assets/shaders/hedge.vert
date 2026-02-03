#version 450 core
layout (location = 0) in vec3 aPos;

uniform mat4 uModel;
uniform mat4 uView;
uniform mat4 uProj;

// Pass position to fragment shader
out vec3 FragPos;

void main()
{
    FragPos = aPos;
    gl_Position = uProj * uView * uModel * vec4(aPos, 1.0);
}
