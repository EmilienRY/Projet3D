#version 330

in vec2 uv;
out vec4 frag;

uniform sampler2D tex;

void main()
{
    vec3 color = texture(tex, uv).rgb;
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));
    frag = vec4(color, 1.0);
}
