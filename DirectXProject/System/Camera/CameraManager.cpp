#include <System/Camera/CameraManager.hpp>
#include <System/Camera/CameraDebug.h>
#include <System/Camera/Camera2D.h>

void CameraManager::Init()
{
	m_mainCamera = Create<CameraDebug>();
	m_list.front()->Enable(true);
	Create<Camera2D>();
}
void CameraManager::Uninit()
{
	for (Camera* pCamera : m_list)
	{
		delete pCamera;
	}
}

void CameraManager::Update()
{
	for (Camera* pCamera : m_list)
	{
		if (pCamera->IsEnable())
		{
			pCamera->Execute();
		}
	}
}
void CameraManager::Draw()
{
	for (Camera* pCamera : m_list)
	{
		pCamera->Draw();
	}
}
void CameraManager::Switch(int no)
{
	// 古いカメラを無効
	CameraList::iterator oldIt = m_list.begin();
	while (m_mainCamera--)
	{
		++oldIt;
	}
	(*oldIt)->Enable(false);
	m_mainCamera = no;
	// 新しいカメラを有効
	CameraList::iterator it = m_list.begin();
	while (no--)
	{
		++it;
	}
	(*it)->Enable(true);
	(*it)->OnActive(**oldIt);
}
Camera* CameraManager::Get(int no)
{
	// 現在有効なカメラ番号を設定
	if (no <= -1)
	{
		no = m_mainCamera;
	}

	// 指定された番号が範囲外
	if (m_list.size() <= no)
	{
		return nullptr;
	}

	// カメラ取得
	CameraList::iterator it = m_list.begin();
	while (no--)
	{
		++it;
	}
	return *it;
}