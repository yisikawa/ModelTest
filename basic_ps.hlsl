// 基本ピクセルシェーダー（Phong反射モデル + シャドウマップ）

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

static const float shininess   = 32.0f;
static const float shadowBias  = 0.001f;

struct PS_INPUT
{
    float4 position     : SV_POSITION;
    float2 texcoord     : TEXCOORD0;
    float3 worldPos     : TEXCOORD1;
    float3 worldNormal  : TEXCOORD2;
    float4 posLightSpace: TEXCOORD3;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    float3 N = normalize(input.worldNormal);
    float3 L = normalize(-lightDir.xyz);
    float3 V = normalize(eyePos.xyz - input.worldPos);
    float3 R = reflect(-L, N);

    float diffuse  = saturate(dot(N, L));
    float specular = pow(saturate(dot(R, V)), shininess);

    // シャドウ係数（ライト空間へ射影してシャドウマップと比較）
    float3 projCoord = input.posLightSpace.xyz / input.posLightSpace.w;
    float2 shadowUV  = projCoord.xy * float2(0.5f, -0.5f) + 0.5f;
    float  shadow    = 1.0f;
    if (saturate(shadowUV.x) == shadowUV.x && saturate(shadowUV.y) == shadowUV.y)
        shadow = g_texShadow.SampleCmpLevelZero(g_samShadow, shadowUV, projCoord.z - shadowBias);

    float3 lighting = lightAmbient.rgb
                    + shadow * diffuse  * lightDiffuse.rgb
                    + shadow * specular * lightSpecular.rgb;

    float4 tex = g_texDiffuse.Sample(g_samLinear, input.texcoord);
    if (all(tex == 0.0f))
        tex = float4(1.0f, 1.0f, 1.0f, 1.0f);
    clip(tex.a - 0.5f);
    float3 finalColor = tex.rgb * lighting;
    return float4(finalColor, tex.a);
}
