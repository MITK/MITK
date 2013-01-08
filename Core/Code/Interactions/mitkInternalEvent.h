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

  // Constants for Internal commands that are understood by the Dispatcher
  const std::string INTERNALDeactivateMe = "DeactivateMe";

  class DataInteracor;
  class MITK_CORE_EXPORT InternalEvent : public InteractionEvent {

  public:
    mitkClassMacro(InternalEvent,InteractionEvent);
    mitkNewMacro3Param(Self, BaseRenderer*, DataInteractor*, std::string);

    std::string GetSignalName();
    DataInteractor* GetTargetInteractor();

    virtual bool isEqual(InteractionEvent::Pointer);

  protected:
    InternalEvent(BaseRenderer*, DataInteractor* sourceInteractor, std::string signalName);
    virtual ~InternalEvent();


  private:
    DataInteractor* m_DataInteractor;
    std::string m_SignalName;
  };
} /* namespace mitk */

#endif /* mitkInternalEvent_h */
