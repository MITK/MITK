/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkITKEventObserverGuard.h"

#include <itkObject.h>
#include <itkEventObject.h>
#include <mitkWeakPointer.h>

namespace mitk
{
  struct ITKEventObserverGuard::Impl
  {
    explicit Impl(const itk::Object* sender, unsigned long observerTag);

    ~Impl();

  private:
    mitk::WeakPointer<itk::Object> m_Sender;
    unsigned long m_ObserverTag;
  };

  ITKEventObserverGuard::Impl::Impl(const itk::Object* sender, unsigned long observerTag) : m_Sender(const_cast<itk::Object*>(sender)), m_ObserverTag(observerTag)
  {
    //we cast const sender to non const in order to be able to remove observers but
    //also support constness in code that uses the guard.
  }

  ITKEventObserverGuard::Impl::~Impl()
  {
    auto sender = m_Sender.Lock();

    if (sender.IsNotNull())
    {
      sender->RemoveObserver(m_ObserverTag);
    }
  }

  ITKEventObserverGuard::ITKEventObserverGuard() : m_ITKEventObserverGuardImpl(nullptr) {}

  ITKEventObserverGuard::ITKEventObserverGuard(const itk::Object* sender, unsigned long observerTag) : m_ITKEventObserverGuardImpl(new Impl(sender, observerTag)) {}

  ITKEventObserverGuard::ITKEventObserverGuard(const itk::Object* sender, const itk::EventObject& event, itk::Command* command)
  {
    auto tag = sender->AddObserver(event, command);
    m_ITKEventObserverGuardImpl = std::make_unique<Impl>(sender, tag);
  }

  ITKEventObserverGuard::ITKEventObserverGuard(const itk::Object* sender, const itk::EventObject& event, std::function<void(const itk::EventObject&)> function)
  {
    auto tag = sender->AddObserver(event, function);
    m_ITKEventObserverGuardImpl = std::make_unique<Impl>(sender, tag);
  }

  ITKEventObserverGuard::ITKEventObserverGuard(ITKEventObserverGuard&& other): m_ITKEventObserverGuardImpl(std::move(other.m_ITKEventObserverGuardImpl))
  {
  }

  ITKEventObserverGuard::~ITKEventObserverGuard() { }

  ITKEventObserverGuard& ITKEventObserverGuard::operator=(ITKEventObserverGuard&& other)
  {
    if (&other != this)
    {
      m_ITKEventObserverGuardImpl = std::move(other.m_ITKEventObserverGuardImpl);
    }

    return *this;
  }

  void ITKEventObserverGuard::Reset()
  {
    m_ITKEventObserverGuardImpl.reset();
  }

  void ITKEventObserverGuard::Reset(const itk::Object* sender, unsigned long observerTag)
  {
    m_ITKEventObserverGuardImpl.reset(new Impl(sender, observerTag));
  }

  void ITKEventObserverGuard::Reset(const itk::Object* sender, const itk::EventObject& event, itk::Command* command)
  {
    auto tag = sender->AddObserver(event, command);
    m_ITKEventObserverGuardImpl.reset(new Impl(sender, tag));
  }

  void ITKEventObserverGuard::Reset(const itk::Object* sender, const itk::EventObject& event, std::function<void(const itk::EventObject&)> function)
  {
    auto tag = sender->AddObserver(event, function);
    m_ITKEventObserverGuardImpl.reset(new Impl(sender, tag));
  }

  bool ITKEventObserverGuard::IsInitialized() const
  {
    return nullptr != this->m_ITKEventObserverGuardImpl;
  }

}
