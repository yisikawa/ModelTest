// 基本ピクセルシェーダー（ソフトライティング + 3x3 PCFシャドウ + リムライト）

Texture2D    g_texDiffuse : register(t0);
SamplerState g_samLinear  : register(s0);

Texture2D<float>       g_texShadow : register(t1);
SamplerComparisonState g_samShadow : register(s1);

cbuffer CBPerFrame : register(b0)
{
    row_major float4x4 matView;
    row_major float4x4 matProj;
    float4             lightDir;
    float4             lightDiffuse;
    float4             lightAmbient;
    float4             lightSpecular;
    float4             eyePos;
    row_major float4x4 matLightView;
    row_major float4x4 matLightProj;
};

static const float shadowMapSize    = 2048.0f;
static const float minShadowBias    = 0.0005f;
static const float slopeShadowBias  = 0.0025f;
static const float specularPower    = 48.0f;
static const float specularStrength = 0.35f;
static const float rimPower         = 2.5f;
static const float rimStrength      = 0.18f;
static const bool  enableGamma      = false;

struct PS_INPUT
{
    float4 position     : SV_POSITION;
    float2 texcoord     : TEXCOORD0;
    float3 worldPos     : TEXCOORD1;
    float3 worldNormal  : TEXCOORD2;
    float4 posLightSpace: TEXCOORD3;
};

float ComputeHalfLambert(float3 normal, float3 lightDirToSurface)
{
    float diffuse = saturate(dot(normal, lightDirToSurface) * 0.5f + 0.5f);
    return diffuse * diffuse;
}

float ComputeBlinnSpecular(float3 normal, float3 lightDirToSurface, float3 viewDir)
{
    float3 halfDir = normalize(lightDirToSurface + viewDir);
    return pow(saturate(dot(normal, halfDir)), specularPower) * specularStrength;
}

float ComputeRim(float3 normal, float3 viewDir)
{
    return pow(1.0f - saturate(dot(normal, viewDir)), rimPower) * rimStrength;
}

float ComputeShadowBias(float3 normal, float3 lightDirToSurface)
{
    float ndotl = saturate(dot(normal, lightDirToSurface));
    return max(minShadowBias, slopeShadowBias * (1.0f - ndotl));
}

float SampleShadow3x3(float2 uv, float depth)
{
    float2 texelSize = float2(1.0f / shadowMapSize, 1.0f / shadowMapSize);
    float shadow = 0.0f;

    [unroll]
    for (int y = -1; y <= 1; ++y)
    {
        [unroll]
        for (int x = -1; x <= 1; ++x)
        {
            shadow += g_texShadow.SampleCmpLevelZero(
                g_samShadow,
                uv + float2((float)x, (float)y) * texelSize,
                depth);
        }
    }

    return shadow / 9.0f;
}

float ComputeShadow(float4 posLightSpace, float3 normal, float3 lightDirToSurface)
{
    float3 projCoord = posLightSpace.xyz / posLightSpace.w;
    float2 shadowUV  = projCoord.xy * float2(0.5f, -0.5f) + 0.5f;

    if (saturate(shadowUV.x) != shadowUV.x || saturate(shadowUV.y) != shadowUV.y)
        return 1.0f;

    float bias = ComputeShadowBias(normal, lightDirToSurface);
    return SampleShadow3x3(shadowUV, projCoord.z - bias);
}

float3 ApplyOutputGamma(float3 color)
{
    if (!enableGamma)
        return color;

    return pow(saturate(color), 1.0f / 2.2f);
}

float4 main(PS_INPUT input) : SV_TARGET
{
    float3 N = normalize(input.worldNormal);
    float3 L = normalize(-lightDir.xyz);
    float3 V = normalize(eyePos.xyz - input.worldPos);

    float4 tex = g_texDiffuse.Sample(g_samLinear, input.texcoord);
    if (all(tex == 0.0f))
        tex = float4(1.0f, 1.0f, 1.0f, 1.0f);
    clip(tex.a - 0.5f);

    float diffuse  = ComputeHalfLambert(N, L);
    float specular = ComputeBlinnSpecular(N, L, V);
    float shadow   = ComputeShadow(input.posLightSpace, N, L);
    float rim      = ComputeRim(N, V);

    float3 lighting = lightAmbient.rgb
                    + shadow * diffuse  * lightDiffuse.rgb
                    + shadow * specular * lightSpecular.rgb
                    + rim * lightDiffuse.rgb;

    float3 finalColor = tex.rgb * lighting;
    finalColor = ApplyOutputGamma(finalColor);
    return float4(finalColor, tex.a);
}
