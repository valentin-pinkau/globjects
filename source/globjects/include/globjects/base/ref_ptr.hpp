#pragma once

#include <globjects/base/ref_ptr.h>

#include <globjects/base/Referenced.h>

namespace globjects {

template<typename T>
ref_ptr<T>::ref_ptr()
: m_referenced(nullptr)
{
}

template<typename T>
ref_ptr<T>::ref_ptr(T * referenced)
: m_referenced(referenced)
{
	increaseRef();
}

template<typename T>
ref_ptr<T>::ref_ptr(const ref_ptr & reference)
: m_referenced(reference.m_referenced)
{
	increaseRef();
}

template<typename T>
ref_ptr<T>::ref_ptr(ref_ptr && reference)
: m_referenced(reference.m_referenced)
{
    reference.m_referenced = nullptr;
}

template<typename T>
ref_ptr<T>::~ref_ptr()
{
	decreaseRef();
}

template<typename T>
ref_ptr<T> & ref_ptr<T>::operator=(const ref_ptr & reference)
{
	decreaseRef();
	m_referenced = reference.m_referenced;
	increaseRef();

	return *this;
}

template<typename T>
T & ref_ptr<T>::operator*() const
{
    return *m_referenced;
}

template<typename T>
T * ref_ptr<T>::operator->() const
{
    return m_referenced;
}

template<typename T>
T * ref_ptr<T>::get() const
{
    return m_referenced;
}

template<typename T>
ref_ptr<T>::operator T *() const
{
    return m_referenced;
}

template<typename T>
bool ref_ptr<T>::operator<(const ref_ptr & reference) const
{
    return m_referenced < reference.m_referenced;
}

template<typename T>
bool ref_ptr<T>::operator>(const ref_ptr & reference) const
{
    return m_referenced > reference.m_referenced;
}

template<typename T>
bool ref_ptr<T>::operator==(const ref_ptr & reference) const
{
    return m_referenced == reference.m_referenced;
}

template<typename T>
bool ref_ptr<T>::operator!=(const ref_ptr & reference) const
{
    return m_referenced != reference.m_referenced;
}

template<typename T>
bool ref_ptr<T>::operator<(T * pointer) const
{
    return m_referenced < pointer;
}

template<typename T>
bool ref_ptr<T>::operator>(T * pointer) const
{
    return m_referenced > pointer;
}

template<typename T>
bool ref_ptr<T>::operator==(T * pointer) const
{
    return m_referenced == pointer;
}

template<typename T>
bool ref_ptr<T>::operator!=(T * pointer) const
{
    return m_referenced != pointer;
}

template<typename T>
void ref_ptr<T>::increaseRef()
{
	if (m_referenced)
	{
		m_referenced->ref();
	}
}

template<typename T>
void ref_ptr<T>::decreaseRef()
{
	if (m_referenced)
	{
		m_referenced->unref();
	}
}

template <typename T>
ref_ptr<T> make_ref(T * object)
{
    return ref_ptr<T>(object);
}

} // namespace globjects
