/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkMouseMoveEvent_h
#define mitkMouseMoveEvent_h
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
  class MITKCORE_EXPORT MouseMoveEvent : public InteractionPositionEvent
  {
  public:
    mitkClassMacro(MouseMoveEvent, InteractionPositionEvent);
    mitkNewMacro4Param(Self, BaseRenderer *, const Point2D &, MouseButtons, ModifierKeys);

    ModifierKeys GetModifiers() const;
    MouseButtons GetButtonStates() const;
    void SetModifiers(ModifierKeys modifiers);
    void SetButtonStates(MouseButtons buttons);

    bool IsSuperClassOf(const InteractionEvent::Pointer &baseClass) const override;

  protected:
    MouseMoveEvent(BaseRenderer *,
                   const Point2D & = Point2D(),
                   MouseButtons buttonStates = NoButton,
                   ModifierKeys modifiers = NoKey);
    ~MouseMoveEvent() override;

    bool IsEqual(const InteractionEvent &) const override;

  private:
    MouseButtons m_ButtonStates;
    ModifierKeys m_Modifiers;
  };
} /* namespace mitk */

#endif
