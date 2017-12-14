#version 330 core

in vec3 color;

// out location 0 is piped to the default draw buffer
out vec4 outColor;

void main()
{
    outColor = vec4(color, 1.0);
}
