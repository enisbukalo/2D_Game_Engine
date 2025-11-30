// Procedural ocean water fragment shader
uniform float u_time;
uniform vec2 u_resolution;

varying vec2 v_texCoord;

// Hash function for pseudo-random noise
float hash(vec2 p)
{
    return fract(sin(dot(p, vec2(127.1, 311.7))) * 43758.5453123);
}

// 2D Perlin-like noise function
float noise(vec2 p)
{
    vec2 i = floor(p);
    vec2 f = fract(p);
    
    // Smooth interpolation
    f = f * f * (3.0 - 2.0 * f);
    
    // Four corners of the grid cell
    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));
    
    // Bilinear interpolation
    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

// Fractal Brownian Motion - layered noise
float fbm(vec2 p, int octaves)
{
    float value = 0.0;
    float amplitude = 0.5;
    float frequency = 1.0;
    
    for(int i = 0; i < octaves; i++)
    {
        value += amplitude * noise(p * frequency);
        amplitude *= 0.5;
        frequency *= 2.0;
    }
    
    return value;
}

void main()
{
    // Normalized pixel coordinates (0.0 to 1.0)
    vec2 uv = gl_FragCoord.xy / u_resolution;
    
    // Time-based animation
    float time = u_time * 0.3;
    
    // Create base noise pattern for natural water texture
    vec2 noiseUV1 = uv * 8.0 + vec2(time * 0.15, time * 0.1);
    vec2 noiseUV2 = uv * 12.0 - vec2(time * 0.1, time * 0.2);
    vec2 noiseUV3 = uv * 20.0 + vec2(time * 0.08, -time * 0.15);
    
    float n1 = fbm(noiseUV1, 3);
    float n2 = fbm(noiseUV2, 3);
    float n3 = fbm(noiseUV3, 2);
    
    // Add horizontal wave motion influenced by noise
    float horizontalWave = sin((uv.y + n1 * 0.1) * 60.0 + time * 1.5 + uv.x * 8.0) * 0.5 + 0.5;
    
    // Combine noise layers with subtle horizontal bias
    float waves = (n1 * 0.35 + n2 * 0.25 + n3 * 0.15 + horizontalWave * 0.25);
    
    // Ocean color palette - deep to shallow water
    vec3 deepWater = vec3(0.0, 0.15, 0.35);      // Deep blue
    vec3 shallowWater = vec3(0.0, 0.4, 0.6);     // Medium blue
    vec3 surfaceWater = vec3(0.2, 0.55, 0.7);    // Light blue-green
    
    // Create depth gradient based on wave patterns
    float depth = waves * 0.8 + 0.2;
    vec3 waterColor = mix(deepWater, shallowWater, depth);
    waterColor = mix(waterColor, surfaceWater, pow(depth, 2.0) * 0.5);
    
    // Add foam on wave peaks
    float foamThreshold = 0.55;  // Lower threshold for more foam
    float foam = smoothstep(foamThreshold, foamThreshold + 0.1, waves);
    
    // Add sharp white foam tips for breaking waves
    float waveBreaks = smoothstep(0.7, 0.75, waves);
    float sharpFoam = pow(waveBreaks, 2.0);
    
    vec3 foamColor = vec3(0.95, 0.98, 1.0);
    waterColor = mix(waterColor, foamColor, foam * 0.5);
    waterColor = mix(waterColor, vec3(1.0, 1.0, 1.0), sharpFoam * 0.9);  // Bright white tips
    
    // Caustics effect - light patterns underwater
    vec2 causticUV = uv * 8.0 + time * 0.1;
    float caustic1 = fbm(causticUV, 3);
    float caustic2 = fbm(causticUV * 1.3 + vec2(1.7, 2.3), 3);
    float caustics = pow(abs(sin(caustic1 * 3.14159) * sin(caustic2 * 3.14159)), 3.0);
    waterColor += caustics * vec3(0.15, 0.25, 0.3) * depth;
    
    // Specular highlights - sun reflection
    vec2 specUV = uv * 6.0 - vec2(time * 0.3, time * 0.2);
    float specNoise = fbm(specUV, 2);
    float specular = pow(specNoise, 6.0) * 1.5;
    waterColor += vec3(specular * 0.8, specular * 0.9, specular);
    
    // Subtle wave motion displacement
    float waveDisplacement = sin(uv.x * 15.0 + time * 2.0) * cos(uv.y * 12.0 + time * 1.5);
    waterColor += waveDisplacement * 0.03;
    
    // Final color output
    gl_FragColor = vec4(waterColor, 1.0);
}
