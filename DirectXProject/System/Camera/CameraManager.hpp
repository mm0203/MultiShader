#ifndef __CAMERA_MANAGER_HPP__
#define __CAMERA_MANAGER_HPP__

#include <System/Camera/Camera.h>
#include <System/Singleton.hpp>
#include <list>
#include <algorithm>
#include <typeinfo>

#define CAMERA (CameraManager::GetInstance())

class CameraManager : public Singleton<CameraManager>
{
	SINGLETON_CONSTRACTOR(CameraManager);
private:
	using CameraList = std::list<Camera*>;

public:	
	void Init() final;
	void Uninit() final;
	void Update();
	void Draw();
	void Switch(int no);
	template<class T> int Create();
	Camera* Get(int no = -1);
	template<class T> T* Get();
	
private:
	CameraList m_list;
	int m_mainCamera;
};

template<class T> int CameraManager::Create()
{
	// テンプレートのデータ型チェック
	static_assert(std::is_base_of<Camera, T>::value,
		"[CameraManager::Create<T>] template T not inherit from 'Camera'");
	
	m_list.push_back(new T);
	m_list.back()->Execute();
	return static_cast<int>(m_list.size() - 1);
}
template<class T> T* CameraManager::Get()
{
	// 実行時型チェック
	const std::type_info& type = typeid(T);
	CameraList::iterator it = std::find_if(m_list.begin(), m_list.end(),
		[&type](Camera*& obj)
		{
			return typeid(*obj) == type;
		});
	return reinterpret_cast<T*>(*it);
}

#endif