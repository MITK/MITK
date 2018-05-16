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

#ifndef MITKMOUSEWHEELEVENT_H_
#define MITKMOUSEWHEELEVENT_H_

#include "itkObject.h"
#include "itkObjectFactory.h"
#include "mitkBaseRenderer.h"
#include "mitkCommon.h"
#include "mitkInteractionEvent.h"
#include "mitkInteractionEventConst.h"
#include "mitkInteractionPositionEvent.h"

#include <MitkCoreExports.h>

/**
 * Note: A Click with the MiddleButton is to be handled with MousePressEvents
 */

namespace mitk
{
  class MITKCORE_EXPORT MouseWheelEvent : public InteractionPositionEvent
  {
  public:
    mitkClassMacro(MouseWheelEvent, InteractionPositionEvent)
      mitkNewMacro5Param(Self, BaseRenderer *, const Point2D &, MouseButtons, ModifierKeys, int)

        ModifierKeys GetModifiers() const;
    MouseButtons GetButtonStates() const;
    void SetModifiers(ModifierKeys modifiers);
    void SetButtonStates(MouseButtons buttons);
    int GetWheelDelta() const;
    void SetWheelDelta(int delta);

    bool IsSuperClassOf(const InteractionEvent::Pointer &baseClass) const override;

  protected:
    MouseWheelEvent(BaseRenderer * = nullptr,
                    const Point2D &mousePosition = Point2D(),
                    MouseButtons buttonStates = NoButton,
                    ModifierKeys modifiers = NoKey,
                    int wheelDelta = 0);
    ~MouseWheelEvent() override;

    bool IsEqual(const InteractionEvent &) const override;

  private:
    int m_WheelDelta;
    MouseButtons m_ButtonStates;
    ModifierKeys m_Modifiers;
  };
} /* namespace mitk */

#endif /* MITKMOUSEPRESSEVENT_H_ */
