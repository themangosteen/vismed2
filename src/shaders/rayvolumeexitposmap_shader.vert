#version 330 core

in vec3 vertexPosition;

// out attributes are interpolated for each fragment and then accessible in fragment shader
// here we store model vertex positions in color information to interpolate
// between all possible positions where a ray might leave the volume
// the idea is to use model space vertex positions of a cube in range [0,1], and store the
// interpolated colors since they will later then be used to sample a volume texture (tex coords normalized to [0,1])
// note that obviously we want only the vertices of volume cube backfaces, thus front face culling must be enabled.
out vec3 color;

// uniforms use the same value for all vertices
uniform mat4 modelViewProjMat;

void main()
{
    color = vertexPosition; // store volume cube backface vertex model space positions in color information
    gl_Position = modelViewProjMat * vec4(vertexPosition, 1.0);
}
