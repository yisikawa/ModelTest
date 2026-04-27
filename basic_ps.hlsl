// 基本ピクセルシェーダー

Texture2D    g_texDiffuse : register(t0);
SamplerState g_samLinear  : register(s0);

struct PS_INPUT
{
    float4 position : SV_POSITION;
    float2 texcoord : TEXCOORD;
    float4 color    : COLOR;
};

float4 main(PS_INPUT input) : SV_TARGET
{
    float4 tex = g_texDiffuse.Sample(g_samLinear, input.texcoord);

    // テクスチャがない場合（サンプラーが白を返す）はライティングカラーをそのまま使用
    float3 finalColor = tex.rgb * input.color.rgb;
    return float4(finalColor, tex.a);
}
