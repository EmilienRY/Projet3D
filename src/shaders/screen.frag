#version 330

in vec2 uv;
out vec4 frag;

uniform sampler2D tex;
uniform float u_exposure;

void main()
{
    vec3 color = texture(tex, uv).rgb;
    color = color * u_exposure;
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));
    frag = vec4(color, 1.0);
}
