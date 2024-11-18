#version 430 core

#define MAX_PARTICLES 5000

layout(binding = 0, std140) uniform ParticlesBlock
{
	vec2 particles[MAX_PARTICLES];
};

uniform int particleCount;
uniform float mass;
uniform float scale;
uniform vec2 iResolution;
uniform float targetDensity;
uniform float particleRadius;
uniform float particleInfluenceRadius;

out vec4 color;

float transform_vals(float val)
{
	return val / iResolution.x;
}

vec2 transform_coord(vec2 coord)
{
	return (2.0 * coord.xy - iResolution) / iResolution.x;
}

void main()
{
	vec2 xy = transform_coord(gl_FragCoord.xy);
	color = vec4(1.0, 1.0, 1.0, 1.0);

	float radius = transform_vals(particleRadius);
	float influence = transform_vals(particleInfluenceRadius);
	float inside = 0.0;

	for (int i = 0; i < particleCount; ++i)
	{
		vec2 particle = transform_coord(particles[i] * scale);
		float dst = length(xy - particle);
	    if (radius >= dst) inside += 1.0;
	}
	if (inside > 0.0)
	{
		color.g *= 0.0;
		color.b *= 1.0 - smoothstep(1.0, 4.0, inside);
		color.r *= smoothstep(1.0, 4.0, inside);
	}
} 