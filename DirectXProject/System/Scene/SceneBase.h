#ifndef __SCENE_BASE_H__
#define __SCENE_BASE_H__

#include <Windows.h>
#include <string>

#define SCENE_DECLARATION(scene) \
public:\
	scene() : Scene(#scene){}\
	virtual ~scene(){}\
	HRESULT Load();\
	void Release();\
	void Update(float tick);\
	void Draw() // ; ‚ ‚¦‚Ä‚Â‚¯‚Ä‚È‚¢

class Scene
{
public:
	Scene(const char *sceneName);
	virtual ~Scene();

	virtual HRESULT Load() = 0;
	virtual void Release() = 0;
	virtual void Update(float tick) = 0;
	virtual void Draw() = 0;

	const std::string& GetName();

private:
	std::string m_name;
};

#endif // __SCENE_BASE_H__