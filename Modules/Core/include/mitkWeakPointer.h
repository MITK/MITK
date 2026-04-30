/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkWeakPointer_h
#define mitkWeakPointer_h

#include <itkCommand.h>
#include <compare>
#include <functional>

namespace mitk
{
  /**
   * \brief A weak pointer that observes an itk::Object without preventing its deletion.
   *
   * WeakPointer holds a non-owning reference to an itk::Object-derived instance.
   * It automatically detects when the observed object is deleted by registering
   * an itk::DeleteEvent observer. After the object is deleted, the weak pointer
   * expires (IsExpired() returns true and Lock() returns nullptr).
   *
   * An optional delete-event callback can be registered via SetDeleteEventCallback()
   * to receive notification when the observed object is destroyed.
   *
   * Unlike itk::WeakPointer, this implementation supports const pointers.
   *
   * All comparison operators (==, !=, <, >, <=, >=) are supported via C++20
   * operator== and operator<=> generation.
   *
   * \tparam T  The type of the observed object. Must derive from itk::Object.
   *
   * \ingroup Core
   */
  template <class T>
  class WeakPointer final
  {
  public:
    /** \brief Callback type invoked when the observed object is deleted. */
    using DeleteEventCallbackType = std::function<void ()>;

    /** \brief Default constructor. Creates an expired weak pointer (null). */
    WeakPointer() noexcept
      : m_RawPointer(nullptr)
    {
    }

    /**
     * \brief Construct a weak pointer observing the given raw pointer.
     *
     * \param[in] rawPointer  The object to observe. May be nullptr.
     */
    WeakPointer(T *rawPointer)
      : m_RawPointer(rawPointer)
    {
      this->AddDeleteEventObserver();
    }

    /**
     * \brief Copy constructor. Observes the same object as \p other.
     *
     * \param[in] other  The weak pointer to copy from.
     */
    WeakPointer(const WeakPointer &other)
      : m_RawPointer(other.m_RawPointer)
    {
      this->AddDeleteEventObserver();
    }

    /**
     * \brief Move constructor. Takes over the observed object from \p other.
     *
     * After the move, \p other is in an expired state (null).
     *
     * \param[in] other  The weak pointer to move from.
     */
    WeakPointer(WeakPointer &&other)
      : m_RawPointer(other.m_RawPointer)
    {
      other.RemoveDeleteEventObserver();
      other.m_RawPointer = nullptr;
      this->AddDeleteEventObserver();
    }

    /** \brief Destructor. Removes the delete-event observer from the observed object. */
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

    /**
     * \brief Copy assignment operator.
     *
     * Uses classic implementation rather than copy-and-swap, because swapping
     * is non-trivial when the observed object keeps references to its observers.
     *
     * \param[in] other  The weak pointer to copy from.
     * \return Reference to this weak pointer.
     */
    WeakPointer & operator =(const WeakPointer &other)
    {
      if (this != &other)
      {
        this->RemoveDeleteEventObserver();
        m_RawPointer = other.m_RawPointer;
        this->AddDeleteEventObserver();
      }

      return *this;
    }

    /**
     * \brief Move assignment operator.
     *
     * After the move, \p other is in an expired state (null).
     *
     * \param[in] other  The weak pointer to move from.
     * \return Reference to this weak pointer.
     */
    WeakPointer & operator =(WeakPointer &&other)
    {
      // No check for self-assignment as it is allowed to assume that the
      // parameter is a unique reference to this argument.

      this->RemoveDeleteEventObserver();
      m_RawPointer = other.m_RawPointer;
      other.m_RawPointer = nullptr;
      this->AddDeleteEventObserver();

      return *this;
    }

    /**
     * \brief Assign nullptr to reset this weak pointer to an expired state.
     *
     * \return Reference to this weak pointer.
     */
    WeakPointer & operator =(std::nullptr_t)
    {
      this->RemoveDeleteEventObserver();
      m_RawPointer = nullptr;

      return *this;
    }

    /**
     * \brief Assign a raw pointer to observe.
     *
     * \param[in] other  The raw pointer to observe. May be nullptr.
     * \return Reference to this weak pointer.
     */
    WeakPointer & operator =(T *other)
    {
      if (m_RawPointer != other)
      {
        this->RemoveDeleteEventObserver();
        m_RawPointer = other;
        this->AddDeleteEventObserver();
      }

      return *this;
    }

    /**
     * \brief Bool conversion operator.
     *
     * \return True if the observed object is still alive, false if expired.
     */
    explicit operator bool() const noexcept
    {
      return nullptr != m_RawPointer;
    }

    /**
     * \brief Check whether the observed object has been deleted.
     *
     * \return True if the weak pointer is expired (null), false otherwise.
     */
    bool IsExpired() const noexcept
    {
      return !*this;
    }

    /**
     * \brief Obtain a smart pointer to the observed object.
     *
     * If the observed object is still alive, a valid itk::SmartPointer is
     * returned. If it has been deleted, a null SmartPointer is returned.
     *
     * \return An itk::SmartPointer to the observed object, or nullptr.
     */
    itk::SmartPointer<T> Lock() const
    {
      return m_RawPointer;
    }

    /**
     * \brief Set a callback to be invoked when the observed object is deleted.
     *
     * \param[in] callback  The callback function. May be empty to clear.
     */
    void SetDeleteEventCallback(const DeleteEventCallbackType &callback)
    {
      m_DeleteEventCallback = callback;
    }

  private:
    void AddDeleteEventObserver()
    {
      if (nullptr != m_RawPointer)
      {
        auto command = itk::SimpleMemberCommand<WeakPointer>::New();
        command->SetCallbackFunction(this, &WeakPointer::OnDeleteEvent);

        using TWithoutConst = typename std::remove_const_t<T>;
        //cast the pointer to non const before adding the observer. This is done to ensure that
        //weak pointer also supports const pointers.
        auto nonConstPointer = const_cast<TWithoutConst*>(m_RawPointer);
        m_ObserverTag = nonConstPointer->AddObserver(itk::DeleteEvent(), command);
      }
    }

    void RemoveDeleteEventObserver()
    {
      if (nullptr != m_RawPointer)
      {
        using TWithoutConst = typename std::remove_const_t<T>;
        //cast the pointer to non const before removing the observer. This is done to ensure that
        //weak pointer also supports const pointers.
        auto nonConstPointer = const_cast<TWithoutConst*>(m_RawPointer);
        nonConstPointer->RemoveObserver(m_ObserverTag);
      }
    }

    void OnDeleteEvent() noexcept
    {
      // Don't remove any observers from the observed object as it is about to
      // die and can't handle this operation anymore.

      m_RawPointer = nullptr;

      if (m_DeleteEventCallback)
        m_DeleteEventCallback();
    }

    // In C++20, operator== and operator<=> generate all comparison operators
    // (!=, <, >, <=, >=) and their reversed-argument forms automatically.
    // This also covers comparisons to T::Pointer and T::ConstPointer as
    // itk::SmartPointer can be implicitly converted to a raw pointer.

    friend bool operator ==(const WeakPointer &left, const WeakPointer &right) noexcept
    {
      return left.m_RawPointer == right.m_RawPointer;
    }

    friend std::strong_ordering operator <=>(const WeakPointer &left, const WeakPointer &right) noexcept
    {
      return std::compare_three_way()(left.m_RawPointer, right.m_RawPointer);
    }

    friend bool operator ==(const WeakPointer &left, const T *right) noexcept
    {
      return left.m_RawPointer == right;
    }

    friend std::strong_ordering operator <=>(const WeakPointer &left, const T *right) noexcept
    {
      return std::compare_three_way()(left.m_RawPointer, right);
    }

    T *m_RawPointer;

    // m_ObserverTag is completely managed by the two methods
    // AddDeleteEventObserver() and RemoveDeleteEventObserver(). There
    // isn't any need to initialize or use it at all outside of these methods.
    unsigned long m_ObserverTag;

    DeleteEventCallbackType m_DeleteEventCallback;
  };
}

#endif
