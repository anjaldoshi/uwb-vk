#version 450

layout(location = 0) in vec4 inPos;
layout(location = 1) in vec3 inColor;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec2 inUV;

layout (binding = 0) uniform UBO 
{
	mat4 projection;
	mat4 model;
} ubo;

layout (location = 0) out vec2 outUV;
layout (location = 1) out vec4 outPos;

out gl_PerVertex
{
	vec4 gl_Position;
};

void main() 
{
	outUV = inUV;
	outPos = ubo.projection * ubo.model * vec4(inPos.xyz, 1.0);;
	gl_Position = outPos;		
}
