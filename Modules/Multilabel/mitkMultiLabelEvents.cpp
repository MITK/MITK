/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkMultiLabelEvents.h"

namespace mitk
{
  AnyLabelEvent::AnyLabelEvent(Label::PixelType labelValue) : m_LabelValue(labelValue) {}

  AnyLabelEvent::AnyLabelEvent(const AnyLabelEvent& s) : itk::ModifiedEvent(s), m_LabelValue(s.m_LabelValue) {}

  AnyLabelEvent::~AnyLabelEvent() {}

  const char* AnyLabelEvent::GetEventName() const { return "AnyLabelEvent"; }

  bool AnyLabelEvent::CheckEvent(const itk::EventObject* e) const
  {
    auto castedE = dynamic_cast<const AnyLabelEvent*>(e);

    return (nullptr != castedE) && ((castedE->m_LabelValue == m_LabelValue) || (ANY_LABEL == castedE->m_LabelValue) || (ANY_LABEL == m_LabelValue));
  }

  itk::EventObject* AnyLabelEvent::MakeObject() const { return new AnyLabelEvent(); }

  void AnyLabelEvent::SetLabelValue(Label::PixelType labelValue)
  {
    m_LabelValue = labelValue;
  }

  Label::PixelType AnyLabelEvent::GetLabelValue() const
  {
    return m_LabelValue;
  }

  mitkMultiLabelEventMacroDefinition(LabelAddedEvent, AnyLabelEvent, Label::PixelType);
  mitkMultiLabelEventMacroDefinition(LabelModifiedEvent, AnyLabelEvent, Label::PixelType);
  mitkMultiLabelEventMacroDefinition(LabelRemovedEvent, AnyLabelEvent, Label::PixelType);

  LabelsChangedEvent::LabelsChangedEvent(std::vector<Label::PixelType> labelValues) : m_LabelValues(labelValues) {}

  LabelsChangedEvent::LabelsChangedEvent(const LabelsChangedEvent& s) : itk::ModifiedEvent(s), m_LabelValues(s.m_LabelValues) {}

  LabelsChangedEvent::~LabelsChangedEvent() {}

  const char* LabelsChangedEvent::GetEventName() const { return "LabelsChangedEvent"; }

  bool LabelsChangedEvent::CheckEvent(const itk::EventObject* e) const
  {
    auto castedE = dynamic_cast<const LabelsChangedEvent*>(e);

    return (nullptr != castedE) && ((castedE->m_LabelValues == m_LabelValues) || (castedE->m_LabelValues.empty()) || (m_LabelValues.empty()));
  }

  itk::EventObject* LabelsChangedEvent::MakeObject() const { return new LabelsChangedEvent(); }

  void LabelsChangedEvent::SetLabelValues(std::vector<Label::PixelType> labelValues)
  {
    m_LabelValues = labelValues;
  }

  std::vector<Label::PixelType> LabelsChangedEvent::GetLabelValues() const
  {
    return m_LabelValues;
  }

  AnyGroupEvent::AnyGroupEvent(GroupIndexType groupID) : m_GroupID(groupID) {}

  AnyGroupEvent::AnyGroupEvent(const AnyGroupEvent& s) : itk::ModifiedEvent(s), m_GroupID(s.m_GroupID) {}

  AnyGroupEvent::~AnyGroupEvent() {}

  const char* AnyGroupEvent::GetEventName() const { return "AnyGroupEvent"; }

  bool AnyGroupEvent::CheckEvent(const itk::EventObject* e) const
  {
    auto castedE = dynamic_cast<const AnyGroupEvent*>(e);

    return (nullptr != castedE) && ((castedE->m_GroupID == m_GroupID) || (ANY_GROUP == castedE->m_GroupID) || (ANY_GROUP == m_GroupID));
  }

  itk::EventObject* AnyGroupEvent::MakeObject() const { return new AnyGroupEvent(); }

  void AnyGroupEvent::SetGroupID(AnyGroupEvent::GroupIndexType groupID)
  {
    m_GroupID = groupID;
  }

  AnyGroupEvent::GroupIndexType AnyGroupEvent::GetGroupID() const
  {
    return m_GroupID;
  }

  mitkMultiLabelEventMacroDefinition(GroupAddedEvent, AnyGroupEvent, AnyGroupEvent::GroupIndexType);
  mitkMultiLabelEventMacroDefinition(GroupModifiedEvent, AnyGroupEvent, AnyGroupEvent::GroupIndexType);
  mitkMultiLabelEventMacroDefinition(GroupRemovedEvent, AnyGroupEvent, AnyGroupEvent::GroupIndexType);
}