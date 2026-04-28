// スキニング頂点シェーダー（2ボーンブレンド）
// Slot 0 (VB1): bone1 ローカル空間頂点・重み・インデックス
// Slot 1 (VB2): bone2 ローカル空間頂点・重み・インデックス

cbuffer CBPerFrame : register(b0)
{
    row_major float4x4 matView;
    row_major float4x4 matProj;
    float4             lightDir;
    float4             lightDiffuse;
    float4             lightAmbient;
    float4             lightSpecular;
    float4             eyePos;
};

cbuffer CBPerObject : register(b1)
{
    row_major float4x4 matWorld;
    row_major float4x4 boneMatrices[128];
};

struct VS_INPUT
{
    // Slot 0 (VB1)
    float3 position      : POSITION;
    float  blendWeight1  : BLENDWEIGHT;
    uint4  blendIndices1 : BLENDINDICES;
    float3 normal        : NORMAL;
    float2 texcoord      : TEXCOORD;
    // Slot 1 (VB2)
    float3 position2     : POSITION1;
    float  blendWeight2  : BLENDWEIGHT1;
    uint4  blendIndices2 : BLENDINDICES1;
    float3 normal2       : NORMAL1;
};

struct VS_OUTPUT
{
    float4 position    : SV_POSITION;
    float2 texcoord    : TEXCOORD0;
    float3 worldPos    : TEXCOORD1;
    float3 worldNormal : TEXCOORD2;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;

    uint  bone1 = input.blendIndices1.r;
    uint  bone2 = input.blendIndices2.r;
    float w1    = input.blendWeight1;
    float w2    = input.blendWeight2;

    // 2ボーンブレンド
    float4 localPos1 = float4(input.position,  1.0f);
    float4 localPos2 = float4(input.position2, 1.0f);
    float4 bonePos   = mul(localPos1, boneMatrices[bone1]) * w1
                     + mul(localPos2, boneMatrices[bone2]) * w2;

    float4 worldPos  = mul(bonePos,  matWorld);
    float4 viewPos   = mul(worldPos, matView);
    output.position  = mul(viewPos,  matProj);
    output.worldPos  = worldPos.xyz;

    // 法線ブレンド
    float3 boneNormal  = mul(input.normal,  (float3x3)boneMatrices[bone1]) * w1
                       + mul(input.normal2, (float3x3)boneMatrices[bone2]) * w2;
    output.worldNormal = normalize(mul(boneNormal, (float3x3)matWorld));

    output.texcoord = input.texcoord;

    return output;
}
