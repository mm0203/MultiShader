#include <System/Scene/SceneManager.hpp>
#include <System/Scene/SampleScene.h>
#include <System/Macro.h>
#include <algorithm>

void SceneManager::Init()
{
	Load<SampleScene>();
}
void SceneManager::Uninit()
{
	Unload();
}

void SceneManager::Update(float tick)
{
	for (Scene* pScene : m_scenes)
	{
		pScene->Update(tick);
	}
}
void SceneManager::Draw()
{
	for (Scene* pScene : m_scenes)
	{
		pScene->Draw();
	}
}

void SceneManager::Unload()
{
	for (Scene* pScene : m_scenes)
	{
		pScene->Release();
		SAFE_DELETE(pScene);
	}
	m_scenes.clear();
}
void SceneManager::Unload(UINT index)
{
	Scenes::iterator it = FindScene(index);
	if (it != m_scenes.end())
	{
		(*it)->Release();
		m_scenes.erase(it);
	}
}
void SceneManager::Unload(std::string name)
{
	Scenes::iterator it = FindScene(name);
	if (it != m_scenes.end())
	{
		(*it)->Release();
		m_scenes.erase(it);
	}
}
SceneManager::Scenes::iterator SceneManager::FindScene(UINT index)
{
	Scenes::iterator it = m_scenes.begin();
	while (index--)
	{
		++it;
	}
	return it;
}
SceneManager::Scenes::iterator SceneManager::FindScene(std::string name)
{
	return std::find_if(m_scenes.begin(), m_scenes.end(),
		[&name](Scene* pScene)
		{
			return pScene->GetName() == name;
		});
}