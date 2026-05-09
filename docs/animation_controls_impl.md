# アニメーションコントロール 実装計画

GPT5.5 作成の `animation_controls_plan.md` をベースに、コード調査結果を加えた具体的変更仕様。

## コード調査結果（確定事項）

| 項目 | 詳細 |
|---|---|
| `commctrl.h` | `WinMain.h:8` で既にインクルード済み → Trackbar 即利用可 |
| `pPC`/`pNPC`/`g_mPCFlag` | `WinMain.cpp:42-53` で extern 済み → Dlg2Proc から直接アクセス可 |
| `AddTime` 呼び出し箇所 | `Render.cpp:99`（PC）と `Render.cpp:115`（NPC）の2箇所 |
| `AddTime`/`GetTime`/`SetTime` | `Model.h:453-455` で virtual 定義済み |
| `MaxMotionTime()` | `Model.h:556` で宣言済み |
| 既存 resource ID 最大値 | `IDC_RADIO4 = 1051` → 新規 ID は 1052 以降 |
| `g_mAnimPlaying` 定義場所 | `Render.cpp`（`g_mPCFlag` と同じブロック line 34付近） |

---

## Phase 1: グローバル状態 + ヘルパー追加

### Render.cpp（line 34 付近）

```cpp
// 既存グローバルの直後に追加
bool  g_mAnimPlaying = true;
float g_mAnimStep    = 100.0f; // g_mMotionSpeed(3000) / 30fps ≈ 100
```

### Render.h（extern 宣言追加）

```cpp
extern bool  g_mAnimPlaying;
extern float g_mAnimStep;
CModel* GetActiveModel();
```

### Render.cpp（Rendering() の直前に追加）

```cpp
CModel* GetActiveModel() {
    return g_mPCFlag ? (CModel*)pPC : (CModel*)pNPC;
}
```

---

## Phase 2: Render.cpp 再生制御ガード

**対象行:** `Render.cpp:99` と `Render.cpp:115`

```cpp
// 変更前
if (g_mPCFlag) {
    pPC->AddTime(fTime*g_mMotionSpeed);
    ...
} else {
    pNPC->AddTime(fTime*g_mMotionSpeed);
    ...
}

// 変更後
if (g_mPCFlag) {
    if (g_mAnimPlaying) pPC->AddTime(fTime*g_mMotionSpeed);
    ...
} else {
    if (g_mAnimPlaying) pNPC->AddTime(fTime*g_mMotionSpeed);
    ...
}
```

---

## Phase 3: resource.h 新規 ID 追加

`resource.h` 末尾（IDC_RADIO4 = 1051 の後）に追加：

```c
#define IDC_BTN_ANIM_FIRST      1052
#define IDC_BTN_ANIM_PREV       1053
#define IDC_BTN_ANIM_PLAY       1054
#define IDC_BTN_ANIM_NEXT       1055
#define IDC_BTN_ANIM_LAST       1056
#define IDC_SLIDER_ANIM         1057
#define IDC_STATIC_ANIM_TIME    1058
#define IDC_TIMER_ANIM          1
```

---

## Phase 4: resource.rc ダイアログ拡張

### ダイアログ高さ変更

```rc
// 変更前
IDD_DIALOG2 DIALOGEX 5, 100, 153, 314
// 変更後
IDD_DIALOG2 DIALOGEX 5, 100, 153, 380
```

### NPC セクション下（y=305付近）に Playback グループ追加

```rc
GROUPBOX        "Playback", IDC_STATIC,      4,   305, 145, 70
PUSHBUTTON      "|<", IDC_BTN_ANIM_FIRST,    6,   320,  20, 14
PUSHBUTTON      "<",  IDC_BTN_ANIM_PREV,    29,   320,  20, 14
PUSHBUTTON      "Play", IDC_BTN_ANIM_PLAY,  52,   320,  30, 14
PUSHBUTTON      ">",  IDC_BTN_ANIM_NEXT,    85,   320,  20, 14
PUSHBUTTON      ">|", IDC_BTN_ANIM_LAST,   108,   320,  20, 14
CONTROL         "", IDC_SLIDER_ANIM, TRACKBAR_CLASS,
                TBS_HORZ|TBS_NOTICKS|WS_CHILD|WS_VISIBLE,
                6, 339, 141, 14
LTEXT           "0 / 0", IDC_STATIC_ANIM_TIME, 6, 358, 141, 10
```

> **注意:** resource.rc のエンコーディング（Shift-JIS）を壊さないよう、テキストエディタの文字コード設定に注意。

---

## Phase 5: WinMain.cpp UI ハンドラ追加

### UpdateAnimControlState() 関数（Dlg2Proc の直前に追加）

```cpp
static void UpdateAnimControlState(HWND hWnd) {
    CModel* model = GetActiveModel();
    if (!model) return;

    float maxT = model->MaxMotionTime();
    float curT = model->GetTime();

    SetWindowText(GetDlgItem(hWnd, IDC_BTN_ANIM_PLAY),
                  g_mAnimPlaying ? "Pause" : "Play");

    SendDlgItemMessage(hWnd, IDC_SLIDER_ANIM, TBM_SETRANGE, TRUE,
                       MAKELPARAM(0, (int)maxT));
    SendDlgItemMessage(hWnd, IDC_SLIDER_ANIM, TBM_SETPOS, TRUE, (int)curT);

    char buf[64];
    wsprintfA(buf, "%d / %d", (int)curT, (int)maxT);
    SetWindowText(GetDlgItem(hWnd, IDC_STATIC_ANIM_TIME), buf);
}
```

### WM_INITDIALOG（既存処理の末尾に追加）

```cpp
SetTimer(in_hWnd, IDC_TIMER_ANIM, 100, NULL);
```

### WM_TIMER ハンドラ（新規追加）

```cpp
case WM_TIMER:
    if (wParam == IDC_TIMER_ANIM)
        UpdateAnimControlState(in_hWnd);
    return 0;
```

### WM_COMMAND に新規ボタンハンドラ追加

```cpp
case IDC_BTN_ANIM_PLAY:
    g_mAnimPlaying = !g_mAnimPlaying;
    UpdateAnimControlState(in_hWnd);
    break;

case IDC_BTN_ANIM_PREV:
    g_mAnimPlaying = false;
    {
        float t = GetActiveModel()->GetTime() - g_mAnimStep;
        GetActiveModel()->SetTime(t < 0.0f ? 0.0f : t);
    }
    UpdateAnimControlState(in_hWnd);
    break;

case IDC_BTN_ANIM_NEXT:
    g_mAnimPlaying = false;
    {
        float maxT = GetActiveModel()->MaxMotionTime();
        float t    = GetActiveModel()->GetTime() + g_mAnimStep;
        GetActiveModel()->SetTime(t > maxT ? maxT : t);
    }
    UpdateAnimControlState(in_hWnd);
    break;

case IDC_BTN_ANIM_FIRST:
    g_mAnimPlaying = false;
    GetActiveModel()->SetTime(0.0f);
    UpdateAnimControlState(in_hWnd);
    break;

case IDC_BTN_ANIM_LAST:
    g_mAnimPlaying = false;
    GetActiveModel()->SetTime(GetActiveModel()->MaxMotionTime());
    UpdateAnimControlState(in_hWnd);
    break;
```

### WM_HSCROLL ハンドラ（新規追加）

```cpp
case WM_HSCROLL:
    if ((HWND)lParam == GetDlgItem(in_hWnd, IDC_SLIDER_ANIM)) {
        int pos = (int)SendMessage((HWND)lParam, TBM_GETPOS, 0, 0);
        GetActiveModel()->SetTime((float)pos);
        UpdateAnimControlState(in_hWnd);
    }
    return 0;
```

### モーション変更・PC/NPC 切り替え時にも呼ぶ

既存の `IDC_COMBO8`/`IDC_COMBO9`（モーション選択）と
`IDC_RADIO8`/`IDC_RADIO9`（PC/NPC 切り替え）ハンドラの末尾に追加：

```cpp
UpdateAnimControlState(in_hWnd);
```

---

## 変更ファイル一覧

| ファイル | 変更内容 |
|---|---|
| `Render.h` | `g_mAnimPlaying`/`g_mAnimStep` の extern、`GetActiveModel()` プロトタイプ |
| `Render.cpp` | グローバル追加、`GetActiveModel()` 実装、`AddTime` ガード |
| `resource.h` | 新規 ID 7個 + タイマー ID 追加 |
| `resource.rc` | ダイアログ高さ拡張 + Playback グループ追加 |
| `WinMain.cpp` | `UpdateAnimControlState()`、各ハンドラ、タイマー |

## 実装順序

1. `Render.cpp`/`Render.h` → Phase1
2. `Render.cpp` → Phase2（ここでビルド確認推奨）
3. `resource.h` → Phase3
4. `resource.rc` → Phase4
5. `WinMain.cpp` → Phase5
