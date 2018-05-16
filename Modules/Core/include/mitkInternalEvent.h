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
#include "mitkBaseRenderer.h"
#include "mitkCommon.h"
#include "mitkInteractionEvent.h"
#include "mitkInteractionEventConst.h"
#include <string>

#include <MitkCoreExports.h>

namespace mitk
{
  /**
   *\brief Class to create events from within the application to signal about internal events.
   *
   * These events can target a specific DataInteractor, if this DataInteractor is specified in the constructor;
   * else this parameter is set to nullptr and the event is treated as a regular event.
   */
  class MITKCORE_EXPORT InternalEvent : public InteractionEvent
  {
  public:
    mitkClassMacro(InternalEvent, InteractionEvent);
    mitkNewMacro3Param(Self, BaseRenderer *, DataInteractor *, const std::string &);

    std::string GetSignalName() const;
    DataInteractor *GetTargetInteractor() const;

    bool IsSuperClassOf(const InteractionEvent::Pointer &baseClass) const override;

  protected:
    InternalEvent(BaseRenderer *, DataInteractor *destInteractor, const std::string &signalName);
    ~InternalEvent() override;

    bool IsEqual(const InteractionEvent &) const override;

  private:
    DataInteractor::Pointer m_DataInteractor;
    std::string m_SignalName;
  };
}
#endif /* mitkInternalEvent_h */
