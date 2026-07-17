// 輪郭線ピクセルシェーダー(背面法)。アルファ抜きテクスチャに対応するためt0を参照
Texture2D    g_texDiffuse : register(t0);
SamplerState g_samLinear  : register(s0);

static const float3 outlineColor = float3(0.05f, 0.05f, 0.05f);

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
    float a = g_texDiffuse.Sample(g_samLinear, input.texcoord).a;
    clip(a - 0.1f);
    return float4(outlineColor, 1.0f);
}
