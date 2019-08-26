#version 330 core

smooth in vec3 fragNorm;	// Interpolated model-space normal

out vec4 outCol;	// Final pixel color

void main() {
	// Visualize normals as colors
	// outCol = normalize(fragNorm) * 0.5f + vec3(0.5f);
	outCol = vec4(fragNorm, 1.0f) * 0.5f + vec4(0.5f);
}