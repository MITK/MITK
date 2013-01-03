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

#ifndef mitkInternalEvent_h
#define mitkInternalEvent_h

#include "itkObject.h"
#include "itkObjectFactory.h"
#include "mitkCommon.h"
#include "mitkInteractionEventConst.h"
#include "mitkInteractionEvent.h"
#include "mitkBaseRenderer.h"
#include <string>


#include <MitkExports.h>

namespace mitk
{
  class MITK_CORE_EXPORT InternalEvent : public InteractionEvent {

  public:
    mitkClassMacro(InternalEvent,InteractionEvent);
    mitkNewMacro2Param(Self, BaseRenderer*, std::string);

    virtual bool isEqual(InteractionEvent::Pointer);

  protected:
    InternalEvent(BaseRenderer*, std::string signalName);
    virtual ~InternalEvent();

    std::string GetSignalName();

  private:
    std::string m_SignalName;
  };
} /* namespace mitk */

#endif /* mitkInternalEvent_h */
