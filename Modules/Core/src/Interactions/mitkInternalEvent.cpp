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

#include "mitkInternalEvent.h"
#include "mitkDataInteractor.h"

mitk::InternalEvent::InternalEvent(mitk::BaseRenderer *baseRenderer,
                                   DataInteractor *sourceInteractor,
                                   const std::string &signalName)
  : InteractionEvent(baseRenderer), m_DataInteractor(sourceInteractor), m_SignalName(signalName)
{
}

bool mitk::InternalEvent::IsEqual(const mitk::InteractionEvent &interactionEvent) const
{
  const auto &internalEvent = static_cast<const mitk::InternalEvent &>(interactionEvent);
  return (m_SignalName == internalEvent.GetSignalName() && Superclass::IsEqual(interactionEvent));
}

mitk::InternalEvent::~InternalEvent()
{
}

std::string mitk::InternalEvent::GetSignalName() const
{
  return m_SignalName;
}

mitk::DataInteractor *mitk::InternalEvent::GetTargetInteractor() const
{
  return m_DataInteractor.GetPointer();
}

bool mitk::InternalEvent::IsSuperClassOf(const InteractionEvent::Pointer &baseClass) const
{
  return (nullptr != dynamic_cast<InternalEvent *>(baseClass.GetPointer()));
}
