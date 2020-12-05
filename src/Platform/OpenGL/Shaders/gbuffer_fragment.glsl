in VSout
{
	vec2 TexCoord;
	vec3 Normal;
	vec3 RenderColor;
	mat3 TBN;
	vec3 Position;
} fsin;

layout(location = 0) out vec4 OutAlbedo;
layout(location = 1) out vec4 OutNormal;
layout(location = 2) out vec4 OutMaterial;

struct Material
{
	float emmisive;
	float roughness;
	float metallic;
	float transparency;
};

uniform sampler2D map_albedo;
uniform sampler2D map_roughness;
uniform sampler2D map_metallic;
uniform sampler2D map_emmisive;
uniform sampler2D map_normal;
uniform sampler2D map_occlusion;
uniform Material material;
uniform vec2 uvMultipliers;
uniform float gamma;

vec3 calcNormal(vec2 texcoord, mat3 TBN, sampler2D normalMap)
{
	vec3 normal = texture(normalMap, texcoord).rgb;
	normal = normalize(normal * 2.0f - 1.0f);
	return TBN * normal;
}

void main()
{
	vec2 TexCoord = uvMultipliers * fsin.TexCoord;

	vec4 albedoAlphaTex = texture(map_albedo, TexCoord).rgba;
	if (albedoAlphaTex.a < 0.5f) discard; // mask fragments with low opacity

	vec3 normal = calcNormal(TexCoord, fsin.TBN, map_normal);

	vec3 albedoTex = albedoAlphaTex.rgb;
	float occlusion = texture(map_occlusion, TexCoord).r;
	float emmisiveTex = texture(map_emmisive, TexCoord).r;
	float metallicTex = texture(map_metallic, TexCoord).r;
	float roughnessTex = texture(map_roughness, TexCoord).r;

	float emmisive = material.emmisive * emmisiveTex;
	float roughness = material.roughness * roughnessTex;
	float metallic = material.metallic * metallicTex;

	vec3 albedo = pow(fsin.RenderColor * albedoTex, vec3(gamma));

	OutAlbedo = vec4(fsin.RenderColor * albedo, occlusion);
	OutNormal = vec4(0.5f * normal + 0.5f, 1.0f);
	OutMaterial = vec4(emmisive / (emmisive + 1.0f), roughness, metallic, 1.0f);
}