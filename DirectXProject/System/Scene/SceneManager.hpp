#ifndef __SCENE_MANAGER_HPP__
#define __SCENE_MANAGER_HPP__

#include <System/Scene/SceneBase.h>
#include <System/Singleton.hpp>
#include <list>

#define SCENE SceneManager::GetInstance()

class SceneManager : public Singleton<SceneManager>
{
	SINGLETON_CONSTRACTOR(SceneManager);
private:
	using Scenes = std::list<Scene*>;
public:
	void Init() final;
	void Uninit() final;
	void Update(float tick);
	void Draw();

	template<class T> HRESULT Load();
	template<class T> HRESULT LoadSub();
	void Unload();
	void Unload(UINT index);
	void Unload(std::string name);

	template<class T> T* GetScene(UINT index);
	template<class T> T* GetScene(std::string name);

private:
	Scenes::iterator FindScene(UINT index);
	Scenes::iterator FindScene(std::string name);

private:
	static SceneManager* m_pInstance;
	Scenes m_scenes;
};

template<class T>
HRESULT SceneManager::Load()
{
	// テンプレートのデータ型チェック
	static_assert(std::is_base_of<Scene, T>::value,
		"[SceneManager::Load<T>] template T not inherit from 'Scene'");

	Unload();
	m_scenes.push_back(new T);
	return m_scenes.front()->Load();
}
template<class T> HRESULT SceneManager::LoadSub()
{
	// テンプレートのデータ型チェック
	static_assert(std::is_base_of<Scene, T>::value,
		"[SceneManager::SubLoad<T>] template T not inherit from 'Scene'");

	// シーンがロードされてない
	if (m_scenes.empty())
	{
		return E_FAIL;
	}

	// 多重読み込みチェック
	Scene* pScene = new T;
	if (FindScene(pScene->GetName()) != m_scenes.end())
	{
		delete pScene;
		return E_FAIL;
	}

	// 登録
	m_scenes.push_back(pScene);
	return pScene->Load();
}
template<class T> T* SceneManager::GetScene(UINT index)
{
	Scenes::iterator it = FindScene(index);
	return it == m_scenes.end() ? nullptr : reinterpret_cast<T*>(*it);
}
template<class T> T* SceneManager::GetScene(std::string name)
{
	Scenes::iterator it = FindScene(name);
	return it == m_scenes.end() ? nullptr : reinterpret_cast<T*>(*it);
}


#endif