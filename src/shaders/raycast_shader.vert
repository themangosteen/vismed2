#version 330 core

in vec3 vertexPosition;

// entry positions for rays through the volume
// these will be interpolated by the gpu and then accessible by same name in fragment shader
// since we already have a texture of exit positions (interpolated volume cube backface vertices) from another shader
// we get all the possible rays (entry-exit pairs) through the volume
// entry and exit positions shall be interpolated model space vertex positions (stored as color) of a cube in range [0,1],
// since they will be used to sample the volume texture (tex coords normalized to [0,1])
// note that since we already have the backface exit positions back face culling should be enabled here
out vec3 entryPos;

// uniforms use the same value for all vertices
uniform mat4 modelViewProjMat;

void main()
{
    entryPos = vertexPosition; // store volume cube front face vertex positions in color information
    gl_Position = modelViewProjMat * vec4(vertexPosition, 1.0);
}
