/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
