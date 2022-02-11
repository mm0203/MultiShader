#ifndef __INPUT_H__
#define __INPUT_H__

#include <Windows.h>

HRESULT InitInput();
void UninitInput();
void UpdateInput();

void InstantForceInput(int nKey, bool bPress);

bool IsKeyPress(int nKey);
bool IsKeyTrigger(int nKey);
bool IsKeyRelease(int nKey);
bool IsKeyRepeat(int nKey);

bool IsMouseDoubleClick(int nKey);
const POINT& GetMousePos();
int GetMouseWheel();

#endif // __INPUT_H__