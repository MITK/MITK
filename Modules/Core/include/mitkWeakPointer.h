/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef mitkWeakPointer_h
#define mitkWeakPointer_h

#include <itkCommand.h>
#include <functional>

namespace mitk
{
  template <class T>
  class WeakPointer final
  {
  public:
    WeakPointer() noexcept
      : m_RawPointer(nullptr)
    {
    }

    WeakPointer(T *rawPointer)
      : m_RawPointer(rawPointer)
    {
      this->AddDeleteEventObserver();
    }

    WeakPointer(const WeakPointer &other)
      : m_RawPointer(other.m_RawPointer)
    {
      this->AddDeleteEventObserver();
    }

    // Because the observed object is keeping references to its observers,
    // move semantics can't be implemented more effective than its alternative
    // to simply copy and destruct. It would still be necessary to remove the
    // old observer and to add the new one.
    WeakPointer(WeakPointer &&) = delete;
    WeakPointer & operator =(WeakPointer &&) = delete;

    ~WeakPointer() noexcept
    {
      try
      {
        this->RemoveDeleteEventObserver();
      }
      catch (...)
      {
        // Swallow. Otherwise, the application would terminate if another
        // exception is already propagating.
      }
    }

    // Prefer classic implementation to copy-and-swap idiom. Swapping is
    // non-trivial for this class as the observed object is keeping references
    // to its observers.
    WeakPointer & operator =(const WeakPointer &other)
    {
      if (this != &other)
      {
        this->RemoveDeleteEventObserver();

        // m_RawPointer is guaranteed not to be nullptr as nullptr assignment
        // is handled in a separate implementation of the assignment operator.
        m_RawPointer = other.m_RawPointer;
        this->AddDeleteEventObserver();
      }

      return *this;
    }

    WeakPointer & operator =(std::nullptr_t)
    {
      this->RemoveDeleteEventObserver();
      m_RawPointer = nullptr;

      return *this;
    }

    explicit operator bool() const noexcept
    {
      return nullptr != m_RawPointer;
    }

    bool IsExpired() const noexcept
    {
      return !*this;
    }

    typename T::Pointer Lock() const noexcept
    {
      return m_RawPointer;
    }

  private:
    void AddDeleteEventObserver()
    {
      if (nullptr != m_RawPointer)
      {
        auto command = itk::SimpleMemberCommand<WeakPointer>::New();
        command->SetCallbackFunction(this, &WeakPointer::OnDeleteEvent);
        m_ObserverTag = m_RawPointer->AddObserver(itk::DeleteEvent(), command);
      }
    }

    void RemoveDeleteEventObserver()
    {
      if (nullptr != m_RawPointer)
        m_RawPointer->RemoveObserver(m_ObserverTag);
    }

    void OnDeleteEvent() noexcept
    {
      m_RawPointer = nullptr;

      // Don't remove any observers from the observed object as it is about to
      // die and can't handle this operation anymore.
    }

    // The following comparison operators need access to class internals.
    // All remaining comparison operators are implemented as non-member
    // non-friend functions that use logical combinations of these non-member
    // friend functions.

    friend bool operator ==(const WeakPointer &left, const WeakPointer &right) noexcept
    {
      return left.m_RawPointer == right.m_RawPointer;
    }

    // Also covers comparisons to T::Pointer and T::ConstPointer as
    // itk::SmartPointer can be implicitly converted to a raw pointer.
    friend bool operator ==(const WeakPointer &left, const T *right) noexcept
    {
      return left.m_RawPointer == right;
    }

    friend bool operator <(const WeakPointer &left, const WeakPointer &right) noexcept
    {
      // The specialization of std::less for any pointer type yields a total
      // order, even if the built-in operator < doesn't.
      return std::less<T*>()(left.m_RawPointer, right.m_RawPointer);
    }

    friend bool operator <(const WeakPointer &left, std::nullptr_t right) noexcept
    {
      return std::less<T*>()(left.m_RawPointer, right);
    }

    friend bool operator <(std::nullptr_t left, const WeakPointer &right) noexcept
    {
      return std::less<T*>()(left, right.m_RawPointer);
    }

    friend bool operator <(const WeakPointer &left, const T *right) noexcept
    {
      return std::less<T*>()(left.m_RawPointer, right);
    }

    friend bool operator <(const T *left, const WeakPointer &right) noexcept
    {
      return std::less<T*>()(left, right.m_RawPointer);
    }

    T *m_RawPointer;

    // m_ObserverTag is completely managed by the two methods
    // AddDeleteEventObserver() and RemoveDeleteEventObserver(). There
    // isn't any need to initialize or use it at all outside of these methods.
    unsigned long m_ObserverTag;
  };
}

template <class T>
bool operator !=(const mitk::WeakPointer<T> &left, const mitk::WeakPointer<T> &right) noexcept
{
  return !(left == right);
}

template <class T>
bool operator <=(const mitk::WeakPointer<T> &left, const mitk::WeakPointer<T> &right) noexcept
{
  return !(right < left);
}

template <class T>
bool operator >(const mitk::WeakPointer<T> &left, const mitk::WeakPointer<T> &right) noexcept
{
  return right < left;
}

template <class T>
bool operator >=(const mitk::WeakPointer<T> &left, const mitk::WeakPointer<T> &right) noexcept
{
  return !(left < right);
}

template <class T>
bool operator ==(const mitk::WeakPointer<T> &left, std::nullptr_t) noexcept
{
  return !left;
}

template <class T>
bool operator !=(const mitk::WeakPointer<T> &left, std::nullptr_t right) noexcept
{
  return !(left == right);
}

template <class T>
bool operator ==(std::nullptr_t, const mitk::WeakPointer<T> &right) noexcept
{
  return !right;
}

template <class T>
bool operator !=(std::nullptr_t left, const mitk::WeakPointer<T> &right) noexcept
{
  return !(left == right);
}

template <class T>
bool operator <=(const mitk::WeakPointer<T> &left, std::nullptr_t right) noexcept
{
  return !(right < left);
}

template <class T>
bool operator >(const mitk::WeakPointer<T> &left, std::nullptr_t right) noexcept
{
  return right < left;
}

template <class T>
bool operator >=(const mitk::WeakPointer<T> &left, std::nullptr_t right) noexcept
{
  return !(left < right);
}

template <class T>
bool operator <=(std::nullptr_t left, const mitk::WeakPointer<T> &right) noexcept
{
  return !(right < left);
}

template <class T>
bool operator >(std::nullptr_t left, const mitk::WeakPointer<T> &right) noexcept
{
  return right < left;
}

template <class T>
bool operator >=(std::nullptr_t left, const mitk::WeakPointer<T> &right) noexcept
{
  return !(left < right);
}

template <class T>
bool operator !=(const mitk::WeakPointer<T> &left, const T *right) noexcept
{
  return !(left == right);
}

template <class T>
bool operator <=(const mitk::WeakPointer<T> &left, const T *right) noexcept
{
  return !(right < left);
}

template <class T>
bool operator >(const mitk::WeakPointer<T> &left, const T *right) noexcept
{
  return right < left;
}

template <class T>
bool operator >=(const mitk::WeakPointer<T> &left, const T *right) noexcept
{
  return !(left < right);
}

template <class T>
bool operator ==(const T *left, const mitk::WeakPointer<T> &right) noexcept
{
  return right == left;
}

template <class T>
bool operator !=(const T *left, const mitk::WeakPointer<T> &right) noexcept
{
  return !(right == left);
}

template <class T>
bool operator <=(const T *left, const mitk::WeakPointer<T> &right) noexcept
{
  return !(right < left);
}

template <class T>
bool operator >(const T *left, const mitk::WeakPointer<T> &right) noexcept
{
  return right < left;
}

template <class T>
bool operator >=(const T *left, const mitk::WeakPointer<T> &right) noexcept
{
  return !(left < right);
}

#endif
