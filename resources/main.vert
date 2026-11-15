#version 330 core
layout (location=0) in vec3 aPos;
layout (location=1) in vec3 aNormal;
layout (location=2) in vec2 aUV;
layout (location=3) in vec3 iPos;
layout (location=4) in vec4 iCol;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;
uniform bool instanced;

out vec3 vNormal;
out vec3 vWorldpos;
out vec2 vUV;
out vec4 vInstCol;

void main() {
    vec4 wp = model * vec4(aPos, 1.0f);

    if(instanced) wp.xyz+=iPos;

    vWorldpos = wp.xyz;

    vNormal = mat3(transpose(inverse(model))) * aNormal;

    vUV = aUV;
    vInstCol = iCol;

    gl_Position = proj * view * wp;
}