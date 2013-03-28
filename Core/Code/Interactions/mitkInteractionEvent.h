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

#include "itkLightObject.h"
#include "itkObjectFactory.h"
#include "mitkCommon.h"
#include "mitkBaseRenderer.h"

#include <MitkExports.h>
#include <string>

namespace mitk
{

  class MITK_CORE_EXPORT InteractionEvent: public itk::LightObject
  {

  public:
    mitkClassMacro(InteractionEvent,itk::LightObject)
    mitkNewMacro2Param(Self,BaseRenderer*, const std::string&)

    void SetSender(BaseRenderer* sender);
    BaseRenderer* GetSender() const;

    /**
     * Return unique string identifier that gives the event class of this object, as it can be used in a state machine pattern.
     * --- itk
     */
    std::string GetEventClass() const;

    /**
     * This class implements an up cast to check if the provided baseClass object is derived from this class.
     * This function is used to support polymorphism on state machine pattern (XML) level.
     */
    virtual bool IsSuperClassOf(const InteractionEvent::Pointer& baseClass) const;

  protected:
    InteractionEvent(BaseRenderer*, const std::string&);
    virtual ~InteractionEvent();

    friend bool operator==(const InteractionEvent&, const InteractionEvent&);
    virtual bool Equals(const InteractionEvent& other) const;

  private:
    BaseRenderer* m_Sender;
    std::string m_EventClass;
  };

  /**
   * Implementation of equality for event classes.
   * Equality does \b not mean an exact copy or pointer equality.
   *
   * A match is determined by agreement in all attributes that are necessary to describe
   * the event for a state machine transition.
   * E.g. for a mouse event press event, it is important which modifiers are used,
   * which mouse button was used to triggered the event, but the mouse position is irrelevant.
   */
  MITK_CORE_EXPORT bool operator==(const InteractionEvent& a, const InteractionEvent& b);
  MITK_CORE_EXPORT bool operator!=(const InteractionEvent& a, const InteractionEvent& b);

} /* namespace mitk */

#endif /* MITKINTERACTIONEVENT_H_ */
