# saveFBX アニメーション出力 詳細調査レポート

以下に、`outputFBXAnimation` および関連する `outputFBXBone`, `GetAnimationMatrix` を精査した結果として、潜在的な問題点を**重要度順**に整理します。

---

## 問題1（重大）: アニメーション行列の二重適用

**該当箇所**: `outputFBXAnimation` 内 (line 397-398)

```cpp
iMatrix = m_Bones[tboneNo].m_mTransform;  // バインドポーズ行列
iMatrix *= *pAnimMat;                      // アニメーション行列を乗算
```

しかし `GetAnimationMatrix` の内部（Model.cpp line 2448）を見ると：

```cpp
D3DXMatrixTransformation(&oMatrix, &vCT, &pKeys1[keyNo].Quat,
    &pKeys3[keyNo].Scale, &vCT, &pKeys1[keyNo].Quat, &pKeys2[keyNo].Pos);
```

ここで `vCT` は `iMatrix`（= `m_mTransform`）で変換済みの座標中心です。つまり、**`GetAnimationMatrix` は既に `m_mTransform` を考慮した行列を返している**のに、さらに `m_mTransform` を掛けている可能性があります。

> [!WARNING]
> もし `GetAnimationMatrix` が「バインドポーズからの差分変換」を返す設計であればこの乗算は正しいですが、「ワールド最終行列」を返す設計であれば**二重適用**となり、ボーンが異常に動く原因になります。
> `GetAnimationMatrix` の `iMatrix` 引数の用途を確認してください。

---

## 問題2（重大）: `m_RotationKeyNum` と `m_pTranslateKeys` のキー数不整合リスク

**該当箇所**: `outputFBXAnimation` 内 (line 370, 391)

```cpp
int keyNum = m_MotionArray[tboneNo].m_RotationKeyNum;  // 回転キー数でループ
// ...
fbxTime.SetSecondDouble(m_MotionArray[tboneNo].m_pTranslateKeys[k].Time / 3000.);
// ↑ 移動キー配列にアクセス
```

ループ回数は `m_RotationKeyNum` で決定されていますが、時間の取得には `m_pTranslateKeys[k].Time` を使用しています。`m_RotationKeyNum` と `m_TranslateKeyNum` が異なる場合、**配列の範囲外アクセス**が発生してクラッシュするか、不正な時間値を取得します。

> [!CAUTION]
> `m_RotationKeyNum > m_TranslateKeyNum` の場合、未定義動作（クラッシュ or メモリ破壊）を引き起こします。ループの上限を `min(m_RotationKeyNum, m_TranslateKeyNum)` にするか、各キー配列のサイズを別々にチェックする必要があります。

---

## 問題3（中程度）: `keyNum <= 0` 時のフォールバックロジック

**該当箇所**: `outputFBXAnimation` 内 (line 373-380)

```cpp
if (keyNum <= 0) {
    tboneNo = 0;  // ボーン0のキーを代用
    tkeyNum = m_MotionArray[tboneNo].m_RotationKeyNum;
    // ...
}
```

キーフレームが無いボーンに対して、**ボーン0のキーデータで代用**しています。これにより：
- ボーン0と同じ回転・移動アニメーションが全ての「キー無し」ボーンに適用される
- 本来は静止しているべきボーンが不自然に動く

通常、キーフレームが0のボーンはアニメーションカーブ自体を作成しない（`continue`する）のが正しい動作です。

---

## 問題4（軽微）: `GetAnimationMatrix` の static 変数

**該当箇所**: `Model.cpp` line 2435

```cpp
static D3DXMATRIX oMatrix;  // static!
```

`GetAnimationMatrix` は `static` なローカル変数のポインタを返しています。呼び出し元で即座に値をコピー（`*pAnimMat` で使用）しているため現状は動作しますが、マルチスレッド環境や、戻り値を保持したまま再度呼び出すと**データが上書き**されます。

---

## 問題5（情報）: `outputFBXBone` でのルートボーンの回転値

**該当箇所**: `outputFBXBone` 内 (line 255)

```cpp
pRBoneNode->LclRotation.Set(FbxDouble3(-180., -90., 0.));
```

ルートボーン（`Scene_Root`）に `-180, -90, 0` のハードコードされた回転を設定しています。座標系変換（DirectX→FBX）のための補正値と思われますが、この値が正しいかどうかはDirectX側の座標系（左手系Y-up）とFBX側の設定（通常は右手系Y-up）の関係に依存します。

---

## まとめ

| # | 問題 | 重大度 | 修正難易度 |
|---|------|--------|------------|
| 1 | アニメーション行列の二重適用 | 🔴 重大 | 要調査 |
| 2 | RotationKeyNum / TranslateKeys 不整合 | 🔴 重大 | 低 |
| 3 | keyNum<=0 フォールバック | 🟡 中 | 低 |
| 4 | static 変数の戻り値 | 🟢 軽微 | 低 |
| 5 | ルートボーン回転値 | ℹ️ 情報 | 要確認 |
