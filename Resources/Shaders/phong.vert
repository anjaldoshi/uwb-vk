#version 450

layout (location = 0) in vec4 inPos;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec3 inTangent;
layout (location = 4) in vec2 inUV;

layout (binding = 0) uniform UBO 
{
	mat4 projection;
	mat4 model;
	vec4 lightPos;
} ubo;

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec3 outColor;
layout (location = 2) out vec3 outEyePos;
layout (location = 3) out vec3 outLightVec;
layout (location = 4) out vec2 outUV;

out gl_PerVertex
{
	vec4 gl_Position;
	float gl_ClipDistance[];
};

void main() 
{
	outNormal = inNormal;
	outColor = inColor;
	outUV = inUV;
	gl_Position = ubo.projection * ubo.model * inPos;
	outEyePos = vec3(ubo.model * inPos);
	outLightVec = normalize(ubo.lightPos.xyz - outEyePos);

	// Clip against reflection plane
	vec4 clipPlane = vec4(0.0, -1.0, 0.0, 1.5);	
	gl_ClipDistance[0] = dot(inPos, clipPlane);	
}
