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
     * Implementation of equality for each event class
     */
    virtual bool isEqual(InteractionEvent::Pointer);
    std::string GetEventClass();
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
