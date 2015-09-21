//======================================================================
// INCLUDE
//======================================================================
#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include "WinMain.h"
#include "Dx.h"
#include "Render.h"
#include "model.h"
#include "resource.h"
#include <commdlg.h>

#pragma comment(lib,"comdlg32.lib")


//======================================================================
// PROTOTYPE
//======================================================================
LRESULT CALLBACK WinProc( HWND hWnd, UINT msg, UINT wParam, LONG lParam );
LRESULT CALLBACK Dlg2Proc( HWND,UINT,WPARAM,LPARAM);
DWORD	ConvertStr2Dno( char* DataName );
DWORD	ConvertStr2Dno2( char* DataName );
BOOL GetFileNameFromDno(LPSTR filename,DWORD dwID);
BOOL GetFileNameFromDir(LPSTR filename,char *DataName );
BOOL GetFileNameFromDir2(LPSTR filename,char *DataName );
//======================================================================
// GLOBAL
//======================================================================
#define PAI					(3.1415926535897932384626433832795f)
#define PAI2				(PAI*2.0f)
char	ffxidir[512];
bool	g_mDispWire = true,g_mDispIdl = true,g_mDispBone = false;
//								hum m   hum f   el m    el f    tar m   tar f   M       G
int		g_mShlBoneTbl[8][2]={ {64,78},{60,45},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0},{ 0, 0}};
int		g_mDispBoneNo=1,g_mShlBoneNoR=g_mShlBoneTbl[0][0],g_mShlBoneNoL=g_mShlBoneTbl[0][1];
extern	CPC		*pPC;
extern	CNPC	*pNPC;
extern	D3DLIGHT9		g_mLight,g_mLightbase;
extern	D3DXMATRIX		g_mProjection, g_mView,g_mEyeMat;
extern	float			g_mEyeScale,g_mEyeAlph,g_mEyeBeta;
extern	float			g_mLightAlph,g_mLightBeta;
extern	D3DXVECTOR3		g_mEye,g_mEyebase,g_mAt,g_mUp;
extern	float			g_mFov;				// FOV : 60度
extern	float			g_mAspect;			// 画面のアスペクト比
extern	float			g_mNear_z;			// 最近接距離
extern	float			g_mFar_z;			// 最遠方距離
extern  bool			g_mPCFlag;
extern  int				g_mPCMotion;		// PCのモーション種類
		long			g_mScreenWidth	= 800;
		long			g_mScreenHeight	= 600;
static	char			*AppName = "EwhM ver0.1";
static	char			*ClassName = "Model Test";
static	DWORD			FPS;

HWND hWindow;				// ウィンドウハンドル
HWND hDlg2;					// ダイアログ２

extern	int				g_mMotionSpeed;
		unsigned long	Polygons;

static const	LPCTSTR		ListRace[] = {
	"1-27-82,ヒューム♂","1-32-58,ヒューム♀",
	"1-37-31,エルヴァーン♂","1-42-4,エルヴァーン♀",
	"1-46-93,タルタル♂","1-46-93,タルタル♀",
	"1-51-89,ミスラ","1-56-59,ガルカ","1-0-0,なし" };


char	execDir[512];

//======================================================================
//
//		各種関数
//
//======================================================================
long GetScreenWidth( void ) { return g_mScreenWidth; }
long GetScreenHeight( void ) { return g_mScreenHeight; }
HWND GetWindow( void ) { return hWindow; }
void AdDrawPolygons( unsigned long polys ) { Polygons += polys; }

static const	LPCTSTR		OriginRace[] = {
	"ヒューム♂","ヒューム♀",
	"エルヴァーン♂","エルヴァーン♀",
	"タルタル♂","タルタル♀",
	"ミスラ","ガルカ","なし" };
static const	LPCTSTR		OriginParts[] = {
	"種族","顔","頭","胴","両手","両脚","両足",
	"右手武器","左手武器","遠隔武器" };

DWORD	ConvertStr2Dno2( char* DataName )
{
	int		Dno,Type,Hi,Lo;
	Type = 1;
	sscanf(DataName,"%d-%d-%d,",&Type,&Hi,&Lo);
	Dno = Type*0x10000 + Hi*0x80 + Lo%0x80;
	return Dno ;
}

DWORD	ConvertStr2Dno( char* DataName )
{
	int		Dno,Type,Hi,Lo;
	Type = 1;
	sscanf(DataName,"%d-%d,",&Hi,&Lo);
	Dno = Type*0x10000 + Hi*0x80 + Lo%0x80;
	return Dno ;
}

BOOL GetFileNameFromDno(LPSTR filename,DWORD Dno)
{
	DWORD	dum1,dum2,dum3,no,ComboNo,ComboNoBkup;
	char	ComboString[128],PartsName[64];
 
	no = LOWORD(Dno);

	if( HIWORD(Dno)==1 ) wsprintf(filename,"%sROM\\%d\\%d.dat",ffxidir,no/0x80,no%0x80);
	else if( HIWORD(Dno)==2 ) wsprintf(filename,"%sROM2\\%d\\%d.dat",ffxidir,no/0x80,no%0x80);
	else if( HIWORD(Dno)==3 ) wsprintf(filename,"%sROM3\\%d\\%d.dat",ffxidir,no/0x80,no%0x80);
	else if( HIWORD(Dno)==4 ) wsprintf(filename,"%sROM4\\%d\\%d.dat",ffxidir,no/0x80,no%0x80);
	else if( HIWORD(Dno)==5 ) wsprintf(filename,"%sROM5\\%d\\%d.dat",ffxidir,no/0x80,no%0x80);
	else if( HIWORD(Dno)==6 ) wsprintf(filename,"%sROM6\\%d\\%d.dat",ffxidir,no/0x80,no%0x80);
	else if( HIWORD(Dno)==7 ) wsprintf(filename,"%sROM7\\%d\\%d.dat",ffxidir,no/0x80,no%0x80);
	else if (HIWORD(Dno) == 8) wsprintf(filename, "%sROM8\\%d\\%d.dat", ffxidir, no / 0x80, no % 0x80);
	else if (HIWORD(Dno) == 9) wsprintf(filename, "%sROM9\\%d\\%d.dat", ffxidir, no / 0x80, no % 0x80);
	else if (HIWORD(Dno) == 254) {
		sprintf(ComboString,"%d-%d-%d,",HIWORD(Dno),LOWORD(Dno)/0x80,LOWORD(Dno)%0x80);
		ComboNoBkup = SendMessage(GetDlgItem(hDlg2, IDC_COMBO10), CB_GETCURSEL, 0L, 0L);
		ComboNo = SendMessage(GetDlgItem(hDlg2, IDC_COMBO10), CB_FINDSTRING, 0, (LPARAM)ComboString);
		SendMessage(GetDlgItem(hDlg2, IDC_COMBO10), CB_SETCURSEL, ComboNo, 0);
		GetWindowText(GetDlgItem(hDlg2, IDC_COMBO10), ComboString, sizeof(ComboString));
		SendMessage(GetDlgItem(hDlg2, IDC_COMBO10), CB_SETCURSEL, ComboNoBkup, 0);
		sscanf(ComboString,"%d-%d-%d,%s",&dum1,&dum2,&dum3,PartsName);
		wsprintf(filename,"%s\\List\\Original\\NPC\\%s",execDir,PartsName);
	} else if( HIWORD(Dno)>=255 ) {
		sprintf(ComboString,"%d-%d-%d,",HIWORD(Dno),LOWORD(Dno)/0x80,LOWORD(Dno)%0x80);
		switch( LOWORD(Dno)/0x80 ) {
			case 1:
				ComboNo = SendMessage(GetDlgItem(hDlg2, IDC_COMBO2), CB_FINDSTRING, 0, (LPARAM)ComboString);
				GetWindowText(GetDlgItem(hDlg2, IDC_COMBO2), ComboString, sizeof(ComboString));
				break;
			case 2:
				ComboNo = SendMessage(GetDlgItem(hDlg2, IDC_COMBO3), CB_FINDSTRING, 0, (LPARAM)ComboString);
				GetWindowText(GetDlgItem(hDlg2, IDC_COMBO3), ComboString, sizeof(ComboString));
				break;
			case 3:
				ComboNo = SendMessage(GetDlgItem(hDlg2, IDC_COMBO4), CB_FINDSTRING, 0, (LPARAM)ComboString);
				GetWindowText(GetDlgItem(hDlg2, IDC_COMBO4), ComboString, sizeof(ComboString));
				break;
			case 4:
				ComboNo = SendMessage(GetDlgItem(hDlg2, IDC_COMBO5), CB_FINDSTRING, 0, (LPARAM)ComboString);
				GetWindowText(GetDlgItem(hDlg2, IDC_COMBO5), ComboString, sizeof(ComboString));
				break;
			case 5:
				ComboNo = SendMessage(GetDlgItem(hDlg2, IDC_COMBO6), CB_FINDSTRING, 0, (LPARAM)ComboString);
				GetWindowText(GetDlgItem(hDlg2, IDC_COMBO6), ComboString, sizeof(ComboString));
				break;
			case 6:
				ComboNo = SendMessage(GetDlgItem(hDlg2, IDC_COMBO7), CB_FINDSTRING, 0, (LPARAM)ComboString);
				GetWindowText(GetDlgItem(hDlg2, IDC_COMBO7), ComboString, sizeof(ComboString));
				break;
			case 7:
				ComboNo = SendMessage(GetDlgItem(hDlg2, IDC_COMBO10), CB_FINDSTRING, 0, (LPARAM)ComboString);
				GetWindowText(GetDlgItem(hDlg2, IDC_COMBO10), ComboString, sizeof(ComboString));
				break;
			case 8:
				ComboNo = SendMessage(GetDlgItem(hDlg2, IDC_COMBO11), CB_FINDSTRING, 0, (LPARAM)ComboString);
				GetWindowText(GetDlgItem(hDlg2, IDC_COMBO11), ComboString, sizeof(ComboString));
				break;
			case 9:
				ComboNo = SendMessage(GetDlgItem(hDlg2, IDC_COMBO12), CB_FINDSTRING, 0, (LPARAM)ComboString);
				GetWindowText(GetDlgItem(hDlg2, IDC_COMBO12), ComboString, sizeof(ComboString));
				break;
		}
		sscanf(ComboString,"%d-%d-%d,%s",&dum1,&dum2,&dum3,PartsName);
		wsprintf(filename,"%s\\List\\Original\\%s\\%s\\%s",
		execDir,OriginRace[HIWORD(Dno)-255],OriginParts[LOWORD(Dno)/0x80],PartsName);
	}
	return TRUE;
}

DWORD _GetFileNameFromIDsub(DWORD dwV,DWORD dwID)
{
  HANDLE hFile;
  char fname[512];
  DWORD dwRead;
  BYTE v;
  WORD f;
 
 // if( dwID>=51439 ) return 0;
  if( dwV!=1 && dwV!=2 ) return 0;
  // ROM2を調べる
  lstrcpy(fname,ffxidir);
  if( dwV==1 ) lstrcat(fname,"VTABLE.DAT");
  else if( dwV==2 ) lstrcat(fname,"ROM2\\VTABLE2.DAT");
  else if( dwV==3 ) lstrcat(fname,"ROM3\\VTABLE2.DAT");
  else if( dwV==4 ) lstrcat(fname,"ROM4\\VTABLE2.DAT");
  else if( dwV==5 ) lstrcat(fname,"ROM5\\VTABLE2.DAT");
  else if( dwV==6 ) lstrcat(fname,"ROM6\\VTABLE2.DAT");
  else if (dwV == 7) lstrcat(fname, "ROM7\\VTABLE2.DAT");
  else if (dwV == 8) lstrcat(fname, "ROM8\\VTABLE2.DAT");
  else  lstrcat(fname, "ROM9\\VTABLE2.DAT");
  hFile = CreateFile(fname,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_ARCHIVE,NULL);
  if( hFile==INVALID_HANDLE_VALUE ) return 0;
  SetFilePointer(hFile,dwID,NULL,FILE_BEGIN);
  ReadFile(hFile,&v,1,&dwRead,NULL);
  CloseHandle(hFile);
  if(dwRead!=1) return 0;
  if( v==0 ) return 0;
  lstrcpy(fname,ffxidir);
  if( dwV==1 ) lstrcat(fname,"FTABLE.DAT");
  else if( dwV==2 ) lstrcat(fname,"ROM2\\FTABLE2.DAT");
  else if( dwV==3 ) lstrcat(fname,"ROM3\\FTABLE2.DAT");
  else if( dwV==4 ) lstrcat(fname,"ROM4\\FTABLE2.DAT");
  else if( dwV==5 ) lstrcat(fname,"ROM5\\VTABLE2.DAT");
  else if( dwV==6 ) lstrcat(fname,"ROM6\\VTABLE2.DAT");
  else if (dwV == 7) lstrcat(fname, "ROM7\\VTABLE2.DAT");
  else if (dwV == 8) lstrcat(fname, "ROM8\\VTABLE2.DAT");
  else  lstrcat(fname, "ROM9\\VTABLE2.DAT");
  hFile = CreateFile(fname,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_ARCHIVE,NULL);
  if( hFile==INVALID_HANDLE_VALUE ) return 0;
  SetFilePointer(hFile,dwID*2,NULL,FILE_BEGIN);
  ReadFile(hFile,&f,2,&dwRead,NULL);
  CloseHandle(hFile);
  if(dwRead!=2) return 0;
  return (DWORD)MAKELONG(f,v);
}


BOOL GetFileNameFromDir(LPSTR filename,char *DataName )
{
	int		Type,Hi,Lo;

	Type = 1;
	sscanf(DataName,"%d-%d,",&Hi,&Lo);
	if(  Type==1 ) wsprintf(filename,"%sROM\\%d\\%d.dat",ffxidir,Hi,Lo);
	else if( Type==2 ) wsprintf(filename,"%sROM2\\%d\\%d.dat",ffxidir,Hi,Lo);
	else if( Type==3 ) wsprintf(filename,"%sROM3\\%d\\%d.dat",ffxidir,Hi,Lo);
	else if( Type==4 ) wsprintf(filename,"%sROM4\\%d\\%d.dat",ffxidir,Hi,Lo);
	else if( Type==5 ) wsprintf(filename,"%sROM5\\%d\\%d.dat",ffxidir,Hi,Lo);
	else if( Type==6 ) wsprintf(filename,"%sROM6\\%d\\%d.dat",ffxidir,Hi,Lo);
	else if (Type == 7) wsprintf(filename, "%sROM7\\%d\\%d.dat", ffxidir, Hi, Lo);
	else if (Type == 8) wsprintf(filename, "%sROM8\\%d\\%d.dat", ffxidir, Hi, Lo);
	else wsprintf(filename, "%sROM9\\%d\\%d.dat", ffxidir, Hi, Lo);
	return TRUE;
}

BOOL  GetFileNameFromDir2(LPSTR filename,char *DataName )
{
	int		Type,Hi,Lo;

	Type = 1;
	sscanf(DataName,"%d-%d-%d,",&Type,&Hi,&Lo);
	if(  Type==1 ) wsprintf(filename,"%sROM\\%d\\%d.dat",ffxidir,Hi,Lo);
	else if( Type==2 )  wsprintf(filename,"%sROM2\\%d\\%d.dat",ffxidir,Hi,Lo);
	else if( Type==3 )  wsprintf(filename,"%sROM3\\%d\\%d.dat",ffxidir,Hi,Lo);
	else if( Type==4 )  wsprintf(filename,"%sROM4\\%d\\%d.dat",ffxidir,Hi,Lo);
	else if( Type==5 ) wsprintf(filename,"%sROM5\\%d\\%d.dat",ffxidir,Hi,Lo);
	else if( Type==6 ) wsprintf(filename,"%sROM6\\%d\\%d.dat",ffxidir,Hi,Lo);
	else if (Type == 7) wsprintf(filename, "%sROM7\\%d\\%d.dat", ffxidir, Hi, Lo);
	else if (Type == 8) wsprintf(filename, "%sROM8\\%d\\%d.dat", ffxidir, Hi, Lo);
	else wsprintf(filename, "%sROM9\\%d\\%d.dat", ffxidir, Hi, Lo);
	return TRUE;
}

//======================================================================
//
//		WinMain関数
//
//======================================================================
int __stdcall WinMain( HINSTANCE inst, HINSTANCE prev, LPSTR cmd, int show )
{
	HKEY hKey;
	GetCurrentDirectory(sizeof(execDir),execDir);
#if 0
	strcpy(ffxidir,"C:\\cross1");
#else
	ffxidir[0]=0;
	if( ERROR_SUCCESS==RegOpenKeyEx(HKEY_LOCAL_MACHINE,"SOFTWARE\\PlayOnline\\InstallFolder",0,KEY_READ,&hKey) ){
		DWORD dwData = sizeof(ffxidir);
		DWORD dwType = REG_SZ;
		RegQueryValueEx(hKey,"0001",NULL,&dwType,(LPBYTE)ffxidir,&dwData);
		RegCloseKey( hKey );
	}
	if( !*ffxidir ){
		MessageBox(NULL,"FinalFantasyXIをインストールしているPCで起動してください。！","FF XI がインストールされてません",MB_OK);
		GetCurrentDirectory(sizeof(ffxidir),ffxidir);
		return -1;
	}
#endif
	if( lstrlen(ffxidir)>0 ){
		if( ffxidir[lstrlen(ffxidir)-1]!='\\' ){
			lstrcat(ffxidir,"\\");
		}
	}
	timeBeginPeriod( 1 );

	//============================================================
	// ウィンドウクラス登録
	//============================================================
	WNDCLASS wc;
	wc.style			= CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wc.lpfnWndProc		= WinProc; 
	wc.cbClsExtra		= 0;
	wc.cbWndExtra		= 0;
	wc.hInstance		= inst;
	wc.hIcon			= LoadIcon(inst, MAKEINTRESOURCE(IDI_ICON1));
	wc.hCursor			= LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground	= ( HBRUSH )GetStockObject( BLACK_BRUSH );
	wc.lpszMenuName		= MAKEINTRESOURCE(IDR_MENU1);
	wc.lpszClassName	= ClassName;
	if ( RegisterClass( &wc ) == NULL ) return false;
	
	//============================================================
	// ウィンドウサイズ取得
	//============================================================
	long window_w = g_mScreenWidth + GetSystemMetrics(SM_CXEDGE) + GetSystemMetrics(SM_CXBORDER) + GetSystemMetrics(SM_CXDLGFRAME);
	long window_h = g_mScreenHeight + GetSystemMetrics(SM_CYEDGE) + GetSystemMetrics(SM_CYBORDER) + GetSystemMetrics(SM_CYDLGFRAME) + GetSystemMetrics(SM_CYCAPTION);

	//============================================================
	// ウィンドウ生成
	//============================================================
	hWindow = CreateWindowEx(
				WS_EX_APPWINDOW,
				ClassName,
				AppName,
				WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_VISIBLE ,
				GetSystemMetrics(SM_CXSCREEN)/2 - window_w/2,
				GetSystemMetrics(SM_CYSCREEN)/2 - window_h/2,
				window_w,
				window_h,
				NULL,
				NULL,
				inst,
				NULL );
	// ダイアログ２作成
	hDlg2 = CreateDialog((HINSTANCE)GetWindowLong(hWindow,GWL_HINSTANCE),MAKEINTRESOURCE(IDD_DIALOG2),NULL,(DLGPROC)Dlg2Proc);
	InvalidateRect(hDlg2, NULL, TRUE);
	SetWindowPos( hDlg2,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE );
	ShowWindow( hDlg2,SW_HIDE );
	ShowWindow( hWindow,SW_SHOW );
	//============================================================
	// DirectXGraphics初期化
	//============================================================
	if ( InitD3D() == false ) return false;
	//============================================================
	// 描画処理初期化
	//============================================================
	if ( InitRender() == false ) return false;
	//============================================================
	// メッセージループ
	//============================================================
	MSG msg;
	D3DXVECTOR3	PosPC;
	while ( true )
	{
		//==================================================
		// メッセージ処理
		//==================================================
		if ( PeekMessage( &msg, 0, 0, 0, PM_REMOVE ) ) {
			if( !IsDialogMessage(hDlg2,&msg) ) {
				if ( msg.message == WM_QUIT ) break;
				TranslateMessage( &msg );
				DispatchMessage( &msg );
			}
		}
		//==================================================
		// 他
		//==================================================
		else{
			//======================================
			// FPSの計測
			//======================================
			
			static DWORD cnt=0,BeforeTime = timeGetTime();
			DWORD NowTime = timeGetTime();
			int		x,y,z;

			if ( (NowTime - BeforeTime-25*cnt) >= 25 ) {
				cnt++;
			}
			if ( NowTime - BeforeTime >= 500 ) {
				char FpsStr[128];
				pPC->GetWorldPosition( PosPC );
				x = (int)PosPC.x; y = (int)PosPC.y; z = (int)PosPC.z;
				sprintf( FpsStr, "%s  [ POS : %4d %4d %4d ] [ FPS : %03d/s ] [ %upolygon/sec ]", AppName,
				x,y,z,FPS*2, Polygons*2 );
				SetWindowText( hWindow, FpsStr );
				BeforeTime = NowTime;
				Polygons = 0;
				FPS = 0;
				cnt = 0;
			}

			FPS++;

			//======================================
			// Direct3Dの描画
			//======================================

			// バックバッファと Z バッファをクリア
			GetDevice()->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER| D3DCLEAR_STENCIL, D3DCOLOR_XRGB(200,200,255), 1.f, 0 );
//			GetDevice()->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0,0,0), 1, 0 );
			// シーン開始
			if SUCCEEDED( GetDevice()->BeginScene() ) {
				// 各種処理
				Rendering();

				// シーン終了
				GetDevice()->EndScene();

				// バックバッファの内容をプライマリに転送
				if FAILED( GetDevice()->Present( NULL, NULL, NULL, NULL ) ) {
					// リセット
					GetDevice()->Reset( GetAdapter() );
				}
			}
		}
	}

	//============================================================
	// 描画処理開放
	//============================================================
	UnInitRender();

	//============================================================
	// DirectXGraphics開放
	//============================================================
	ReleaseD3D();

	//============================================================
	// おしまい
	//============================================================
	timeEndPeriod( 1 );

	return msg.wParam;
}

void Inventory_init(void)
{
	char	ComboString[128];
	int		ComboNo;
	//	種族の設定
	ComboNo = (int)(DWORD)SendMessage(GetDlgItem(hDlg2, IDC_COMBO1), CB_GETCURSEL, 0L, 0L);
	pPC->SetType( ComboNo );
	GetWindowText(GetDlgItem(hDlg2, IDC_COMBO1), ComboString, sizeof(ComboString));
	pPC->SetRace(ConvertStr2Dno2( ComboString ));
	//	顔の設定
	GetWindowText(GetDlgItem(hDlg2, IDC_COMBO2), ComboString, sizeof(ComboString));
	pPC->SetFace(ConvertStr2Dno2( ComboString ));
	//	頭の設定
	GetWindowText(GetDlgItem(hDlg2, IDC_COMBO3), ComboString, sizeof(ComboString));
	pPC->SetHead(ConvertStr2Dno2( ComboString ));
	//	胴の設定
	GetWindowText(GetDlgItem(hDlg2, IDC_COMBO4), ComboString, sizeof(ComboString));
	pPC->SetBody(ConvertStr2Dno2( ComboString ));
	//	手の設定
	GetWindowText(GetDlgItem(hDlg2, IDC_COMBO5), ComboString, sizeof(ComboString));
	pPC->SetHand(ConvertStr2Dno2( ComboString ));
	//	脚の設定
	GetWindowText(GetDlgItem(hDlg2, IDC_COMBO6), ComboString, sizeof(ComboString));
	pPC->SetLegs(ConvertStr2Dno2( ComboString ));
 	//	足の設定
	GetWindowText(GetDlgItem(hDlg2, IDC_COMBO7), ComboString, sizeof(ComboString));
	pPC->SetFoot(ConvertStr2Dno2( ComboString ));
 	//	右手武器の設定
	GetWindowText(GetDlgItem(hDlg2, IDC_COMBO10), ComboString, sizeof(ComboString));
	pPC->SetRightWeapon(ConvertStr2Dno2( ComboString ));
 	//	左手武器の設定
	GetWindowText(GetDlgItem(hDlg2, IDC_COMBO11), ComboString, sizeof(ComboString));
	pPC->SetLeftWeapon(ConvertStr2Dno2( ComboString ));
 	//	遠隔の設定
	GetWindowText(GetDlgItem(hDlg2, IDC_COMBO12), ComboString, sizeof(ComboString));
	pPC->SetRemoteWeapon(ConvertStr2Dno2( ComboString ));
}

void InventoryReverce(void)
{
	char	ComboString[128];
	int		ComboNo;
	//	顔の設定
	ComboNo = pPC->GetFace();
	sprintf(ComboString,"%d-%d-%d,",HIWORD(ComboNo),LOWORD(ComboNo)/0x80,LOWORD(ComboNo)%0x80);
	ComboNo = SendMessage(GetDlgItem(hDlg2, IDC_COMBO2), CB_FINDSTRING, 0, (LPARAM)ComboString);
	if( ComboNo < 0 ) {
		SendMessage(GetDlgItem(hDlg2, IDC_COMBO2), CB_SETCURSEL, 0, 0);
		GetWindowText(GetDlgItem(hDlg2, IDC_COMBO2), ComboString, sizeof(ComboString));
		pPC->SetFace(ConvertStr2Dno2( ComboString ));
	} else {
		SendMessage(GetDlgItem(hDlg2, IDC_COMBO2), CB_SETCURSEL, ComboNo, 0);
	}
	//	頭の設定
	ComboNo = pPC->GetHead();
	sprintf(ComboString,"%d-%d-%d,",HIWORD(ComboNo),LOWORD(ComboNo)/0x80,LOWORD(ComboNo)%0x80);
	ComboNo = SendMessage(GetDlgItem(hDlg2, IDC_COMBO3), CB_FINDSTRING, 0,(LPARAM)ComboString);
	if( ComboNo < 0 ) {
		SendMessage(GetDlgItem(hDlg2, IDC_COMBO3), CB_SETCURSEL, 0, 0);
		GetWindowText(GetDlgItem(hDlg2, IDC_COMBO3), ComboString, sizeof(ComboString));
		pPC->SetHead(ConvertStr2Dno2( ComboString ));
	} else {
		SendMessage(GetDlgItem(hDlg2, IDC_COMBO3), CB_SETCURSEL, ComboNo, 0);
	}
	//	胴の設定
	ComboNo = pPC->GetBody();
	sprintf(ComboString,"%d-%d-%d,",HIWORD(ComboNo),LOWORD(ComboNo)/0x80,LOWORD(ComboNo)%0x80);
	ComboNo = SendMessage(GetDlgItem(hDlg2, IDC_COMBO4), CB_FINDSTRING, 0, (LPARAM)ComboString);
	if( ComboNo < 0 ) {
		SendMessage(GetDlgItem(hDlg2, IDC_COMBO4), CB_SETCURSEL, 0, 0);
		GetWindowText(GetDlgItem(hDlg2, IDC_COMBO4), ComboString, sizeof(ComboString));
		pPC->SetBody(ConvertStr2Dno2( ComboString ));
	} else {
		SendMessage(GetDlgItem(hDlg2, IDC_COMBO4), CB_SETCURSEL, ComboNo, 0);
	}
	//	手の設定
	ComboNo = pPC->GetHand();
	sprintf(ComboString,"%d-%d-%d,",HIWORD(ComboNo),LOWORD(ComboNo)/0x80,LOWORD(ComboNo)%0x80);
	ComboNo = SendMessage(GetDlgItem(hDlg2, IDC_COMBO5), CB_FINDSTRING, 0,(LPARAM)ComboString);
	if( ComboNo < 0 ) {
		SendMessage(GetDlgItem(hDlg2, IDC_COMBO5), CB_SETCURSEL, 0, 0);
		GetWindowText(GetDlgItem(hDlg2, IDC_COMBO5), ComboString, sizeof(ComboString));
		pPC->SetHand(ConvertStr2Dno2( ComboString ));
	} else {
		SendMessage(GetDlgItem(hDlg2, IDC_COMBO5), CB_SETCURSEL, ComboNo, 0);
	}
	//	脚の設定
	ComboNo = pPC->GetLegs();
	sprintf(ComboString,"%d-%d-%d,",HIWORD(ComboNo),LOWORD(ComboNo)/0x80,LOWORD(ComboNo)%0x80);
	ComboNo = SendMessage(GetDlgItem(hDlg2, IDC_COMBO6), CB_FINDSTRING, 0, (LPARAM)ComboString);
	if( ComboNo < 0 ) {
		SendMessage(GetDlgItem(hDlg2, IDC_COMBO6), CB_SETCURSEL, 0, 0);
		GetWindowText(GetDlgItem(hDlg2, IDC_COMBO6), ComboString, sizeof(ComboString));
		pPC->SetLegs(ConvertStr2Dno2( ComboString ));
	} else {
		SendMessage(GetDlgItem(hDlg2, IDC_COMBO6), CB_SETCURSEL, ComboNo, 0);
	}
 	//	足の設定
	ComboNo = pPC->GetFoot();
	sprintf(ComboString,"%d-%d-%d,",HIWORD(ComboNo),LOWORD(ComboNo)/0x80,LOWORD(ComboNo)%0x80);
	ComboNo = SendMessage(GetDlgItem(hDlg2, IDC_COMBO7), CB_FINDSTRING, 0, (LPARAM)ComboString);
	if( ComboNo < 0 ) {
		SendMessage(GetDlgItem(hDlg2, IDC_COMBO7), CB_SETCURSEL, 0, 0);
		GetWindowText(GetDlgItem(hDlg2, IDC_COMBO7), ComboString, sizeof(ComboString));
		pPC->SetFoot(ConvertStr2Dno2( ComboString ));
	} else {
		SendMessage(GetDlgItem(hDlg2, IDC_COMBO7), CB_SETCURSEL, ComboNo, 0);
	}
 	//	右手武器の設定
	ComboNo = pPC->GetRightWeapon();
	sprintf(ComboString,"%d-%d-%d,",HIWORD(ComboNo),LOWORD(ComboNo)/0x80,LOWORD(ComboNo)%0x80);
	ComboNo = SendMessage(GetDlgItem(hDlg2, IDC_COMBO10), CB_FINDSTRING, 0, (LPARAM)ComboString);
	if( ComboNo < 0 ) {
		SendMessage(GetDlgItem(hDlg2, IDC_COMBO10), CB_SETCURSEL, 0, 0);
		GetWindowText(GetDlgItem(hDlg2, IDC_COMBO10), ComboString, sizeof(ComboString));
		pPC->SetRightWeapon(ConvertStr2Dno2( ComboString ));
	} else {
		SendMessage(GetDlgItem(hDlg2, IDC_COMBO10), CB_SETCURSEL, ComboNo, 0);
	}
 	//	左手武器の設定
	ComboNo = pPC->GetLeftWeapon();
	sprintf(ComboString,"%d-%d-%d,",HIWORD(ComboNo),LOWORD(ComboNo)/0x80,LOWORD(ComboNo)%0x80);
	ComboNo = SendMessage(GetDlgItem(hDlg2, IDC_COMBO11), CB_FINDSTRING, 0, (LPARAM)ComboString);
	if( ComboNo < 0 ) {
		SendMessage(GetDlgItem(hDlg2, IDC_COMBO11), CB_SETCURSEL, 0, 0);
		GetWindowText(GetDlgItem(hDlg2, IDC_COMBO11), ComboString, sizeof(ComboString));
		pPC->SetLeftWeapon(ConvertStr2Dno2( ComboString ));
	} else {
		SendMessage(GetDlgItem(hDlg2, IDC_COMBO11), CB_SETCURSEL, ComboNo, 0);
	}
 	//	遠隔の設定
	ComboNo = pPC->GetRemoteWeapon();
	sprintf(ComboString,"%d-%d-%d,",HIWORD(ComboNo),LOWORD(ComboNo)/0x80,LOWORD(ComboNo)%0x80);
	ComboNo = SendMessage(GetDlgItem(hDlg2, IDC_COMBO12), CB_FINDSTRING, 0, (LPARAM)ComboString);
	if( ComboNo < 0 ) {
		SendMessage(GetDlgItem(hDlg2, IDC_COMBO12), CB_SETCURSEL, 0, 0);
		GetWindowText(GetDlgItem(hDlg2, IDC_COMBO12), ComboString, sizeof(ComboString));
		pPC->SetRemoteWeapon(ConvertStr2Dno2( ComboString ));
	} else {
		SendMessage(GetDlgItem(hDlg2, IDC_COMBO12), CB_SETCURSEL, ComboNo, 0);
	}
}

bool ChangeInventory( HWND in_hWnd, char* RaceName ) 
{
	FILE	*fd;
	char	ListName[4096];
	char	ComboString[128];
	int		i;

	//	顔装備　
	while( SendMessage(GetDlgItem(in_hWnd, IDC_COMBO2), CB_GETCOUNT, 0, 0) != 0) {
		SendMessage(GetDlgItem(in_hWnd, IDC_COMBO2), CB_DELETESTRING, 0, 0);
	}
	sprintf(ListName,"%s\\List\\%s顔.lst",execDir,RaceName);
	if ((fd = fopen(ListName, "r")) < 0) return false;
	for( i=0 ; fgets(ComboString,sizeof(ComboString),fd) ; ) {
		if( strlen(ComboString)<=1 ) continue;
		sprintf(ListName,"1-%s",ComboString);
		SendMessage(GetDlgItem(in_hWnd, IDC_COMBO2), CB_INSERTSTRING, (WPARAM)i, (LPARAM)ListName);
		i++;
	}
	fclose(fd);
	SendMessage(GetDlgItem(in_hWnd, IDC_COMBO2), CB_SETCURSEL, (WPARAM)0, 0L);
	//	頭装備　
	while( SendMessage(GetDlgItem(in_hWnd, IDC_COMBO3), CB_GETCOUNT, 0, 0) != 0) {
		SendMessage(GetDlgItem(in_hWnd, IDC_COMBO3), CB_DELETESTRING, 0, 0);
	}
	sprintf(ListName,"%s\\List\\%s頭.lst",execDir,RaceName);
	if ((fd = fopen(ListName, "r")) < 0) return false;
	for( i=0 ; fgets(ComboString,sizeof(ComboString),fd) ; ) {
		if( strlen(ComboString)<=1 ) continue;
		sprintf(ListName,"1-%s",ComboString);
		SendMessage(GetDlgItem(in_hWnd, IDC_COMBO3), CB_INSERTSTRING, (WPARAM)i, (LPARAM)ListName);
		i++;
	}
	fclose(fd);
	SendMessage(GetDlgItem(in_hWnd, IDC_COMBO3), CB_SETCURSEL, (WPARAM)0, 0L);
	//	胴装備　
	while( SendMessage(GetDlgItem(in_hWnd, IDC_COMBO4), CB_GETCOUNT, 0, 0) != 0) {
		SendMessage(GetDlgItem(in_hWnd, IDC_COMBO4), CB_DELETESTRING, 0, 0);
	}
	sprintf(ListName,"%s\\List\\%s胴.lst",execDir,RaceName);
	if ((fd = fopen(ListName, "r")) < 0) return false;
	for( i=0 ; fgets(ComboString,sizeof(ComboString),fd) ; ) {
		if( strlen(ComboString)<=1 ) continue;
		sprintf(ListName,"1-%s",ComboString);
		SendMessage(GetDlgItem(in_hWnd, IDC_COMBO4), CB_INSERTSTRING, (WPARAM)i, (LPARAM)ListName);
		i++;
	}
	fclose(fd);
	SendMessage(GetDlgItem(in_hWnd, IDC_COMBO4), CB_SETCURSEL, (WPARAM)0, 0L);
	//	両手装備　
	while( SendMessage(GetDlgItem(in_hWnd, IDC_COMBO5), CB_GETCOUNT, 0, 0) != 0) {
		SendMessage(GetDlgItem(in_hWnd, IDC_COMBO5), CB_DELETESTRING, 0, 0);
	}
	sprintf(ListName,"%s\\List\\%s両手.lst",execDir,RaceName);
	if ((fd = fopen(ListName, "r")) < 0) return false;
	for( i=0 ; fgets(ComboString,sizeof(ComboString),fd) ; ) {
		if( strlen(ComboString)<=1 ) continue;
		sprintf(ListName,"1-%s",ComboString);
		SendMessage(GetDlgItem(in_hWnd, IDC_COMBO5), CB_INSERTSTRING, (WPARAM)i, (LPARAM)ListName);
		i++;
	}
	fclose(fd);
	SendMessage(GetDlgItem(in_hWnd, IDC_COMBO5), CB_SETCURSEL, (WPARAM)0, 0L);
	//	両脚装備　
	while( SendMessage(GetDlgItem(in_hWnd, IDC_COMBO6), CB_GETCOUNT, 0, 0) != 0) {
		SendMessage(GetDlgItem(in_hWnd, IDC_COMBO6), CB_DELETESTRING, 0, 0);
	}
	sprintf(ListName,"%s\\List\\%s両脚.lst",execDir,RaceName);
	if ((fd = fopen(ListName, "r")) < 0) return false;
	for( i=0 ; fgets(ComboString,sizeof(ComboString),fd) ; ) {
		if( strlen(ComboString)<=1 ) continue;
		sprintf(ListName,"1-%s",ComboString);
		SendMessage(GetDlgItem(in_hWnd, IDC_COMBO6), CB_INSERTSTRING, (WPARAM)i, (LPARAM)ListName);
		i++;
	}
	fclose(fd);
	SendMessage(GetDlgItem(in_hWnd, IDC_COMBO6), CB_SETCURSEL, (WPARAM)0, 0L);
	//	両足装備　
	while( SendMessage(GetDlgItem(in_hWnd, IDC_COMBO7), CB_GETCOUNT, 0, 0) != 0) {
		SendMessage(GetDlgItem(in_hWnd, IDC_COMBO7), CB_DELETESTRING, 0, 0);
	}
	sprintf(ListName,"%s\\List\\%s両足.lst",execDir,RaceName);
	if ((fd = fopen(ListName, "r")) < 0) return false;
	for( i=0 ; fgets(ComboString,sizeof(ComboString),fd) ; ) {
		if( strlen(ComboString)<=1 ) continue;
		sprintf(ListName,"1-%s",ComboString);
		SendMessage(GetDlgItem(in_hWnd, IDC_COMBO7), CB_INSERTSTRING, (WPARAM)i, (LPARAM)ListName);
		i++;
	}
	fclose(fd);
	SendMessage(GetDlgItem(in_hWnd, IDC_COMBO7), CB_SETCURSEL, (WPARAM)0, 0L);
	//	右手武器　
	while( SendMessage(GetDlgItem(in_hWnd, IDC_COMBO10), CB_GETCOUNT, 0, 0) != 0) {
		SendMessage(GetDlgItem(in_hWnd, IDC_COMBO10), CB_DELETESTRING, 0, 0);
	}
	sprintf(ListName,"%s\\List\\%s武器R.lst",execDir,RaceName);
	if ((fd = fopen(ListName, "r")) < 0) return false;
	for( i=0 ; fgets(ComboString,sizeof(ComboString),fd) ; ) {
		if( strlen(ComboString)<=1 ) continue;
		sprintf(ListName,"1-%s",ComboString);
		SendMessage(GetDlgItem(in_hWnd, IDC_COMBO10), CB_INSERTSTRING, (WPARAM)i, (LPARAM)ListName);
		i++;
	}
	fclose(fd);
	SendMessage(GetDlgItem(in_hWnd, IDC_COMBO10), CB_SETCURSEL, (WPARAM)0, 0L);
	//	左手武器　
	while( SendMessage(GetDlgItem(in_hWnd, IDC_COMBO11), CB_GETCOUNT, 0, 0) != 0) {
		SendMessage(GetDlgItem(in_hWnd, IDC_COMBO11), CB_DELETESTRING, 0, 0);
	}
	sprintf(ListName,"%s\\List\\%s武器L.lst",execDir,RaceName);
	if ((fd = fopen(ListName, "r")) < 0) return false;
	for( i=0 ; fgets(ComboString,sizeof(ComboString),fd) ; ) {
		if( strlen(ComboString)<=1 ) continue;
		sprintf(ListName,"1-%s",ComboString);
		SendMessage(GetDlgItem(in_hWnd, IDC_COMBO11), CB_INSERTSTRING, (WPARAM)i, (LPARAM)ListName);
		i++;
	}
	fclose(fd);
	SendMessage(GetDlgItem(in_hWnd, IDC_COMBO11), CB_SETCURSEL, (WPARAM)0, 0L);
	//	遠隔武器　
	while( SendMessage(GetDlgItem(in_hWnd, IDC_COMBO12), CB_GETCOUNT, 0, 0) != 0) {
		SendMessage(GetDlgItem(in_hWnd, IDC_COMBO12), CB_DELETESTRING, 0, 0);
	}
	sprintf(ListName,"%s\\List\\%s遠隔.lst",execDir,RaceName);
	if ((fd = fopen(ListName, "r")) < 0) return false;
	for( i=0 ; fgets(ComboString,sizeof(ComboString),fd) ; ) {
		if( strlen(ComboString)<=1 ) continue;
		sprintf(ListName,"1-%s",ComboString);
		SendMessage(GetDlgItem(in_hWnd, IDC_COMBO12), CB_INSERTSTRING, (WPARAM)i, (LPARAM)ListName);
		i++;
	}
	fclose(fd);
	SendMessage(GetDlgItem(in_hWnd, IDC_COMBO12), CB_SETCURSEL, (WPARAM)0, 0L);
	return true;
}


 //　種族、装備等　イベント処理
LRESULT CALLBACK Dlg2Proc(HWND in_hWnd, UINT in_Message,WPARAM in_wParam, LPARAM in_lParam )
{
	char	ComboString[128],RaceName[32];
	int		i,dum1,dum2,dum3,cnt;
	FILE			*fd;
	char	ListName[2048],FileName[128];
	CMotionFrame *pMotionFrame;

	switch( in_Message ) {
        case WM_INITDIALOG:
            SendMessage(GetDlgItem(in_hWnd, IDC_CHECK1), BM_SETCHECK, (WPARAM)0, 0L);
			g_mDispIdl = false;
            SendMessage(GetDlgItem(in_hWnd, IDC_CHECK2), BM_SETCHECK, (WPARAM)0, 0L);
			g_mDispWire = false;
			//SendMessage(GetDlgItem(in_hWnd, IDC_SPIN1), UDM_SETBUDDY, (WPARAM)GetDlgItem(in_hWnd, IDC_EDIT1), 0);     // 対応するEditBox指定
			//SendMessage(GetDlgItem(in_hWnd, IDC_SPIN1), UDM_SETRANGE, (WPARAM)1, (LPARAM)128);              // 範囲指定
			//SendMessage(GetDlgItem(in_hWnd, IDC_SPIN1), UDM_SETPOS, 0, (LPARAM)1);       // 初期値の指定
			//SetDlgItemText(in_hWnd,IDC_EDIT2,(LPCTSTR)"0");
			// PC モーション初期値セット
			SendMessage(GetDlgItem(in_hWnd, IDC_COMBO8), CB_INSERTSTRING, (WPARAM)0, (LPARAM)"idl");
			SendMessage(GetDlgItem(in_hWnd, IDC_COMBO8), CB_SETCURSEL, (WPARAM)0, 0L);
			SendMessage(GetDlgItem(in_hWnd, IDC_COMBO9), CB_INSERTSTRING, (WPARAM)0, (LPARAM)"idl");
			SendMessage(GetDlgItem(in_hWnd, IDC_COMBO9), CB_SETCURSEL, (WPARAM)0, 0L);
			for (i = 0; i<9; i++)
		        SendMessage(GetDlgItem(in_hWnd, IDC_COMBO1), CB_INSERTSTRING, (WPARAM)i, (LPARAM)ListRace[i]);
		    SendMessage(GetDlgItem(in_hWnd, IDC_COMBO1), CB_SETCURSEL, (WPARAM)0, 0L);
            SetFocus(GetDlgItem(in_hWnd, IDC_COMBO1));
			GetWindowText(GetDlgItem(in_hWnd, IDC_COMBO1), ComboString, sizeof(ComboString));
			sscanf(ComboString,"%d-%d-%d,%s",&dum1,&dum2,&dum3,RaceName);
			ChangeInventory( in_hWnd, RaceName );
 			SendMessage(GetDlgItem(in_hWnd, IDC_RADIO8), BM_SETCHECK, (WPARAM)1, 0L);
			g_mPCFlag = true;
			sprintf(ListName,"%s\\List\\NPC_ROM1.lst",execDir);
			if ((fd = fopen(ListName, "r")) < 0) return false;
			strcpy(ComboString,"1-");
			i=0;
			while( fgets(&ComboString[2],sizeof(ComboString),fd) ) {
				if( ComboString[2]==';' ) continue;
				SendMessage(GetDlgItem(in_hWnd, IDC_COMBO31), CB_INSERTSTRING, (WPARAM)i, (LPARAM)ComboString);
				i++;
			}
			fclose(fd);
			sprintf(ListName,"%s\\List\\NPC_ROM2.lst",execDir);
			if ((fd = fopen(ListName, "r")) >= 0) {
				strcpy(ComboString,"2-");
				while ( fgets(&ComboString[2],sizeof(ComboString),fd) ) {
					if( ComboString[2]==';' ) continue;
					SendMessage(GetDlgItem(in_hWnd, IDC_COMBO31), CB_INSERTSTRING, (WPARAM)i, (LPARAM)ComboString);
					i++;
				}
				fclose(fd);
			}
			sprintf(ListName,"%s\\List\\NPC_ROM3.lst",execDir);
			if ((fd = fopen(ListName, "r")) != NULL ) {
				strcpy(ComboString,"3-");
				while( fgets(&ComboString[2],sizeof(ComboString),fd) ) {
					if( ComboString[2]==';' ) continue;
					SendMessage(GetDlgItem(in_hWnd, IDC_COMBO31), CB_INSERTSTRING, (WPARAM)i, (LPARAM)ComboString);
					i++;
				}
				fclose(fd);
			}
			sprintf(ListName,"%s\\List\\NPC_ROM4.lst",execDir);
			if ((fd = fopen(ListName, "r")) != NULL ) {
				strcpy(ComboString,"4-");
				while( fgets(&ComboString[2],sizeof(ComboString),fd) ) {
					if( ComboString[2]==';' ) continue;
					SendMessage(GetDlgItem(in_hWnd, IDC_COMBO31), CB_INSERTSTRING, (WPARAM)i, (LPARAM)ComboString);
					i++;
				}
				fclose(fd);
			}
			sprintf(ListName,"%s\\List\\NPC_ROM5.lst",execDir);
			if ((fd = fopen(ListName, "r")) != NULL ) {
				strcpy(ComboString,"5-");
				while( fgets(&ComboString[2],sizeof(ComboString),fd) ) {
					if( ComboString[2]==';' ) continue;
					SendMessage(GetDlgItem(in_hWnd, IDC_COMBO31), CB_INSERTSTRING, (WPARAM)i, (LPARAM)ComboString);
					i++;
				}
				fclose(fd);
			}
			sprintf(ListName,"%s\\List\\NPC_ROM6.lst",execDir);
			if ((fd = fopen(ListName, "r")) != NULL ) {
				strcpy(ComboString,"6-");
				while ( fgets(&ComboString[2],sizeof(ComboString),fd) ) {
					if( ComboString[2]==';' ) continue;
					SendMessage(GetDlgItem(in_hWnd, IDC_COMBO31), CB_INSERTSTRING, (WPARAM)i, (LPARAM)ComboString);
					i++;
				}
				fclose(fd);
			}
			sprintf(ListName,"%s\\List\\NPC_ROM7.lst",execDir);
			if ((fd = fopen(ListName, "r")) != NULL ) {
				strcpy(ComboString,"7-");
				while( fgets(&ComboString[2],sizeof(ComboString),fd) ) {
					if( ComboString[2]==';' ) continue;
					SendMessage(GetDlgItem(in_hWnd, IDC_COMBO31), CB_INSERTSTRING, (WPARAM)i, (LPARAM)ComboString);
					i++;
				}
				fclose(fd);
			}
			sprintf(ListName, "%s\\List\\NPC_ROM8.lst", execDir);
			if ((fd = fopen(ListName, "r")) != NULL) {
				strcpy(ComboString, "8-");
				while (fgets(&ComboString[2], sizeof(ComboString), fd)) {
					if (ComboString[2] == ';') continue;
					SendMessage(GetDlgItem(in_hWnd, IDC_COMBO31), CB_INSERTSTRING, (WPARAM)i, (LPARAM)ComboString);
					i++;
				}
				fclose(fd);
			}
			sprintf(ListName, "%s\\List\\NPC_ROM9.lst", execDir);
			if ((fd = fopen(ListName, "r")) != NULL) {
				strcpy(ComboString, "9-");
				while (fgets(&ComboString[2], sizeof(ComboString), fd)) {
					if (ComboString[2] == ';') continue;
					SendMessage(GetDlgItem(in_hWnd, IDC_COMBO31), CB_INSERTSTRING, (WPARAM)i, (LPARAM)ComboString);
					i++;
				}
				fclose(fd);
			}
			SendMessage(GetDlgItem(in_hWnd, IDC_COMBO31), CB_SETCURSEL, (WPARAM)0, 0L);
			break;
 		case WM_COMMAND:
			switch( LOWORD(in_wParam ) ) {
				case IDC_COMBO1:
					if( HIWORD(in_wParam) == CBN_SELCHANGE ) {
						//	種族の設定
						dum1 =SendMessage(GetDlgItem(in_hWnd, IDC_COMBO1), CB_GETCURSEL,(WPARAM)0,(LPARAM)0L);
						g_mShlBoneNoR = g_mShlBoneTbl[dum1][0];
						g_mShlBoneNoL = g_mShlBoneTbl[dum1][1];
						GetWindowText(GetDlgItem(in_hWnd, IDC_COMBO1), ComboString, sizeof(ComboString));
						sscanf(ComboString,"%d-%d-%d,%s",&dum1,&dum2,&dum3,RaceName);
						ChangeInventory( in_hWnd, RaceName );
						Inventory_init();
						pPC->LoadPCParts();
						pPC->LoadPCMotion();
						pPC->InitialTransform();
						pPC->ConvertMesh();
						while (SendMessage(GetDlgItem(in_hWnd, IDC_COMBO8), CB_GETCOUNT, 0, 0) != 0) {
							SendMessage(GetDlgItem(in_hWnd, IDC_COMBO8), CB_DELETESTRING, 0, 0);
						}
						pMotionFrame = (CMotionFrame*)pPC->m_motions.Top();
						cnt = 0;
						while (pMotionFrame) {
							char mName[6]; strncpy(mName, (char*)pMotionFrame->m_Name, 3); mName[3] = '\0';
							if (SendMessage(GetDlgItem(in_hWnd, IDC_COMBO8), CB_FINDSTRING, 0, (LPARAM)mName) < 0) {
								SendMessage(GetDlgItem(in_hWnd, IDC_COMBO8), CB_ADDSTRING, (WPARAM)0, (LPARAM)mName);
								cnt++;
							}
							pMotionFrame = (CMotionFrame*)pMotionFrame->Next;
						}
						SendMessage(GetDlgItem(in_hWnd, IDC_COMBO8), CB_SETCURSEL, (WPARAM)0, 0L);
						GetWindowText(GetDlgItem(in_hWnd, IDC_COMBO8), ComboString, sizeof(ComboString));
						pPC->SetMotionName(ComboString);
						pPC->LoadPCMotion();
					}
					break;
				case IDC_COMBO2:
					if( HIWORD(in_wParam) == CBN_SELCHANGE ) {
						//	顔の設定
						GetWindowText(GetDlgItem(in_hWnd, IDC_COMBO2), ComboString, sizeof(ComboString));
						pPC->SetFace(ConvertStr2Dno2( ComboString ));
						pPC->LoadPCParts();
						pPC->LoadPCMotion();
						pPC->InitialTransform();
						pPC->ConvertMesh();
					}
					break;
				case IDC_COMBO3:
					if( HIWORD(in_wParam) == CBN_SELCHANGE ) {
						//	頭の設定
						GetWindowText(GetDlgItem(in_hWnd, IDC_COMBO3), ComboString, sizeof(ComboString));
						pPC->SetHead(ConvertStr2Dno2( ComboString ));
						pPC->LoadPCParts();
						pPC->LoadPCMotion();
						pPC->InitialTransform();
						pPC->ConvertMesh();
					}
					break;
				case IDC_COMBO4:
					if( HIWORD(in_wParam) == CBN_SELCHANGE ) {
						//	胴の設定
						GetWindowText(GetDlgItem(in_hWnd, IDC_COMBO4), ComboString, sizeof(ComboString));
						pPC->SetBody(ConvertStr2Dno2( ComboString ));
						pPC->LoadPCParts();
						pPC->LoadPCMotion();
						pPC->InitialTransform();
						pPC->ConvertMesh();
					}
					break;
				case IDC_COMBO5:
					if( HIWORD(in_wParam) == CBN_SELCHANGE ) {
						//	手の設定
						GetWindowText(GetDlgItem(in_hWnd, IDC_COMBO5), ComboString, sizeof(ComboString));
						pPC->SetHand(ConvertStr2Dno2( ComboString ));
						pPC->LoadPCParts();
						pPC->LoadPCMotion();
						pPC->InitialTransform();
						pPC->ConvertMesh();
					}
					break;
				case IDC_COMBO6:
					if( HIWORD(in_wParam) == CBN_SELCHANGE ) {
						//	脚の設定
						GetWindowText(GetDlgItem(in_hWnd, IDC_COMBO6), ComboString, sizeof(ComboString));
						pPC->SetLegs(ConvertStr2Dno2( ComboString ));
						pPC->LoadPCParts();
						pPC->LoadPCMotion();
						pPC->InitialTransform();
						pPC->ConvertMesh();
					}
					break;
				case IDC_COMBO7:
					if( HIWORD(in_wParam) == CBN_SELCHANGE ) {
 						//	足の設定
						GetWindowText(GetDlgItem(in_hWnd, IDC_COMBO7), ComboString, sizeof(ComboString));
						pPC->SetFoot(ConvertStr2Dno2( ComboString ));
						pPC->LoadPCParts();
						pPC->LoadPCMotion();
						pPC->InitialTransform();
						pPC->ConvertMesh();
					}
					break;
				case IDC_COMBO8:
					if (HIWORD(in_wParam) == CBN_SELCHANGE) {
						//	PCのモーション設定
						GetWindowText(GetDlgItem(in_hWnd, IDC_COMBO8), ComboString, sizeof(ComboString));
						pPC->SetMotionName(ComboString);
						pPC->LoadPCMotion();
					}
					break;
				case IDC_COMBO9:
					if (HIWORD(in_wParam) == CBN_SELCHANGE) {
						//	NPCのモーション設定
						GetWindowText(GetDlgItem(in_hWnd, IDC_COMBO9), ComboString, sizeof(ComboString));
						pNPC->SetMotionName(ComboString);
						pNPC->LoadNPCMotion();
					}
					break;
				case IDC_COMBO10:
					if( HIWORD(in_wParam) == CBN_SELCHANGE ) {
 						//	右手武器の設定
						GetWindowText(GetDlgItem(in_hWnd, IDC_COMBO10), ComboString, sizeof(ComboString));
						pPC->SetRightWeapon(ConvertStr2Dno2( ComboString ));
						pPC->LoadPCParts();
						pPC->LoadPCMotion();
						pPC->InitialTransform();
						pPC->ConvertMesh();
					}
					break;
				case IDC_COMBO11:
					if( HIWORD(in_wParam) == CBN_SELCHANGE ) {
 						//	左手武器の設定
						GetWindowText(GetDlgItem(in_hWnd, IDC_COMBO11), ComboString, sizeof(ComboString));
						pPC->SetLeftWeapon(ConvertStr2Dno2( ComboString ));
						pPC->LoadPCParts();
						pPC->LoadPCMotion();
						pPC->InitialTransform();
						pPC->ConvertMesh();
					}
					break;
				case IDC_COMBO12:
					if( HIWORD(in_wParam) == CBN_SELCHANGE ) {
 						//	遠隔の設定
						GetWindowText(GetDlgItem(in_hWnd, IDC_COMBO12), ComboString, sizeof(ComboString));
						pPC->SetRemoteWeapon(ConvertStr2Dno2( ComboString ));
						pPC->LoadPCParts();
						pPC->LoadPCMotion();
						pPC->InitialTransform();
						pPC->ConvertMesh();
					}
					break;
				case IDC_COMBO31:
					if( HIWORD(in_wParam) == CBN_SELCHANGE ) {
 						//	NPCデータ設定
						GetWindowText(GetDlgItem(in_hWnd, IDC_COMBO31), ComboString, sizeof(ComboString));
						pNPC->SetBody(ConvertStr2Dno2( ComboString ));
						GetFileNameFromDir2(FileName,ComboString );
						pNPC->LoadNPC();
						pNPC->InitialTransform();
						pNPC->ConvertMesh();
						while (SendMessage(GetDlgItem(in_hWnd, IDC_COMBO9), CB_GETCOUNT, 0, 0) != 0) {
							SendMessage(GetDlgItem(in_hWnd, IDC_COMBO9), CB_DELETESTRING, 0, 0);
						}
						pMotionFrame = (CMotionFrame*)pNPC->m_motions.Top();
						cnt = 0;
						while (pMotionFrame) {
							char mName[6]; strncpy(mName, (char*)pMotionFrame->m_Name, 3); mName[3] = '\0';
							if (SendMessage(GetDlgItem(in_hWnd, IDC_COMBO9), CB_FINDSTRING, 0, (LPARAM)mName) < 0) {
								SendMessage(GetDlgItem(in_hWnd, IDC_COMBO9), CB_ADDSTRING, (WPARAM)0, (LPARAM)mName);
								cnt++;
							}
							pMotionFrame = (CMotionFrame*)pMotionFrame->Next;
						}
						SendMessage(GetDlgItem(in_hWnd, IDC_COMBO9), CB_SETCURSEL, (WPARAM)1, 0L);
						GetWindowText(GetDlgItem(in_hWnd, IDC_COMBO9), ComboString, sizeof(ComboString));
						pNPC->SetMotionName(ComboString);
						pNPC->LoadNPCMotion();
						SetFocus(GetDlgItem(in_hWnd, IDC_COMBO31));
					}
					break;
				case IDC_RADIO1:
					if (IsDlgButtonChecked(in_hWnd, IDC_RADIO1) == BST_CHECKED) {
						g_mPCMotion = 0;
					}
					pPC->LoadPCMotion();
					while (SendMessage(GetDlgItem(in_hWnd, IDC_COMBO8), CB_GETCOUNT, 0, 0) != 0) {
						SendMessage(GetDlgItem(in_hWnd, IDC_COMBO8), CB_DELETESTRING, 0, 0);
					}
					pMotionFrame = (CMotionFrame*)pPC->m_motions.Top();
					cnt = 0;
					while (pMotionFrame) {
						char mName[6]; strncpy(mName, (char*)pMotionFrame->m_Name, 3); mName[3] = '\0';
						if (SendMessage(GetDlgItem(in_hWnd, IDC_COMBO8), CB_FINDSTRING, 0, (LPARAM)mName) < 0) {
							SendMessage(GetDlgItem(in_hWnd, IDC_COMBO8), CB_ADDSTRING, (WPARAM)0, (LPARAM)mName);
							cnt++;
						}
						pMotionFrame = (CMotionFrame*)pMotionFrame->Next;
					}
					SendMessage(GetDlgItem(in_hWnd, IDC_COMBO8), CB_SETCURSEL, (WPARAM)0, 0L);
					GetWindowText(GetDlgItem(in_hWnd, IDC_COMBO8), ComboString, sizeof(ComboString));
					pPC->SetMotionName(ComboString);
					pPC->LoadPCMotion();
					break;
				case IDC_RADIO2:
					if (IsDlgButtonChecked(in_hWnd, IDC_RADIO2) == BST_CHECKED) {
						g_mPCMotion = 1;
					}
					pPC->LoadPCMotion();
					while (SendMessage(GetDlgItem(in_hWnd, IDC_COMBO8), CB_GETCOUNT, 0, 0) != 0) {
						SendMessage(GetDlgItem(in_hWnd, IDC_COMBO8), CB_DELETESTRING, 0, 0);
					}
					pMotionFrame = (CMotionFrame*)pPC->m_motions.Top();
					cnt = 0;
					while (pMotionFrame) {
						char mName[6]; strncpy(mName, (char*)pMotionFrame->m_Name, 3); mName[3] = '\0';
						if (SendMessage(GetDlgItem(in_hWnd, IDC_COMBO8), CB_FINDSTRING, 0, (LPARAM)mName) < 0) {
							SendMessage(GetDlgItem(in_hWnd, IDC_COMBO8), CB_ADDSTRING, (WPARAM)0, (LPARAM)mName);
							cnt++;
						}
						pMotionFrame = (CMotionFrame*)pMotionFrame->Next;
					}
					SendMessage(GetDlgItem(in_hWnd, IDC_COMBO8), CB_SETCURSEL, (WPARAM)0, 0L);
					GetWindowText(GetDlgItem(in_hWnd, IDC_COMBO8), ComboString, sizeof(ComboString));
					pPC->SetMotionName(ComboString);
					pPC->LoadPCMotion();
					break;
				case IDC_RADIO3:
					if (IsDlgButtonChecked(in_hWnd, IDC_RADIO3) == BST_CHECKED) {
						g_mPCMotion = 2;
					}
					pPC->LoadPCMotion();
					while (SendMessage(GetDlgItem(in_hWnd, IDC_COMBO8), CB_GETCOUNT, 0, 0) != 0) {
						SendMessage(GetDlgItem(in_hWnd, IDC_COMBO8), CB_DELETESTRING, 0, 0);
					}
					pMotionFrame = (CMotionFrame*)pPC->m_motions.Top();
					cnt = 0;
					while (pMotionFrame) {
						char mName[6]; strncpy(mName, (char*)pMotionFrame->m_Name, 3); mName[3] = '\0';
						if (SendMessage(GetDlgItem(in_hWnd, IDC_COMBO8), CB_FINDSTRING, 0, (LPARAM)mName) < 0) {
							SendMessage(GetDlgItem(in_hWnd, IDC_COMBO8), CB_ADDSTRING, (WPARAM)0, (LPARAM)mName);
							cnt++;
						}
						pMotionFrame = (CMotionFrame*)pMotionFrame->Next;
					}
					SendMessage(GetDlgItem(in_hWnd, IDC_COMBO8), CB_SETCURSEL, (WPARAM)0, 0L);
					GetWindowText(GetDlgItem(in_hWnd, IDC_COMBO8), ComboString, sizeof(ComboString));
					pPC->SetMotionName(ComboString);
					pPC->LoadPCMotion();
					break;
				case IDC_RADIO4:
					if (IsDlgButtonChecked(in_hWnd, IDC_RADIO4) == BST_CHECKED) {
						g_mPCMotion = 3;
					}
					pPC->LoadPCMotion();
					while (SendMessage(GetDlgItem(in_hWnd, IDC_COMBO8), CB_GETCOUNT, 0, 0) != 0) {
						SendMessage(GetDlgItem(in_hWnd, IDC_COMBO8), CB_DELETESTRING, 0, 0);
					}
					pMotionFrame = (CMotionFrame*)pPC->m_motions.Top();
					cnt = 0;
					while (pMotionFrame) {
						char mName[6]; strncpy(mName, (char*)pMotionFrame->m_Name, 3); mName[3] = '\0';
						if (SendMessage(GetDlgItem(in_hWnd, IDC_COMBO8), CB_FINDSTRING, 0, (LPARAM)mName) < 0) {
							SendMessage(GetDlgItem(in_hWnd, IDC_COMBO8), CB_ADDSTRING, (WPARAM)0, (LPARAM)mName);
							cnt++;
						}
						pMotionFrame = (CMotionFrame*)pMotionFrame->Next;
					}
					SendMessage(GetDlgItem(in_hWnd, IDC_COMBO8), CB_SETCURSEL, (WPARAM)0, 0L);
					GetWindowText(GetDlgItem(in_hWnd, IDC_COMBO8), ComboString, sizeof(ComboString));
					pPC->SetMotionName(ComboString);
					pPC->LoadPCMotion();
					break;
				case IDC_RADIO8:
					if( IsDlgButtonChecked(in_hWnd, IDC_RADIO8) == BST_CHECKED) {
						g_mPCFlag = true;
					}
					break;
				case IDC_RADIO9:
					if( IsDlgButtonChecked(in_hWnd, IDC_RADIO9) == BST_CHECKED) {
						g_mPCFlag = false;
					}
					break;
				case IDC_CHECK1:
					g_mDispIdl = g_mDispIdl?false:true;
					//if( g_mDispIdl ) {
					//	//pPC->SetMotionName("idl");
					//	//pNPC->SetMotionName("idl");
					//	pPC->SetMotionName("idl");
					//	pNPC->SetMotionName("idl");
					//}
					//else {
					//	//pPC->SetMotionName("xxx");
					//	//pNPC->SetMotionName("xxx");
					//	pPC->SetMotionName("wlk");
					//	pNPC->SetMotionName("wlk");
					//}
					pPC->LoadPCMotion();
					pNPC->LoadNPCMotion();
					break;
				case IDC_CHECK2:
					g_mDispWire = g_mDispWire?false:true;
					break;
				case IDC_EDIT1:
					GetWindowText(GetDlgItem(in_hWnd, IDC_EDIT1), ComboString, sizeof(ComboString));
					sscanf(ComboString,"%d",&dum1);
					if( dum1>0 && dum1<128 ) {
						g_mDispBoneNo = dum1;
						if( pPC ) {
							dum2 = pPC->GetBoneParentNo( dum1 );
							sprintf(ComboString,"%d",dum2);
							SetDlgItemText(in_hWnd,IDC_EDIT2,(LPCTSTR)ComboString);
						}
					}
					break;
				case IDC_SPIN1:
					GetWindowText(GetDlgItem(in_hWnd, IDC_EDIT1), ComboString, sizeof(ComboString));
					sscanf(ComboString,"%d",&g_mDispBoneNo);
					if( pPC ) {
						dum2 = pPC->GetBoneParentNo( g_mDispBoneNo );
						sprintf(ComboString,"%d",dum2);
						SetDlgItemText(in_hWnd,IDC_EDIT2,(LPCTSTR)ComboString);
					}
					break;
					GetWindowText(GetDlgItem(in_hWnd, IDC_COMBO10), ComboString, sizeof(ComboString));
				case IDOK:
					ShowWindow(in_hWnd,SW_HIDE);
					break;
			}
			break;
		case WM_CLOSE:
			ShowWindow(in_hWnd,SW_HIDE);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
	}
	return 0;
}


//========================================================================
//
//		メッセージ処理
//
//========================================================================
LRESULT CALLBACK WinProc( HWND hWnd, UINT msg, UINT wParam, LONG lParam )
{
static float	alpha = 0.,beta = 0.;
static float	Delta=0.,Step=0.2f;
	D3DXVECTOR3 Pos,Post;
	D3DXMATRIX	mm,m1,m2;
static bool		lDrag = false,rDrag = false;
static short	x1=-1,y1=-1,x2,y2;
   // ファイルオープンダイアログボックスのテスト
    OPENFILENAME sfn,sfx;
    char szFPath[256],szFName[256],strmsg[256];
	char szFPathx[256], szFNamex[256];

    lstrcpy(szFPath, "*.mqo");
    ZeroMemory(&sfn, sizeof(sfn));
    sfn.lStructSize = sizeof(sfn);
    sfn.hwndOwner = NULL;
    sfn.lpstrFile = szFPath;
    sfn.nMaxFile = sizeof(szFPath);
    sfn.lpstrFilter = "MQO Format(*.mqo)\0*.mqo\0";
    sfn.nFilterIndex = 1;
    sfn.lpstrFileTitle = szFName;
    sfn.nMaxFileTitle = sizeof(szFName);
	sfn.lpstrTitle = "MQOセーブ";
    sfn.lpstrInitialDir = NULL;
 
	lstrcpy(szFPathx, "*.x");
	ZeroMemory(&sfx, sizeof(sfx));
	sfx.lStructSize = sizeof(sfx);
	sfx.hwndOwner = NULL;
	sfx.lpstrFile = szFPathx;
	sfx.nMaxFile = sizeof(szFPathx);
	sfx.lpstrFilter = "x Format(*.x)\0*.x\0";
	sfx.nFilterIndex = 1;
	sfx.lpstrFileTitle = szFNamex;
	sfx.nMaxFileTitle = sizeof(szFNamex);
	sfx.lpstrTitle = "Xファイル　セーブ";
	sfx.lpstrInitialDir = NULL;
	switch (msg)
	{
		//==============================================
		//	終了時
		//==============================================
		case WM_DESTROY:
			PostQuitMessage( 0 );
			break;
		case WM_MOUSEWHEEL:
			if( wParam==0x00780000 ) {
				if (g_mEyeScale <= 0.1f) break;
				if (GetKeyState(VK_CONTROL) & 0x8000){
					g_mEyeScale -= 0.125f * 10.f;
				} else {
					g_mEyeScale -= 0.125f;
				}
				D3DXMATRIX matY,matX,matS;
				D3DXMatrixRotationY(&matY,g_mEyeAlph);
				D3DXMatrixRotationX(&matX,g_mEyeBeta);
				D3DXMatrixScaling(&matS,g_mEyeScale,g_mEyeScale,g_mEyeScale);
				g_mEyeMat = matS * matX * matY;
				D3DXVec3TransformNormal(&g_mEye,&g_mEyebase,&g_mEyeMat);
				g_mEye += g_mAt;
				D3DXMatrixLookAtLH( &g_mView, &g_mEye, &g_mAt, &g_mUp );
			} else if( wParam == 0xff880000 ) {
				if( GetKeyState(VK_CONTROL)&0x8000 ){
					g_mEyeScale += 0.125f * 10.f;
				} else {
					g_mEyeScale += 0.125f;
				}
				D3DXMATRIX matY,matX,matS;
				D3DXMatrixRotationY(&matY,g_mEyeAlph);
				D3DXMatrixRotationX(&matX,g_mEyeBeta);
				D3DXMatrixScaling(&matS,g_mEyeScale,g_mEyeScale,g_mEyeScale);
				g_mEyeMat = matS * matX * matY;
				D3DXVec3TransformNormal(&g_mEye,&g_mEyebase,&g_mEyeMat);
				g_mEye += g_mAt;
				D3DXMatrixLookAtLH( &g_mView, &g_mEye, &g_mAt, &g_mUp );
			} 
			break;

 		case WM_MOUSEMOVE:
			x2 = LOWORD(lParam);
			y2 = HIWORD(lParam);
			if( wParam & MK_LBUTTON ) {
				if( abs(x2-x1)<20 && abs(y2-y1) <20 ) {
					g_mEyeAlph += (float)(x2-x1)/(float)g_mScreenWidth*2.f*PAI;
					g_mEyeBeta += (float)(y1-y2)/(float)g_mScreenWidth*2.f*PAI;
					g_mEyeAlph = (g_mEyeAlph>PAI2)?(g_mEyeAlph-PAI2):g_mEyeAlph;
					g_mEyeAlph = (g_mEyeAlph<-PAI2)?(g_mEyeAlph+PAI2):g_mEyeAlph;
					g_mEyeBeta = (g_mEyeBeta>(PAI/2.f))?(PAI/2.f-0.02f):g_mEyeBeta;
					g_mEyeBeta = (g_mEyeBeta<(-PAI/2.f))?(-PAI/2.f+0.02f):g_mEyeBeta;
					D3DXMATRIX matY,matX,matS;
					D3DXMatrixRotationY(&matY,g_mEyeAlph);
					D3DXMatrixRotationX(&matX,g_mEyeBeta);
					D3DXMatrixScaling(&matS,g_mEyeScale,g_mEyeScale,g_mEyeScale);
					g_mEyeMat = matS * matX * matY;
					D3DXVec3TransformNormal(&g_mEye,&g_mEyebase,&g_mEyeMat);
					g_mEye += g_mAt;
					D3DXMatrixLookAtLH( &g_mView, &g_mEye, &g_mAt, &g_mUp );
				}
			} else if( wParam & MK_MBUTTON ) {
				if( abs( y2-y1) <20 ) {
					g_mAt.y += (y2-y1)/100.f;
					D3DXVec3TransformNormal(&g_mEye,&g_mEyebase,&g_mEyeMat);
					g_mEye += g_mAt;
					D3DXMatrixLookAtLH( &g_mView, &g_mEye, &g_mAt, &g_mUp );
				}
			} else if( wParam & MK_RBUTTON ) {
				if( abs(x2-x1)<20 && abs(y2-y1) <20 ) {
					g_mLightAlph += (float)(x2-x1)/(float)g_mScreenWidth*2.f*PAI;
					g_mLightBeta += (float)(y1-y2)/(float)g_mScreenWidth*2.f*PAI;
					g_mLightAlph = (g_mLightAlph>PAI2)?(g_mLightAlph-PAI2):g_mLightAlph;
					g_mLightAlph = (g_mLightAlph<-PAI2)?(g_mLightAlph+PAI2):g_mLightAlph;
					g_mLightBeta = (g_mLightBeta>(PAI/2.f))?(PAI/2.f-0.02f):g_mLightBeta;
					g_mLightBeta = (g_mLightBeta<(-PAI/2.f))?(-PAI/2.f+0.02f):g_mLightBeta;
					D3DXMATRIX mat,matY,matX;
					D3DXMatrixRotationY(&matY,g_mLightAlph);
					D3DXMatrixRotationX(&matX,g_mLightBeta);
					mat = matX * matY;
					D3DXVec3TransformNormal((D3DXVECTOR3*)&g_mLight.Direction,(D3DXVECTOR3*)&g_mLightbase.Direction,&mat);
					D3DXVec3Normalize( (D3DXVECTOR3*)&g_mLight.Direction, (D3DXVECTOR3*)&g_mLight.Direction );
					GetDevice()->SetLight( 0, &g_mLight );
				}
			}
			x1 = x2; y1 = y2;
			break;
		case WM_COMMAND:
			if (LOWORD(wParam) == ID_MNU_SAVE) {
				if (g_mPCFlag) {
					if (GetSaveFileName(&sfn)) {
						if (!pPC->saveMQO(szFPath, szFName)) {
							wsprintf(strmsg, "ファイル %s　は正しく処理できませんでした", szFPath);
							MessageBox(NULL, strmsg, "セーブファイルオープン", MB_OK | MB_ICONINFORMATION);
						}
					}
				}
				else {
					wsprintf(strmsg, "NPCは未サポートです");
					MessageBox(NULL, strmsg, "ＷＡＲＮＮＩＮＧ", MB_OK | MB_ICONINFORMATION);
				}
			}
			else if (LOWORD(wParam) == ID_MNU_SAVEX) {
				if (g_mPCFlag) {
					if (GetSaveFileName(&sfx)) {
						if (!pPC->saveX(szFPathx, szFNamex)) {
							wsprintf(strmsg, "ファイル %s　は正しく処理できませんでした", szFPath);
							MessageBox(NULL, strmsg, "セーブファイルオープン", MB_OK | MB_ICONINFORMATION);
						}
					}
				}
				else {
					if (GetSaveFileName(&sfx)) {
						if (!pNPC->saveX(szFPathx, szFNamex)) {
							wsprintf(strmsg, "ファイル %s　は正しく処理できませんでした", szFPath);
							MessageBox(NULL, strmsg, "セーブファイルオープン", MB_OK | MB_ICONINFORMATION);
						}
					}
				}
			} else if( LOWORD(wParam) == ID_MNU_INVENT ) {
				ShowWindow( hDlg2,SW_SHOW );
			} else if(LOWORD(wParam) == ID_MNU_EXIT) {
				if( MessageBox(NULL, "本当に終了しますか？", "プログラム終了", MB_YESNO | MB_ICONQUESTION ) == IDYES ) {
					SendMessage(hWnd, WM_CLOSE, 0L, 0L);
				}
			}
		default:
			return DefWindowProc( hWnd, msg, wParam, lParam );
	}

	return 0;
}

