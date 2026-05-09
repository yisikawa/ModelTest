# FBX Review 風アニメーションコントロール 調査・企画

## 目的

操作パネル（`IDD_DIALOG2`）の下部に、FBX Review のようなアニメーション操作を追加する。

- 再生 / 一時停止
- 先頭へ戻る
- 前フレーム / 次フレーム
- 末尾へ移動
- フレームまたは時間のスライダー
- 現在フレーム / 総フレーム表示

既存のモーション選択 UI は維持し、PC / NPC の選択状態に応じて対象モデルを操作する。

## 現状調査

### UI

- 操作パネルは Win32 ダイアログリソース `IDD_DIALOG2`。
- 定義位置は `resource.rc` の `IDD_DIALOG2 DIALOGEX 5, 100, 153, 314`。
- 現在の最下部は NPC グループと `IDC_COMBO31` で、y=277-300 付近まで使用している。
- ダイアログ高さは 314 dialog unit なので、現状の下にそのまま足す余白はほぼない。

### メッセージ処理

- `WinMain.cpp` の `Dlg2Proc` が操作パネルの `WM_COMMAND` を処理している。
- モーション選択は `IDC_COMBO8` が PC、`IDC_COMBO9` が NPC。
- PC / NPC 切り替えは `IDC_RADIO8` / `IDC_RADIO9` で `g_mPCFlag` を切り替える。
- モーション変更時は `SetMotionName()` と `LoadPCMotion()` / `LoadNPCMotion()` を呼ぶ。

### アニメーション更新

- 描画ループは `WinMain.cpp` のメッセージループ内で `Rendering()` を呼ぶ。
- `Render.cpp` の `Rendering()` が毎フレーム `timeGetTime()` 差分から `fTime` を算出している。
- 現状は毎描画で `pPC->AddTime(fTime*g_mMotionSpeed)` または `pNPC->AddTime(fTime*g_mMotionSpeed)` を呼ぶため、常に再生状態。
- 速度はグローバル `g_mMotionSpeed = 3000`。
- 実際の姿勢更新は `CModel::DynamicTransform()` が `m_Time` を使い、各ボーンの `CMotionElement::GetMotionMatrix(m_Time, ...)` を参照している。
- ループは `GetMotionMatrix()` 内で `fmod(time, 最終キー時刻)` により行われている。
- モーション長は `CModel::MaxMotionTime()` で取得できる。

## 基本方針

アニメーション制御は「時間を進めるかどうか」と「時間を指定値へ移動するか」を切り分ける。

既存の補間・ループ処理は `CMotionElement::GetMotionMatrix()` に任せる。UI側では既存の `GetTime()` / `SetTime()` / `AddTime()` を使って、`CModel::m_Time` に相当する再生時刻を管理する。

## 推奨 UI

`IDD_DIALOG2` を縦に拡張し、NPC セクションの下に `GROUPBOX "Playback"` を追加する。

最小構成:

| 操作 | 推奨コントロール | 備考 |
|---|---|---|
| 先頭 | PushButton `|<` | `m_Time = 0` |
| 前フレーム | PushButton `<` | 1 tick または 1 frame 戻す |
| 再生 / 一時停止 | PushButton `Play` / `Pause` | 押下で状態トグル |
| 次フレーム | PushButton `>` | 1 tick または 1 frame 進める |
| 末尾 | PushButton `>|` | `MaxMotionTime()` 近辺へ移動 |
| シーク | Trackbar | 0 から `MaxMotionTime()` |
| 表示 | Static text | `current / max` |

Win32 標準で済ませるなら `TRACKBAR_CLASS` を使うため、`InitCommonControlsEx()` と `commctrl.h` / `comctl32.lib` の確認が必要。

## 状態設計

追加するグローバル状態案:

```cpp
enum AnimPlaybackMode {
    ANIM_PLAYBACK_PLAYING,
    ANIM_PLAYBACK_PAUSED
};

bool  g_mAnimPlaying = true;
bool  g_mAnimLoop = true;
float g_mAnimStep = 100.0f; // 30fps相当なら g_mMotionSpeed / 30 など
bool  g_mAnimDraggingSeek = false;
```

`g_mAnimStep` はこのプロジェクトの時間単位が「秒」ではなく、`fTime * g_mMotionSpeed` で進む独自 tick なので、初期値は `g_mMotionSpeed / 30` が扱いやすい。

## 既存時刻 API と補助 API 案

`m_Time` は `CData` が持っており、既に以下の API がある。

```cpp
virtual void  AddTime(float addTime);
virtual float GetTime();
virtual void  SetTime(float Time);
```

そのため最初の実装では、新規 API なしで `SetTime()` / `GetTime()` / `AddTime()` を使える。
追加すると便利な補助関数は以下。

```cpp
static CModel* GetActiveModel();
static void    StepActiveModelTime(float delta);
static void    ClampActiveModelTime();
```

補助関数の方針:

- `SetTime()` は `MaxMotionTime()` を使って 0..max に clamp。
- 既存の `SetTime()` 自体は単純代入なので、clamp は UI ヘルパー側で行う。
- ループ再生時は描画側の加算後に max を超えたら 0 に戻す。ただし現状の `GetMotionMatrix()` は `fmod` するので、最低限は clamp なしでも描画破綻しにくい。
- コマ送りは `SetTime(GetTime() + delta)` で実現する。再生中に押された場合は一時停止してから進める運用が自然。

## 描画ループ変更案

`Render.cpp::Rendering()` の対象モデルごとの時間加算を以下の考え方に変える。

```cpp
CModel* model = g_mPCFlag ? (CModel*)pPC : (CModel*)pNPC;

if (g_mAnimPlaying) {
    model->AddTime(fTime * g_mMotionSpeed);
}
model->DynamicTransform();
```

現在は PC / NPC の分岐内に `AddTime()` があるため、まず「対象モデルを取る小ヘルパー」を作ると UI 側と共有しやすい。

## UI イベント処理案

`Dlg2Proc` の `WM_COMMAND` に新しいボタン ID を追加する。

```cpp
case IDC_BTN_ANIM_PLAY:
    g_mAnimPlaying = !g_mAnimPlaying;
    UpdateAnimControlState(in_hWnd);
    break;

case IDC_BTN_ANIM_PREV:
    g_mAnimPlaying = false;
    GetActiveModel()->StepTime(-g_mAnimStep);
    UpdateAnimControlState(in_hWnd);
    break;

case IDC_BTN_ANIM_NEXT:
    g_mAnimPlaying = false;
    GetActiveModel()->StepTime(g_mAnimStep);
    UpdateAnimControlState(in_hWnd);
    break;
```

Trackbar は `WM_HSCROLL` で処理する。

```cpp
case WM_HSCROLL:
    if ((HWND)lParam == GetDlgItem(in_hWnd, IDC_SLIDER_ANIM)) {
        int pos = (int)SendMessage((HWND)lParam, TBM_GETPOS, 0, 0);
        GetActiveModel()->SetTime((float)pos);
        UpdateAnimControlState(in_hWnd);
    }
    break;
```

## 更新タイミング

UI 表示は以下で更新する。

- モーション変更直後
- PC / NPC 切り替え直後
- 再生 / 停止 / コマ送り / シーク操作直後
- 再生中は 100ms 程度の `SetTimer()` で表示のみ更新

描画ループから毎フレーム `SetWindowText` するとダイアログの負荷とちらつきが増えるので、表示更新はタイマーで間引く。

## 実装ステップ

### Phase 1: 再生状態と対象モデルヘルパー

- `g_mAnimPlaying` を追加。
- `GetActiveModel()` を追加し、`g_mPCFlag ? pPC : pNPC` を集約する。
- `StepActiveModelTime()` で `GetTime()` / `SetTime()` を使う。
- `MaxMotionTime()` が 0 の場合は 0 固定。

### Phase 2: 再生 / 一時停止

- `Render.cpp::Rendering()` の `AddTime()` を `g_mAnimPlaying` 条件にする。
- ボタン1つで再生 / 一時停止を切り替え。

### Phase 3: コマ送り

- 前 / 次ボタンを追加。
- 押下時に一時停止し、`StepActiveModelTime(+-g_mAnimStep)` を呼ぶ。
- `g_mAnimStep` はまず `g_mMotionSpeed / 30`。

### Phase 4: スライダー

- Trackbar を追加。
- モーション変更時に range を `0..MaxMotionTime()` へ更新。
- `WM_HSCROLL` で `SetTime()`。

### Phase 5: 仕上げ

- Play ボタンの表示を状態に応じて `Play` / `Pause` に変更。
- 現在値表示を `123 / 456` の形式で更新。
- 先頭 / 末尾ボタンを追加。

## 注意点

- `CMotionElement::GetMotionMatrix()` は回転キー数を基準に `fmod()` しているため、ボーンごとにキー数や最終時刻が違う場合、完全な「末尾停止」は見た目がボーンごとに微妙にずれる可能性がある。
- `MaxMotionTime()` は `m_Bones[i].m_pMotion` を見ている。`m_MotionArray` と同期しているかは実装時に確認する。
- PC と NPC の時間は別々に持たれるはずなので、切り替えたときにそれぞれの現在時刻を表示する。
- ダイアログリソースは文字化けして見える箇所があるため、Visual Studio のリソースエディタで変更するか、既存エンコーディングを壊さないように慎重に編集する。

## 変更対象ファイル見込み

| ファイル | 内容 |
|---|---|
| `resource.h` | 新規 control ID 追加 |
| `resource.rc` | Playback グループ、ボタン、Trackbar、表示テキスト追加 |
| `WinMain.cpp` | UI イベント、タイマー、状態更新関数 |
| `Render.cpp` | 再生中のみ AddTime する制御 |
| `Model.h` / `Model.cpp` | 原則変更なし。clamp をモデル側へ寄せる場合のみ補助 API を検討 |

## 推奨する最初の実装範囲

最初は Trackbar なしで、`Pause` / `Prev` / `Next` / `Play` の4ボタンだけ実装する。

理由:

- 現状の時間単位とモーション長の扱いを確認しやすい。
- `resource.rc` の変更量が小さい。
- `CModel` の時刻 API と描画ループの制御が正しければ、Trackbar は後から安全に追加できる。
