/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKSEGMENTATIONINTERACTIONEVENTS_H
#define MITKSEGMENTATIONINTERACTIONEVENTS_H

#include <MitkSegmentationExports.h>

// mitk core
#include "mitkInteractionEvent.h"

// itk
#include <itkEventObject.h>

namespace mitk
{
  class MITKSEGMENTATION_EXPORT SegmentationInteractionEvent : public itk::AnyEvent
  {
  public:

    typedef SegmentationInteractionEvent Self;
    typedef itk::AnyEvent Superclass;

    SegmentationInteractionEvent() : m_InteractionEvent(nullptr) {}
    SegmentationInteractionEvent(InteractionEvent* interactionEvent, bool enteredRenderWindow)
      : m_InteractionEvent(interactionEvent)
      , m_EnteredRenderWindow(enteredRenderWindow)
    {
    }
    ~SegmentationInteractionEvent() override {}
    const char* GetEventName() const override { return "SegmentationInteractionEvent"; }
    bool CheckEvent(const itk::EventObject* e) const override { return dynamic_cast<const Self *>(e) != nullptr; }
    itk::EventObject* MakeObject() const override { return new Self(m_InteractionEvent, m_EnteredRenderWindow); }
    InteractionEvent* GetInteractionEvent() const { return m_InteractionEvent; }
    bool HasEnteredRenderWindow() const { return m_EnteredRenderWindow; }
    BaseRenderer* GetSender() const
    {
      return m_InteractionEvent != nullptr ? m_InteractionEvent->GetSender() : nullptr;
    }
    SegmentationInteractionEvent(const Self& s)
      : Superclass(s)
      , m_InteractionEvent(s.GetInteractionEvent())
      , m_EnteredRenderWindow(s.m_EnteredRenderWindow)
    {
    };

  private:

    InteractionEvent* m_InteractionEvent;
    bool m_EnteredRenderWindow;
    void operator=(const Self &);
  };

} // end namespace

#endif // MITKSEGMENTATIONINTERACTIONEVENTS_H
