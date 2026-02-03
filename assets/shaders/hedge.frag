#version 450 core
out vec4 FragColor;

in vec3 FragPos; // Received from vertex shader

// Simple hash function for procedural randomness
float hash(vec2 p) {
    p = fract(p * vec2(123.34, 456.21));
    p += dot(p, p + 45.32);
    return fract(p.x * p.y);
}

void main()
{
    // 1. Define Hedge Color
    vec3 leafColor = vec3(0.1, 0.4, 0.1);

    // 2. Generate procedural noise based on position
    // Multiply FragPos to adjust the density/size of the spots
    float noise = hash(FragPos.xy * 15.0 + FragPos.z * 5.0);

    // 3. The "See-Through" Effect
    // Adjust this threshold: 0.1 = few holes, 0.8 = mostly holes
    float threshold = 0.4;
    if (noise < threshold) {
        discard;
    }

    // 4. Basic Shading (Optional: darkens lower parts)
    float shade = mix(0.7, 1.0, fract(FragPos.y * 2.0));
    FragColor = vec4(leafColor * shade, 1.0);
}
