#version 450 core

in vec3 vWorldPos;
in vec3 vLocalPos;

uniform vec3 uCameraPos;
uniform float uTime;
uniform bool useGlow;
uniform int colorMode; // 0 = cool, 1 = warm, 2 = neon

out vec4 FragColor;

vec3 getBaseColor(int mode, float height)
{
    if(mode == 0) return mix(vec3(0.0,0.6,1.0), vec3(0.2,1.0,0.8), height); // cool
    if(mode == 1) return mix(vec3(1.0,0.5,0.0), vec3(1.0,1.0,0.2), height); // warm
    return mix(vec3(1.0,0.0,0.8), vec3(0.0,1.0,1.0), height); // neon
}

void main()
{
    // Vertical gradient
    float hFactor = clamp((vLocalPos.y + 0.4) / 0.8, 0.0, 1.0);
    vec3 baseColor = getBaseColor(colorMode, hFactor);

    // Animated stripes
    float stripes = sin(vLocalPos.y * 15.0 + uTime * 5.0) * 0.5 + 0.5;
    baseColor += stripes * 0.1;

    // Fresnel rim
    vec3 viewDir = normalize(uCameraPos - vWorldPos);
    vec3 normalApprox = normalize(vLocalPos);
    float fresnel = pow(1.0 - max(dot(viewDir, normalApprox), 0.0), 3.0);
    baseColor += fresnel * 0.5;

    // Optional glow
    if(useGlow)
    {
        float pulse = sin(uTime * 4.0) * 0.5 + 0.5;
        baseColor *= 0.8 + pulse * 0.5;
    }

    FragColor = vec4(baseColor, 1.0);
}
