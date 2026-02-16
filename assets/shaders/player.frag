#version 450 core

in vec3 vWorldPos;
in vec3 vLocalPos;

uniform vec3 uCameraPos;
uniform float uTime;

out vec4 FragColor;

void main()
{
    // -------- Vertical gradient --------
    float heightFactor = clamp((vLocalPos.y + 0.4) / 0.8, 0.0, 1.0);

    vec3 bottomColor = vec3(0.0, 0.8, 1.0);
    vec3 topColor    = vec3(1.0, 0.0, 0.8);

    vec3 baseColor = mix(bottomColor, topColor, heightFactor);

    // -------- Animated energy stripes --------
    float stripes = sin(vLocalPos.y * 20.0 - uTime * 4.0) * 0.5 + 0.5;
    baseColor += stripes * 0.15;

    // -------- Fresnel Rim --------
    vec3 viewDir = normalize(uCameraPos - vWorldPos);
    vec3 normalApprox = normalize(vLocalPos);
    float fresnel = pow(1.0 - max(dot(viewDir, normalApprox), 0.0), 3.0);

    vec3 rimColor = vec3(1.0);
    vec3 finalColor = baseColor + rimColor * fresnel * 0.6;

    // -------- Pulse Glow --------
    float pulse = sin(uTime * 3.0) * 0.5 + 0.5;
    finalColor *= 0.8 + pulse * 0.4;

    FragColor = vec4(finalColor, 1.0);
}
