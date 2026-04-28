// シャドウパス用頂点シェーダー（スキニング + ライト空間変換のみ）

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

cbuffer CBPerObject : register(b1)
{
    row_major float4x4 matWorld;
    row_major float4x4 boneMatrices[128];
};

struct VS_INPUT
{
    float3 position      : POSITION;
    float  blendWeight1  : BLENDWEIGHT;
    uint4  blendIndices1 : BLENDINDICES;
    float3 normal        : NORMAL;
    float2 texcoord      : TEXCOORD;
    float3 position2     : POSITION1;
    float  blendWeight2  : BLENDWEIGHT1;
    uint4  blendIndices2 : BLENDINDICES1;
    float3 normal2       : NORMAL1;
};

float4 main(VS_INPUT input) : SV_POSITION
{
    uint  bone1 = input.blendIndices1.r;
    uint  bone2 = input.blendIndices2.r;
    float w1    = input.blendWeight1;
    float w2    = input.blendWeight2;

    float4 localPos1 = float4(input.position,  1.0f);
    float4 localPos2 = float4(input.position2, 1.0f);
    float4 bonePos   = mul(localPos1, boneMatrices[bone1]) * w1
                     + mul(localPos2, boneMatrices[bone2]) * w2;

    float4 worldPos = mul(bonePos, matWorld);
    return mul(mul(worldPos, matLightView), matLightProj);
}
