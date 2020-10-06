#include "Library/shader_utils.glsl"
EMBEDDED_SHADER(

in vec2 TexCoord;
out vec4 OutColor;

struct Camera
{
    vec3 position;
    mat4 viewProjMatrix;
    mat4 invViewProjMatrix;
};

uniform sampler2D albedoTex;
uniform sampler2D normalTex;
uniform sampler2D materialTex;
uniform sampler2D depthTex;
uniform sampler2D HDRTex;

uniform samplerCube skyboxMap;
uniform mat3 skyboxTransform;
uniform Camera camera;

uniform int   steps;
uniform float thickness;
uniform float maxDistance;
uniform float maxCosAngle;

vec4 toFragSpace(vec4 v, mat4 viewProj)
{
    vec4 proj = viewProj * v;
    proj.xyz /= proj.w;
    proj.xy = proj.xy * 0.5f + vec2(0.5f);
    return proj;
}

void main()
{
    FragmentInfo fragment = getFragmentInfo(TexCoord, albedoTex, normalTex, materialTex, depthTex, camera.invViewProjMatrix);
    vec3 objectColor = texture(HDRTex, TexCoord).rgb;
    if (fragment.reflection == 0.0f)
    {
        OutColor = vec4(objectColor, 1.0f);
        return;
    }

    vec3 viewDistance = camera.position - fragment.position;
    vec3 viewDirection = normalize(viewDistance);

    vec3 pivot = normalize(reflect(-viewDirection, fragment.normal));
    vec3 startPos = fragment.position + (pivot * 0.001f);

    float currentLength = 1.0f;
    float bestDepth = 10000.0f;
    vec2 bestUV = vec2(0.0f);
    float rayCosAngle = dot(viewDirection, pivot);

    for (int i = 0; i < steps; i++)
    {
        vec3 currentPosition = startPos + pivot * currentLength;
        vec4 projectedPosition = toFragSpace(vec4(currentPosition, 1.0f), camera.viewProjMatrix);
        vec2 currentUV = projectedPosition.xy;
        float projectedDepth = projectedPosition.z;

        float currentFragDepth = texture(depthTex, currentUV).r;
        float depthDiff = abs(1.0f / projectedDepth - 1.0f / currentFragDepth);
        if (depthDiff < bestDepth)
        {
            bestUV = currentUV;
            bestDepth = depthDiff;
            if(depthDiff < thickness)
                break;
        }
        else
        {
            vec3 newPosition = reconstructWorldPosition(currentFragDepth, currentUV, camera.invViewProjMatrix);
            currentLength = length(startPos - newPosition);
        }
    }

    vec3 environmentReflection = calcReflectionColor(skyboxMap, skyboxTransform, viewDirection, fragment.normal);
    vec3 ssrReflection = texture(HDRTex, bestUV).rgb;
    vec2 screenCenterDiff = 2.0f * abs(bestUV - vec2(0.5f));

    float fromReflectionPoint = length(bestUV - TexCoord) / maxDistance;
    float fromScreenCenter = max(screenCenterDiff.x, screenCenterDiff.y) * 10.0f - 9.0f;
    float fromRequiredThickness = (bestDepth - thickness) / (bestDepth + thickness);
    float fromCameraAngle = rayCosAngle / maxCosAngle;
    float maxFactor = max(max(fromReflectionPoint, fromScreenCenter), max(fromRequiredThickness, fromCameraAngle));
    float fadingFactor = 1.0f - clamp(maxFactor, 0.0f, 1.0f);

    environmentReflection = mix(environmentReflection, ssrReflection, fadingFactor);
    const vec3 luminance = vec3(0.2125f, 0.7154f, 0.0721f);
    float reflectionFactor = dot(luminance, mix(objectColor, environmentReflection, fragment.reflection));
    environmentReflection *= reflectionFactor;

    OutColor = vec4(mix(objectColor, environmentReflection, fragment.reflection), 1.0f);
}

)