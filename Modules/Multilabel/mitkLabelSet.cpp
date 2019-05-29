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

#include "mitkLabelSet.h"
#include "mitkDICOMSegmentationPropertyHelper.h"

#include <itkCommand.h>

mitk::LabelSet::LabelSet() : m_ActiveLabelValue(0), m_Layer(0)
{
  m_LookupTable = mitk::LookupTable::New();
  m_LookupTable->SetType(mitk::LookupTable::MULTILABEL);
}

mitk::LabelSet::~LabelSet()
{
  m_LabelContainer.clear();
}

mitk::LabelSet::LabelSet(const LabelSet &other)
  : itk::Object(),
    m_LookupTable(other.GetLookupTable()->Clone()),
    m_ActiveLabelValue(other.GetActiveLabel()->GetValue()),
    m_Layer(other.GetLayer())
{
  // clone Labels
  auto otherIt = other.IteratorConstBegin();
  for (; otherIt != other.IteratorConstEnd(); ++otherIt)
  {
    m_LabelContainer[otherIt->first] = otherIt->second->Clone();

    itk::SimpleMemberCommand<LabelSet>::Pointer command = itk::SimpleMemberCommand<LabelSet>::New();
    command->SetCallbackFunction(this, &LabelSet::OnLabelModified);
    m_LabelContainer[otherIt->first]->AddObserver(itk::ModifiedEvent(), command);
  }
}

void mitk::LabelSet::OnLabelModified()
{
  ModifyLabelEvent.Send();
  Superclass::Modified();
}

mitk::LabelSet::LabelContainerConstIteratorType mitk::LabelSet::IteratorConstEnd() const
{
  return m_LabelContainer.end();
}

mitk::LabelSet::LabelContainerConstIteratorType mitk::LabelSet::IteratorConstBegin() const
{
  return m_LabelContainer.begin();
}

mitk::LabelSet::LabelContainerIteratorType mitk::LabelSet::IteratorEnd()
{
  return m_LabelContainer.end();
}

mitk::LabelSet::LabelContainerIteratorType mitk::LabelSet::IteratorBegin()
{
  return m_LabelContainer.begin();
}

unsigned int mitk::LabelSet::GetNumberOfLabels() const
{
  return m_LabelContainer.size();
}

void mitk::LabelSet::SetLayer(unsigned int layer)
{
  m_Layer = layer;
  Modified();
}

void mitk::LabelSet::SetActiveLabel(PixelType pixelValue)
{
  m_ActiveLabelValue = pixelValue;
  ActiveLabelEvent.Send(pixelValue);
  Modified();
}

bool mitk::LabelSet::ExistLabel(PixelType pixelValue)
{
  return m_LabelContainer.count(pixelValue) > 0 ? true : false;
}

// TODO Parameter as Smartpointer
void mitk::LabelSet::AddLabel(mitk::Label *label)
{
  unsigned int max_size = mitk::Label::MAX_LABEL_VALUE + 1;
  if (m_LabelContainer.size() >= max_size)
    return;

  mitk::Label::Pointer newLabel(label->Clone());

  // TODO use layer of label parameter
  newLabel->SetLayer(m_Layer);

  PixelType pixelValue;
  if (m_LabelContainer.empty())
  {
    pixelValue = newLabel->GetValue();
  }
  else
  {
    pixelValue = m_LabelContainer.rbegin()->first;

    if (pixelValue >= newLabel->GetValue() && m_LabelContainer.find(newLabel->GetValue()) != m_LabelContainer.end())
    {
      ++pixelValue;
      newLabel->SetValue(pixelValue);
    }
    else
    {
      pixelValue = newLabel->GetValue();
    }
  }

  // new map entry
  m_LabelContainer[pixelValue] = newLabel;
  UpdateLookupTable(pixelValue);

  // add DICOM information of the label
  DICOMSegmentationPropertyHelper::SetDICOMSegmentProperties(newLabel);

  itk::SimpleMemberCommand<LabelSet>::Pointer command = itk::SimpleMemberCommand<LabelSet>::New();
  command->SetCallbackFunction(this, &LabelSet::OnLabelModified);
  newLabel->AddObserver(itk::ModifiedEvent(), command);
  // newLabel->AddObserver(itk::ModifiedEvent(),command);

  SetActiveLabel(newLabel->GetValue());
  AddLabelEvent.Send();
  Modified();
}

void mitk::LabelSet::AddLabel(const std::string &name, const mitk::Color &color)
{
  if (m_LabelContainer.size() > 255)
    return;

  mitk::Label::Pointer newLabel = mitk::Label::New();
  newLabel->SetName(name);
  newLabel->SetColor(color);
  AddLabel(newLabel);
}

void mitk::LabelSet::RenameLabel(PixelType pixelValue, const std::string &name, const mitk::Color &color)
{
  mitk::Label *label = GetLabel(pixelValue);
  label->SetName(name);
  label->SetColor(color);

  // change DICOM information of the label
  DICOMSegmentationPropertyHelper::SetDICOMSegmentProperties(label);
}

void mitk::LabelSet::SetLookupTable(mitk::LookupTable *lut)
{
  m_LookupTable = lut;
  Modified();
}

void mitk::LabelSet::PrintSelf(std::ostream & /*os*/, itk::Indent /*indent*/) const
{
}

void mitk::LabelSet::RemoveLabel(PixelType pixelValue)
{
  auto it = m_LabelContainer.rbegin();
  PixelType nextActivePixelValue = it->first;

  for (; it != m_LabelContainer.rend(); ++it)
  {
    if (it->first == pixelValue)
    {
      it->second->RemoveAllObservers();
      m_LabelContainer.erase(pixelValue);
      break;
    }
    nextActivePixelValue = it->first;
  }

  if (m_ActiveLabelValue == pixelValue)
  {
    if (ExistLabel(nextActivePixelValue))
      SetActiveLabel(nextActivePixelValue);
    else
      SetActiveLabel(m_LabelContainer.rbegin()->first);
  }

  RemoveLabelEvent.Send();

  Modified();
}

void mitk::LabelSet::RemoveAllLabels()
{
  auto _it = IteratorBegin();
  for (; _it != IteratorConstEnd();)
  {
    RemoveLabelEvent.Send();
    m_LabelContainer.erase(_it++);
  }
  AllLabelsModifiedEvent.Send();
}

void mitk::LabelSet::SetNextActiveLabel()
{
  auto it = m_LabelContainer.begin();

  for (; it != m_LabelContainer.end(); ++it)
  {
    if (it->first == m_ActiveLabelValue)
    {
      // go to next label
      ++it;
      if (it == m_LabelContainer.end())
      {
        // end of container; next label is first label
        it = m_LabelContainer.begin();
      }
      break; // found the active label; finish loop
    }
  }

  SetActiveLabel(it->first);
}

void mitk::LabelSet::SetAllLabelsLocked(bool value)
{
  auto _end = m_LabelContainer.end();
  auto _it = m_LabelContainer.begin();
  for (; _it != _end; ++_it)
    _it->second->SetLocked(value);
  AllLabelsModifiedEvent.Send();
  Modified();
}

void mitk::LabelSet::SetAllLabelsVisible(bool value)
{
  auto _end = m_LabelContainer.end();
  auto _it = m_LabelContainer.begin();
  for (; _it != _end; ++_it)
  {
    _it->second->SetVisible(value);
    UpdateLookupTable(_it->first);
  }
  AllLabelsModifiedEvent.Send();
  Modified();
}

void mitk::LabelSet::UpdateLookupTable(PixelType pixelValue)
{
  const mitk::Color &color = GetLabel(pixelValue)->GetColor();

  double rgba[4];
  m_LookupTable->GetTableValue(static_cast<int>(pixelValue), rgba);
  rgba[0] = color.GetRed();
  rgba[1] = color.GetGreen();
  rgba[2] = color.GetBlue();
  if (GetLabel(pixelValue)->GetVisible())
    rgba[3] = GetLabel(pixelValue)->GetOpacity();
  else
    rgba[3] = 0.0;
  m_LookupTable->SetTableValue(static_cast<int>(pixelValue), rgba);
}

mitk::Label *mitk::LabelSet::GetLabel(PixelType pixelValue)
{
  if (m_LabelContainer.find(pixelValue) == m_LabelContainer.end())
    return nullptr;
  return m_LabelContainer[pixelValue];
}

const mitk::Label *mitk::LabelSet::GetLabel(PixelType pixelValue) const
{
  auto it = m_LabelContainer.find(pixelValue);
  if (it == m_LabelContainer.end())
    return nullptr;
  return it->second.GetPointer();
}

bool mitk::Equal(const mitk::LabelSet &leftHandSide, const mitk::LabelSet &rightHandSide, ScalarType eps, bool verbose)
{
  bool returnValue = true;
  // LabelSetmembers

  MITK_INFO(verbose) << "--- LabelSet Equal ---";

  // m_LookupTable;
  const mitk::LookupTable *lhsLUT = leftHandSide.GetLookupTable();
  const mitk::LookupTable *rhsLUT = rightHandSide.GetLookupTable();

  returnValue = *lhsLUT == *rhsLUT;
  if (!returnValue)
  {
    MITK_INFO(verbose) << "Lookup tabels not equal.";
    return returnValue;
    ;
  }

  // m_ActiveLabel;
  returnValue = mitk::Equal(*leftHandSide.GetActiveLabel(), *rightHandSide.GetActiveLabel(), eps, verbose);
  if (!returnValue)
  {
    MITK_INFO(verbose) << "Active label not equal.";
    return returnValue;
    ;
  }

  // m_Layer;
  returnValue = leftHandSide.GetLayer() == rightHandSide.GetLayer();
  if (!returnValue)
  {
    MITK_INFO(verbose) << "Layer index not equal.";
    return returnValue;
    ;
  }

  // container size;
  returnValue = leftHandSide.GetNumberOfLabels() == rightHandSide.GetNumberOfLabels();
  if (!returnValue)
  {
    MITK_INFO(verbose) << "Number of labels not equal.";
    return returnValue;
    ;
  }

  // Label container (map)

  // m_LabelContainer;
  auto lhsit = leftHandSide.IteratorConstBegin();
  auto rhsit = rightHandSide.IteratorConstBegin();
  for (; lhsit != leftHandSide.IteratorConstEnd(); ++lhsit, ++rhsit)
  {
    returnValue = rhsit->first == lhsit->first;
    if (!returnValue)
    {
      MITK_INFO(verbose) << "Label in label container not equal.";
      return returnValue;
      ;
    }

    returnValue = mitk::Equal(*(rhsit->second), *(lhsit->second), eps, verbose);
    if (!returnValue)
    {
      MITK_INFO(verbose) << "Label in label container not equal.";
      return returnValue;
      ;
    }
  }

  return returnValue;
}
