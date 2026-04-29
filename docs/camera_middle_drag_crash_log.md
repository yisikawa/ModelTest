# 起動直後の中ボタンドラッグで落ちる問題 調査ログ

## 概要

起動直後にマウス中ボタンでドラッグすると、Debug ビルドで DirectXMath のアサートによりアプリが停止していた。

表示されたアサート:

```text
File: ...\DirectXMath\Inc\DirectXCollision.inl
Expression: !XMVector3Equal(EyeDirection, XMVectorZero())
```

## 再現条件

- アプリ起動直後
- 左ドラッグやホイール操作を行う前
- 中ボタンを押してドラッグする

左ドラッグまたはホイール操作を一度行った後は、同じ中ボタンドラッグでも落ちにくい。

## 原因

中ボタンドラッグ時の処理では、`g_mEyeMat` を使って `g_mEye` を再計算している。

該当箇所:

```cpp
D3DXVec3TransformNormal(&g_mEye,&g_mEyebase,&g_mEyeMat);
g_mEye += g_mAt;
D3DXMatrixLookAtLH( &g_mView, &g_mEye, &g_mAt, &g_mUp );
```

しかし起動直後は、`g_mEyeMat` がまだ初期化されていなかった。

左ドラッグやホイール操作では先に `g_mEyeMat = matS * matX * matY;` が実行されるため問題が表面化しないが、中ボタンドラッグだけは未初期化の `g_mEyeMat` を直接使用していた。

その結果、`g_mEye` が `g_mAt` と同じ座標になり、`D3DXMatrixLookAtLH` 内部の `XMMatrixLookAtLH` で視線方向 `EyeDirection` がゼロと判定されてアサートした。

## 修正

`Create3DSpace()` のデフォルトカメラ設定時に、`g_mEyeMat` と `g_mEye` を初期化するようにした。

```cpp
D3DXMatrixIdentity( &g_mEyeMat );
D3DXVec3TransformNormal( &g_mEye, &g_mEyebase, &g_mEyeMat );
g_mEye += g_mAt;
D3DXMatrixLookAtLH( &g_mView, &g_mEye, &g_mAt, &g_mUp );
```

`g_mEyeScale = 1.0f`, `g_mEyeAlph = 0.0f`, `g_mEyeBeta = 0.0f` の初期状態では、Identity 行列が既存の初期カメラ状態と一致する。

## 確認結果

- `Render.cpp` のコンパイルは成功
- リンク確認時は、実行中の `Debug\ModelTest.exe` が掴まれていたため `LNK1168` が発生
- その後、実機操作で起動直後の中ボタンドラッグを確認し、アプリが落ちずに実行継続することを確認

## 関連コミット

```text
ced98a7 カメラの視点行列を初期化
```
