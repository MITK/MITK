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

#ifndef MITKMOUSEMOVEEVENT_H_
#define MITKMOUSEMOVEEVENT_H_
#include "itkObject.h"
#include "itkObjectFactory.h"
#include "mitkCommon.h"
#include "mitkInteractionEventConst.h"
#include "mitkInteractionPositionEvent.h"
#include "mitkBaseRenderer.h"
#include "mitkInteractionEvent.h"

#include <MitkExports.h>

namespace mitk
{

  class MITK_CORE_EXPORT MouseMoveEvent : public InteractionPositionEvent {

  public:
    mitkClassMacro(MouseMoveEvent,InteractionPositionEvent);
    mitkNewMacro4Param(Self, BaseRenderer*, Point2D , MouseButtons , ModifierKeys);

    ModifierKeys GetModifiers() const;
    MouseButtons GetButtonStates() const;
    void SetModifiers(ModifierKeys modifiers);
    void SetButtonStates(MouseButtons buttons);

    virtual bool MatchesTemplate(InteractionEvent::Pointer);

  protected:
    MouseMoveEvent(BaseRenderer*, Point2D = Point2D(), MouseButtons buttonStates = NoButton, mitk::ModifierKeys modifiers = NoKey);
    virtual ~MouseMoveEvent();

  private:
    MouseButtons m_ButtonStates;
    ModifierKeys m_Modifiers;

  };
} /* namespace mitk */

#endif /* MITKMOUSEMOVEEVENT_H_ */
