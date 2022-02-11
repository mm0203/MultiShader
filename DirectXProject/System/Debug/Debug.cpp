#include <System/Debug/Debug.h>
#include <System/Debug/Geometory.h>
#include <System/Debug/Manipulator.h>
#include <System/Debug/Menu.h>

void InitDebug()
{
	Geometory::CreateInstance();
	Manipulator::CreateInstance();
	Menu::CreateInstance();
}
void UninitDebug()
{
	Menu::DestroyInstance();
	Manipulator::DestroyInstance();
	Geometory::DestroyInstance();
}
void UpdateDebug()
{
	MANIPULATOR->Update();
	MENU->Update();
}
void DrawDebug()
{
	MANIPULATOR->Draw();
	MENU->Draw();
}