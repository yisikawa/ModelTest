// シャドウキャッチャー床用 ピクセルシェーダー
// 影の落ちる部分だけ半透明の黒を出力し、影範囲外は完全に透明にする

Texture2D<float>       g_texShadow : register(t1);
SamplerComparisonState g_samShadow : register(s1);

static const float shadowMapSize = 2048.0f;
static const float shadowBias    = 0.001f; // 床は平面でセルフシャドウしないため固定値でよい
static const float shadowAlpha   = 0.45f;  // 影の最大不透明度

struct PS_INPUT
{
    float4 position      : SV_POSITION;
    float4 posLightSpace : TEXCOORD0;
};

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

float ComputeShadow(float4 posLightSpace)
{
    float3 projCoord = posLightSpace.xyz / posLightSpace.w;
    float2 shadowUV  = projCoord.xy * float2(0.5f, -0.5f) + 0.5f;

    // シャドウマップの範囲外は影なし（透明）扱い
    if (saturate(shadowUV.x) != shadowUV.x || saturate(shadowUV.y) != shadowUV.y)
        return 1.0f;

    // ライト空間の深度範囲外（far面より奥）は影なし扱い
    if (projCoord.z > 1.0f)
        return 1.0f;

    return SampleShadow3x3(shadowUV, projCoord.z - shadowBias);
}

float4 main(PS_INPUT input) : SV_TARGET
{
    float shadow = ComputeShadow(input.posLightSpace);

    // shadow=1(影なし) → alpha=0(透明) / shadow=0(完全に影) → alpha=shadowAlpha
    return float4(0.0f, 0.0f, 0.0f, (1.0f - shadow) * shadowAlpha);
}
