# シェーダー即効改善 設計メモ

## 目的

DX11レンダラーの構造は維持したまま、最小限のC++変更、できればHLSLだけでキャラクターの見やすさと影品質を改善する。

対象ファイル:

- `basic_ps.hlsl`
- `skinning_vs.hlsl`
- `shadow_vs.hlsl`
- 調整値をC++側に出す場合のみ `Dx.h` / `Dx.cpp`

## 現状

現在のメインピクセルシェーダーは以下の構成。

- Phongの拡散反射/鏡面反射
- `SampleCmpLevelZero` 1回のシャドウ参照
- 固定 `shininess`
- 固定 `shadowBias`
- `0.5` でアルファクリップ
- ガンマ補正なし

シンプルで安定しているため、最初の改善ではテクスチャスロット、サンプラ、定数バッファ、頂点シェーダー出力を維持する。

## Phase 1: HLSLのみで改善

C++側の定数バッファレイアウトは変更しない。

### 1. 3x3 PCFシャドウ

1回のシャドウ比較を小さなヘルパー関数に置き換える。

```hlsl
float SampleShadow3x3(float2 uv, float depth)
{
    float2 texelSize = float2(1.0f / 2048.0f, 1.0f / 2048.0f);
    float sum = 0.0f;

    [unroll]
    for (int y = -1; y <= 1; ++y)
    {
        [unroll]
        for (int x = -1; x <= 1; ++x)
        {
            sum += g_texShadow.SampleCmpLevelZero(
                g_samShadow,
                uv + float2(x, y) * texelSize,
                depth);
        }
    }

    return sum / 9.0f;
}
```

期待効果:

- キャラクター影が少し柔らかくなる
- シャドウの階段状ジャギーが減る
- ピクセルシェーダー負荷は少し増えるが、このビューア用途では許容範囲

### 2. 傾き対応シャドウバイアス

固定バイアスを、法線とライト方向に応じた値に置き換える。

```hlsl
float ComputeShadowBias(float3 normal, float3 lightDirToSurface)
{
    float ndotl = saturate(dot(normal, lightDirToSurface));
    return max(0.0005f, 0.0025f * (1.0f - ndotl));
}
```

期待効果:

- 斜め面のシャドウアクネを減らす
- グローバル固定バイアスの手調整を減らす

### 3. Blinn-Phong鏡面反射

反射ベクトル式の鏡面反射を、ハーフベクトル式に置き換える。

```hlsl
float3 H = normalize(L + V);
float specular = pow(saturate(dot(N, H)), 48.0f) * 0.35f;
```

期待効果:

- ハイライトが安定する
- 古典的Phongより動きが少し自然になる

### 4. Half-Lambert拡散反射

暗部が潰れにくい拡散反射にする。

```hlsl
float ndotl = dot(N, L);
float diffuse = saturate(ndotl * 0.5f + 0.5f);
diffuse *= diffuse;
```

期待効果:

- 影側の形が見えやすくなる
- 低解像度寄りの古いキャラクターテクスチャと相性がよい

### 5. リムライト

視線角に応じた控えめな輪郭光を追加する。

```hlsl
float rim = pow(1.0f - saturate(dot(N, V)), 2.5f);
float3 rimColor = lightDiffuse.rgb * rim * 0.18f;
```

期待効果:

- キャラクターのシルエットが背景から分離しやすくなる
- モデルビューアとして見やすくなる

### 6. 簡易ガンマ補正

最終出力に保守的なガンマ変換を入れられるようにする。

```hlsl
finalColor = pow(saturate(finalColor), 1.0f / 2.2f);
```

期待効果:

- 中間調が見やすくなる
- リニアそのままの硬い見え方を抑えられる

リスク:

- テクスチャが既に表示ガンマ前提で作られている場合、明るくなりすぎる可能性がある。初期状態では無効にして、簡単に切り替えられるようにする。

## Phase 1のピクセルシェーダー処理順

```hlsl
float3 N = normalize(input.worldNormal);
float3 L = normalize(-lightDir.xyz);
float3 V = normalize(eyePos.xyz - input.worldPos);

float4 tex = g_texDiffuse.Sample(g_samLinear, input.texcoord);
if (all(tex == 0.0f))
    tex = float4(1.0f, 1.0f, 1.0f, 1.0f);
clip(tex.a - 0.5f);

float diffuse = ComputeHalfLambert(N, L);
float specular = ComputeBlinnSpecular(N, L, V);
float shadow = ComputeShadow(input.posLightSpace, N, L);
float rim = ComputeRim(N, V);

float3 lighting = lightAmbient.rgb
                + shadow * diffuse * lightDiffuse.rgb
                + shadow * specular * lightSpecular.rgb
                + rim * lightDiffuse.rgb;

float3 finalColor = tex.rgb * lighting;
finalColor = ApplyOutputGamma(finalColor);
return float4(finalColor, tex.a);
```

## Phase 2: C++側に調整値を出す

Phase 1の見た目を確認した後、固定値を定数バッファへ移す。

追加候補:

```cpp
XMFLOAT4 shaderParams0;
// x: specularPower
// y: specularStrength
// z: rimStrength
// w: gammaEnabled

XMFLOAT4 shadowParams0;
// x: shadowMapSize
// y: minBias
// z: slopeBias
// w: pcfRadius
```

必要な変更:

- `Dx.h` の `CBPerFrame`
- `Model.cpp` の定数バッファ設定
- `basic_ps.hlsl`
- 必要なら後でUI調整項目

## 推奨する最初のパッチ

まず `basic_ps.hlsl` だけ変更する。

- ヘルパー関数を追加
- 3x3 PCFを使用
- 傾き対応バイアスを使用
- Blinn-Phongへ変更
- 控えめなリムライトを追加
- ガンマ補正はローカル定数で無効/有効を切り替え

この形が最も早く見た目を確認でき、調整や差し戻しもしやすい。

## 確認項目

- テクスチャあり/なしでキャラクターが見える
- 透明抜き部分が今まで通りクリップされる
- 浅い角度でも影が消えない
- 影のエッジが以前より柔らかい
- 鏡面反射がプラスチックっぽく強すぎない
- リムライトが輪郭を助けるが、光りすぎない
- Debug/Releaseでシェーダーコンパイルエラーが出ない
