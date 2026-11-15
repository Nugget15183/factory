#version 330 core
in vec3 vNormal;
in vec3 vWorldpos;
in vec2 vUV;
in vec4 vInstCol;

uniform vec3 cameraPosition;
uniform bool instanced;

uniform vec4 baseCol;
uniform bool unlit;

out vec4 FragColor;

vec3 sundir = vec3(0.4f, 1.0f, 0.3f);

void main() {
    vec4 base = instanced ? vInstCol : baseCol;
    vec4 fcol = vec4(0);

    if(!unlit) {
        vec3 n = normalize(vNormal);
        vec3 lightDir = normalize(sundir);
        float diff = max(dot(n, lightDir), 0.0);

        fcol = vec4(base.xyz * (0.2f + 0.8f * diff), base.w);
    } else {
        fcol = base;
    }

    FragColor = fcol;
}