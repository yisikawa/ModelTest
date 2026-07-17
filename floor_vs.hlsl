// シャドウキャッチャー床用 頂点シェーダー（固定ジオメトリ、ワールド変換なし）

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

struct VS_INPUT
{
    float3 position : POSITION;
};

struct VS_OUTPUT
{
    float4 position      : SV_POSITION;
    float4 posLightSpace : TEXCOORD0;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;

    // 床は固定ジオメトリのため頂点座標=ワールド座標（ワールド行列は使わない）
    float4 worldPos = float4(input.position, 1.0f);

    output.position      = mul(mul(worldPos, matView),      matProj);
    output.posLightSpace = mul(mul(worldPos, matLightView), matLightProj);

    return output;
}
