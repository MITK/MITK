/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef MITKMOUSERELEASEEVENT_H_
#define MITKMOUSERELEASEEVENT_H_

#include "itkObject.h"
#include "itkObjectFactory.h"
#include "mitkBaseRenderer.h"
#include "mitkCommon.h"
#include "mitkInteractionEvent.h"
#include "mitkInteractionEventConst.h"
#include "mitkInteractionPositionEvent.h"

#include <MitkCoreExports.h>

namespace mitk
{
  class MITKCORE_EXPORT MouseReleaseEvent : public InteractionPositionEvent
  {
  public:
    mitkClassMacro(MouseReleaseEvent, InteractionPositionEvent)
      mitkNewMacro5Param(Self, BaseRenderer *, const Point2D &, MouseButtons, ModifierKeys, MouseButtons)

        ModifierKeys GetModifiers() const;
    MouseButtons GetButtonStates() const;
    void SetModifiers(ModifierKeys modifiers);
    void SetButtonStates(MouseButtons buttons);
    MouseButtons GetEventButton() const;
    void SetEventButton(MouseButtons buttons);

    bool IsSuperClassOf(const InteractionEvent::Pointer &baseClass) const override;

  protected:
    MouseReleaseEvent(BaseRenderer *,
                      const mitk::Point2D &mousePosition = Point2D(),
                      MouseButtons buttonStates = NoButton,
                      ModifierKeys modifiers = NoKey,
                      MouseButtons eventButton = NoButton);
    ~MouseReleaseEvent() override;

    bool IsEqual(const InteractionEvent &) const override;

  private:
    MouseButtons m_EventButton;
    MouseButtons m_ButtonStates;
    ModifierKeys m_Modifiers;
  };
} /* namespace mitk */

#endif /* MITKMOUSERELEASEEVENT_H_ */
