// Water vertex shader - pass-through with texture coordinates
varying vec2 v_texCoord;

void main()
{
    // Pass texture coordinates to fragment shader
    v_texCoord = gl_MultiTexCoord0.xy;
    
    // Transform vertex position
    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
    
    // Pass through vertex color
    gl_FrontColor = gl_Color;
}
