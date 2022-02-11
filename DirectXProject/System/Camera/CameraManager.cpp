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
	// �Â��J�����𖳌�
	CameraList::iterator oldIt = m_list.begin();
	while (m_mainCamera--)
	{
		++oldIt;
	}
	(*oldIt)->Enable(false);
	m_mainCamera = no;
	// �V�����J������L��
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
	// ���ݗL���ȃJ�����ԍ���ݒ�
	if (no <= -1)
	{
		no = m_mainCamera;
	}

	// �w�肳�ꂽ�ԍ����͈͊O
	if (m_list.size() <= no)
	{
		return nullptr;
	}

	// �J�����擾
	CameraList::iterator it = m_list.begin();
	while (no--)
	{
		++it;
	}
	return *it;
}