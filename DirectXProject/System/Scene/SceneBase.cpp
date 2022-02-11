#include "SceneBase.h"

Scene::Scene(const char* sceneName)
	: m_name(sceneName)
{
}
Scene::~Scene()
{
}

const std::string& Scene::GetName()
{
	return m_name;
}
