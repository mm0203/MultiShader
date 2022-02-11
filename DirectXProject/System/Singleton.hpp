#ifndef __SINGLETON_HPP__
#define __SINGLETON_HPP__

#define SINGLETON_CONSTRACTOR(_class) \
private:\
	_class(){}\
	~_class(){}\
	friend Singleton<_class>

template <class T>
class Singleton
{
protected:
	Singleton() {}
	virtual ~Singleton() {}
	virtual void Init() {}
	virtual void Uninit() {}
public:
	static T* GetInstance()
	{
		return m_pInstance;
	}
	static void CreateInstance()
	{
		DestroyInstance();
		m_pInstance = new T;
		m_pInstance->Init();
	}
	static void DestroyInstance()
	{
		if(m_pInstance)
		{
			m_pInstance->Uninit();
			delete m_pInstance;
			m_pInstance = nullptr;
		}
	}
private:
	static T* m_pInstance;
};

template<class T> T* Singleton<T>::m_pInstance = nullptr;

#endif // __SINGLETON_HPP__