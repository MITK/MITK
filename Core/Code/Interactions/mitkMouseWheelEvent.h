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
#include "mitkCommon.h"
#include "mitkInteractionEventConst.h"
#include "mitkInteractionPositionEvent.h"
#include "mitkBaseRenderer.h"
#include "mitkInteractionEvent.h"

#include <MitkExports.h>

/**
 * Note: A Click with the MiddleButton is to be handled with MousePressEvents
 */


namespace mitk
{
  class MITK_CORE_EXPORT MouseWheelEvent : public InteractionPositionEvent {

  public:
    mitkClassMacro(MouseWheelEvent,InteractionPositionEvent);
    mitkNewMacro5Param(Self, BaseRenderer*, Point2D , MouseButtons , ModifierKeys, int);

    ModifierKeys GetModifiers() const;
    MouseButtons GetButtonStates() const;
    void SetModifiers(ModifierKeys modifiers);
    void SetButtonStates(MouseButtons buttons);
    int GetWheelDelta() const;
    void SetWheelDelta(int delta);

    virtual bool isEqual(InteractionEvent::Pointer);
    virtual bool IsSuperClassOf(InteractionEvent::Pointer baseClass);

  protected:
    MouseWheelEvent(BaseRenderer*, Point2D, MouseButtons buttonStates, ModifierKeys modifiers, int  wheelDelta);
    virtual ~MouseWheelEvent();

  private:
    int m_WheelDelta;
    MouseButtons m_ButtonStates;
    ModifierKeys m_Modifiers;
  };
} /* namespace mitk */

#endif /* MITKMOUSEPRESSEVENT_H_ */
