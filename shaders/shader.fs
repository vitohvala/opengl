#version 330 core
out vec4 FragColor;

in vec3 ourColor;
in vec2 TexCoord;

// texture samplers
uniform sampler2D texture1;
uniform sampler2D texture2;

uniform float time;

vec3 palette(in float t) {
    vec3 a = vec3(0.5, 0.5, 0.5);
    vec3 b = vec3(0.5, 0.5, 0.5);
    vec3 c = vec3(1.0, 1.0, 1.0);
    vec3 d = vec3(0.263,0.416,0.557);

    return a + b*cos( 6.28318*(c*t+d) );
}

void main()
{
	// linearly interpolate between both textures (80% container, 20% awesomeface)
    vec2 p = TexCoord - vec2(0.5, 0.5);
    vec2 uv = vec2(p);

    vec3 fcolor = vec3(0.0);

    for(float i = 0.0; i < 3.0; i++) {
        p *= 1.7;
        p = fract(p);
        p -= 0.5;
        float distance = length(p);

        distance = sin(distance*8 + time);
        distance = abs(distance);

        float d = 0.85 / distance;

        vec3 color = palette(length(uv) + i * 0.4 + time * 0.4);

        fcolor += color *= d;
    }

	FragColor = mix(texture(texture1, TexCoord), texture(texture2, TexCoord), 0.2) * vec4(fcolor, 1.0f);
}


