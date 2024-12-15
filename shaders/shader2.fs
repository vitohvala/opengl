#version 330 core
out vec4 FragColor;

in vec2 uv;

uniform float time;

void main()
{
    vec3 col = cos(time+uv.xyy + vec3(1, 2, 4));
	FragColor = vec4(col, 1.0f);
}


