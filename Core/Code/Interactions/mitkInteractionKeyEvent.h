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

#ifndef MITKINTERACTIONKEYEVENT_H_
#define MITKINTERACTIONKEYEVENT_H_

#include "itkObject.h"
#include "itkObjectFactory.h"
#include "mitkCommon.h"
#include "mitkInteractionEventConst.h"
#include "mitkInteractionPositionEvent.h"
#include "mitkBaseRenderer.h"
#include "mitkInteractionEvent.h"
#include <string.h>

#include <MitkExports.h>

/*
 * Note: A Click with the MiddleButton is to be handled with MousePressEvents
 */
namespace mitk
{
  /**
   * \class InteractionKeyEvent
   * \brief Handles key events.
   */


  class MITK_CORE_EXPORT InteractionKeyEvent : public InteractionEvent {

  public:
    mitkClassMacro(InteractionKeyEvent,InteractionEvent);
    mitkNewMacro3Param(Self, BaseRenderer*, std::string , ModifierKeys);

    virtual bool isEqual(InteractionEvent::Pointer);
    virtual bool IsSuperClassOf(InteractionEvent::Pointer baseClass);

    ModifierKeys GetModifiers() const;
    std::string GetKey() const;

  protected:
    InteractionKeyEvent(BaseRenderer*, std::string key, ModifierKeys modifiers);
    virtual ~InteractionKeyEvent();

  private:
    std::string m_Key;
    ModifierKeys m_Modifiers;
  };
} /* namespace mitk */

#endif /* MITKINTERACTIONKEYEVENT_H_ */
