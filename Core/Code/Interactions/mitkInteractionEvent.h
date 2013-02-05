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

#ifndef MITKINTERACTIONEVENT_H_
#define MITKINTERACTIONEVENT_H_

#include "itkObject.h"
#include "itkObjectFactory.h"
#include "mitkCommon.h"
#include "mitkBaseRenderer.h"

#include <MitkExports.h>
#include <string>

namespace mitk
{

  class MITK_CORE_EXPORT InteractionEvent: public itk::Object
  {


  public:
    mitkClassMacro(InteractionEvent,itk::Object);
    mitkNewMacro2Param(Self,BaseRenderer*, std::string);
    void SetSender(BaseRenderer* sender);
    BaseRenderer* GetSender();

    /**
     * Implementation of equality for each event class.
     * Equality does \b not mean an exact copy or pointer equality.
     *
     * A match is determined by agreement in all attributes that are necessary to describe
     * the event for a state machine transition.
     * E.g. for a mouse event press event, it is important which modifiers are used,
     * which mouse button was used to triggered the event, but the mouse position is irrelevant.
     */
    virtual bool MatchesTemplate(InteractionEvent::Pointer);

    /**
     * Return unique string identifier that gives the event class of this object, as it can be used in a state machine pattern.
     */
    std::string GetEventClass();
    /**
     * This class implements an up cast to check if the provided baseClass object is derived from this class.
     * This function is used to support polymorphism on state machine pattern (XML) level.
     */
    virtual bool IsSuperClassOf(InteractionEvent::Pointer baseClass);

  protected:
    InteractionEvent(BaseRenderer*, std::string);
    virtual ~InteractionEvent();

  private:
    BaseRenderer* m_Sender;
    std::string m_EventClass;
  };

} /* namespace mitk */

#endif /* MITKINTERACTIONEVENT_H_ */
