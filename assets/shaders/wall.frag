#version 450 core

in vec3 vWorldPos;
in vec3 vLocalPos;

uniform vec3 uColor = vec3(0.8, 0.8, 0.8);

out vec4 FragColor;

void main()
{
    // Simple gradient along Y
    float factor = clamp((vLocalPos.y + 0.5) / 1.0, 0.0, 1.0);
    vec3 baseColor = mix(uColor * 0.7, uColor, factor);

    FragColor = vec4(baseColor, 1.0);
}
