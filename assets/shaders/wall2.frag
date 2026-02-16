#version 450 core

in vec3 vWorldPos;
in vec3 vLocalPos;

uniform vec3 uCameraPos;
uniform float uTime;
uniform bool useGlow;
uniform int colorMode; // 0=cool, 1=warm, 2=neon

out vec4 FragColor;

// Utility: vertical gradient
vec3 getBaseColor(int mode, float height)
{
    if(mode == 0) return mix(vec3(0.2,0.2,0.8), vec3(0.0,0.5,1.0), height); // cool
    if(mode == 1) return mix(vec3(0.5,0.2,0.0), vec3(1.0,0.5,0.2), height); // warm
    return mix(vec3(0.8,0.0,1.0), vec3(0.0,1.0,1.0), height); // neon
}

// Simple brick pattern
float brickPattern(vec3 pos)
{
    float brickHeight = 0.2;
    float brickWidth  = 0.5;
    float offset = mod(floor(pos.y / brickHeight), 2.0) * 0.5; // staggered rows
    float xPos = mod(pos.x + offset, brickWidth);
    float yPos = mod(pos.y, brickHeight);
    float edge = 0.05;
    return step(edge, xPos) * step(edge, brickWidth - xPos) * step(edge, yPos) * step(edge, brickHeight - yPos);
}

void main()
{
    // Gradient
    float hFactor = clamp((vLocalPos.y + 0.05) / 1.0, 0.0, 1.0);
    vec3 color = getBaseColor(colorMode, hFactor);

    // Brick effect
    float bricks = brickPattern(vLocalPos);
    color *= bricks;

    // Fresnel rim for subtle glow on edges
    vec3 viewDir = normalize(uCameraPos - vWorldPos);
    vec3 normalApprox = normalize(vLocalPos);
    float fresnel = pow(1.0 - max(dot(viewDir, normalApprox), 0.0), 2.0);
    if(useGlow) color += fresnel * 0.3;

    FragColor = vec4(color, 1.0);
}
