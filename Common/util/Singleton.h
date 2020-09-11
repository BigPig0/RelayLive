// µ¥ÀýÄ£°å
#pragma once

#include "mutex.h"

template<class T>
class Singleton
{
public:
    static T* GetInstance();
    static void ReleaseInstance();
private:
    static T* m_pInstance;
    static CriticalSection m_cs;
};

template<class T>
T* Singleton<T>::m_pInstance = nullptr;

template<class T>
CriticalSection Singleton<T>::m_cs;

template<class T>
T* Singleton<T>::GetInstance()
{
    m_cs.lock();
    if(nullptr == m_pInstance)
    {
        m_pInstance = new T();
    }
    m_cs.unlock();
    return m_pInstance;
}

template<class T>
void Singleton<T>::ReleaseInstance()
{
    m_cs.lock();
    if (nullptr != m_pInstance)
    {
        delete m_pInstance;
        m_pInstance = nullptr;
    }
    m_cs.unlock();
}

