#version 450 core

in vec3 vWorldPos;

out vec4 FragColor;

void main()
{
    // Vertical gradient: bottom = purple, top = cyan
    float height = clamp(vWorldPos.y / 1.0, 0.0, 1.0); // normalize by player height
    vec3 baseColor = mix(vec3(0.6, 0.0, 0.8), vec3(0.0, 1.0, 1.0), height);

    // Radial stripes effect using XZ distance from center
    float radius = length(vWorldPos.xz - vec2(0.5, 0.5)); // approximate local cylinder center
    float stripes = abs(sin(radius * 20.0)); // more stripes = 20.0

    vec3 finalColor = baseColor * mix(0.7, 1.3, stripes);

    FragColor = vec4(finalColor, 1.0);
}
