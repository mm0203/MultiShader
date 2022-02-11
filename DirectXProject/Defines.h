#ifndef __DEFINES_H__
#define __DEFINES_H__

#include <assert.h>
#include <Windows.h>

#define APP_TITLE "Title"

// 画面サイズ
#define SCREEN_WIDTH (1280)
#define SCREEN_HEIGHT (720)

// リソースパス
#define ASSET(path)	"Assets/"path

// 個別設定
#define USER_MYNOTE 1
#define USER_DESKTOP 0
#define USER_WORKPC 0

#if USER_MYNOTE0
#define USE_DIRECTXTEX 1
#define USE_FBXSDK 1
#else
#define USE_DIRECTXTEX 1\0
#define USE_FBXSDK 0
#endif
#define USE_EFFEKSEER 0



#endif // __DEFINES_H__