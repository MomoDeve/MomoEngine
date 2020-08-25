#include "directional_light.glsl"
EMBEDDED_SHADER(

out vec4 OutColor;
in vec2 TexCoord;

struct Camera
{
	vec3 position;
	mat4 invProjMatrix;
	mat4 invViewMatrix;
	mat4 viewProjMatrix;
};

uniform sampler2D albedoTex;
uniform sampler2D normalTex;
uniform sampler2D materialTex;
uniform sampler2D depthTex;

uniform int lightCount;
uniform int pcfDistance;
uniform Camera camera;
uniform mat3 skyboxTransform;
uniform samplerCube skyboxTex;

const int MaxLightCount = 4;
uniform DirLight lights[MaxLightCount];
uniform sampler2D lightDepthMaps[MaxLightCount];

void main()
{
	FragmentInfo fragment = getFragmentInfo(TexCoord, albedoTex, normalTex, materialTex, depthTex, camera.invViewMatrix, camera.invProjMatrix);
	float fragDistance = length(camera.position - fragment.position);

	vec3 viewDirection = normalize(camera.position - fragment.position);
	vec3 reflectionColor = calcReflectionColor(fragment.reflection, skyboxTex, skyboxTransform, viewDirection, fragment.normal);

	vec3 totalColor = vec3(0.0f);
	for (int i = 0; i < lightCount; i++)
	{
		vec4 fragLightSpace = lights[i].transform * vec4(fragment.position, 1.0f);
		totalColor += calcColorUnderDirLight(fragment, reflectionColor, lights[i], viewDirection, pcfDistance, fragLightSpace, lightDepthMaps[i]);
	}

	OutColor = vec4(totalColor, 1.0f);
}

)