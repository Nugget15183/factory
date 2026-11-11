#version 330 core
in vec3 vNormal;
in vec3 vWorldpos;
in vec2 vUV;

uniform vec3 camPos;

out vec4 FragColor;

vec3 sundir = vec3(0.4f, 1.0f, 0.3f);
vec3 base = vec3(0.8f, 0.6f, 0.3f);

void main() {
    vec3 n = normalize(vNormal);

    vec3 lightDir = normalize(sundir);

    float diff = max(dot(n, lightDir), 0.0);
    vec3 baseCol = vec3(0.8f, 0.6f, 0.3f);

    FragColor = vec4(base * (0.2f + 0.8f * diff), 1.0f);
}