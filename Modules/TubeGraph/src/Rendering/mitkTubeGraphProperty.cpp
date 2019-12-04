/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkTubeGraphProperty.h"

#include <mitkRenderingManager.h>

mitk::TubeGraphProperty::TubeGraphProperty()
/*: m_LabelGroups(), m_ActiveTubes(), m_TubeToLabelsMap()*/
{
}

mitk::TubeGraphProperty::TubeGraphProperty(const mitk::TubeGraphProperty &other)
  : BaseProperty(other),
    m_ActiveTubes(other.m_ActiveTubes),
    m_LabelGroups(other.m_LabelGroups),
    m_TubeToLabelsMap(other.m_TubeToLabelsMap),
    m_Annotations(other.m_Annotations)
{
}

mitk::TubeGraphProperty::~TubeGraphProperty()
{
  m_ActiveTubes.clear();
  m_TubeToLabelsMap.clear();

  for (auto it = m_LabelGroups.begin(); it != m_LabelGroups.end(); it++)
    delete *it;
  m_LabelGroups.clear();
}

bool mitk::TubeGraphProperty::IsTubeVisible(const TubeDescriptorType &tube)
{
  // search for any label settings for the tube
  if (m_LabelGroups.size() > 0)
  {
    for (auto it = m_TubeToLabelsMap.begin(); it != m_TubeToLabelsMap.end(); it++)
    {
      if (this->TubeDescriptorsCompare(tube, (*it).first.first))
      {
        // At the moment only the first entry is considered
        LabelGroup *lg = this->GetLabelGroupByName((*it).first.second);
        LabelGroup::Label *label = this->GetLabelByName(lg, (*it).second);
        return label->isVisible;
      }
    }
    // If nothing is found, look if the first labelgroup is visible for "undefined" label
    LabelGroup::Label *label = this->GetLabelByName((*m_LabelGroups.begin()), "Undefined");
    return label->isVisible;
  }
  else
    return true;
}

void mitk::TubeGraphProperty::SetTubeActive(const TubeDescriptorType &tube, const bool &active)
{
  // set active
  if (active)
  {
    for (std::vector<TubeDescriptorType>::const_iterator it = m_ActiveTubes.begin(); it != m_ActiveTubes.end(); it++)
    {
      if (this->TubeDescriptorsCompare(tube, (*it)))
      {
        return;
      }
    }
    // if not found, add it
    m_ActiveTubes.push_back(tube);
    this->Modified();
  }
  // set deactive
  else
  {
    for (auto it = m_ActiveTubes.begin(); it != m_ActiveTubes.end(); it++)
    {
      if (this->TubeDescriptorsCompare(tube, (*it)))
      { // if found, delete it
        m_ActiveTubes.erase(it);
        this->Modified();
        return;
      }
    }
  }
  //// render new activation
  // RenderingManager::GetInstance()->RequestUpdateAll();
}

void mitk::TubeGraphProperty::SetTubesActive(std::vector<TubeDescriptorType> &tubes)
{
  for (auto it = tubes.begin(); it != tubes.end(); it++)
    this->SetTubeActive(*it, true);
}

bool mitk::TubeGraphProperty::IsTubeActive(const TubeDescriptorType &tube)
{
  for (std::vector<TubeDescriptorType>::const_iterator it = m_ActiveTubes.begin(); it != m_ActiveTubes.end(); it++)
  {
    if (this->TubeDescriptorsCompare((*it), tube))
      return true;
  }
  return false;
}

std::vector<mitk::TubeGraphProperty::TubeDescriptorType> mitk::TubeGraphProperty::GetActiveTubes()
{
  return m_ActiveTubes;
}

void mitk::TubeGraphProperty::DeactivateAllTubes()
{
  //  if (!m_ActiveTubes.empty())
  m_ActiveTubes.clear();
  this->Modified();
}

void mitk::TubeGraphProperty::AddAnnotation(mitk::TubeGraphProperty::Annotation *annotation)
{
  for (auto it = m_Annotations.begin(); it != m_Annotations.end(); it++)
  {
    if ((*it)->name == annotation->name)
    {
      MITK_INFO << "An Annotation with this name already exists.";
      return;
    }
  }
  m_Annotations.push_back(annotation);
}
mitk::TubeGraphProperty::Annotation *mitk::TubeGraphProperty::GetAnnotationByName(std::string annotation)
{
  for (auto it = m_Annotations.begin(); it != m_Annotations.end(); it++)
  {
    if ((*it)->name == annotation)
      return *it;
  }
  return nullptr;
}
std::vector<mitk::TubeGraphProperty::Annotation *> mitk::TubeGraphProperty::GetAnnotations()
{
  return m_Annotations;
}
void mitk::TubeGraphProperty::RemoveAnnotation(mitk::TubeGraphProperty::Annotation *annotation)
{
  for (auto it = m_Annotations.begin(); it != m_Annotations.end(); it++)
  {
    if ((*it)->name == annotation->name)
    {
      m_Annotations.erase(it);
      return;
    }
  }
}

mitk::Color mitk::TubeGraphProperty::GetColorOfTube(const TubeDescriptorType &tube)
{
  Color color;
  // grey as default color
  color[0] = 150;
  color[1] = 150;
  color[2] = 150;

  bool isActive(false);
  bool hasLabel(false);
  // search the vector with active tubes first. If the tube is active it can not have another color.
  for (std::vector<TubeDescriptorType>::const_iterator it = m_ActiveTubes.begin(); it != m_ActiveTubes.end(); it++)
  {
    if (this->TubeDescriptorsCompare(tube, (*it)))
    {
      color[0] = 255;
      color[1] = 255;
      color[2] = 0;
      isActive = true;
      break;
    }
  }

  if (!isActive)
  {
    // So let see which label is activ on this tube and which color is set.
    if (m_LabelGroups.size() > 0)
    {
      for (auto it = m_TubeToLabelsMap.begin(); it != m_TubeToLabelsMap.end(); it++)
      {
        if (this->TubeDescriptorsCompare(tube, (*it).first.first))
        { // At the moment only the first entry is considered
          LabelGroup *lg = this->GetLabelGroupByName((*it).first.second);
          LabelGroup::Label *label = this->GetLabelByName(lg, (*it).second);
          color = label->labelColor;
          hasLabel = true;
        }
      }
      // If nothing is found, look if the first labelgroup is visible for "undefined" label
      if (!hasLabel)
      {
        LabelGroup::Label *label = this->GetLabelByName(*(m_LabelGroups.begin()), "Undefined");
        color = label->labelColor;
      }
    }
  }

  return color;
}

void mitk::TubeGraphProperty::SetLabelForActivatedTubes(LabelGroup *labelGroup, LabelGroup::Label *label)
{
  bool isUndefined(label->labelName.compare("Undefined") == 0);
  for (unsigned int i = 0; i < m_ActiveTubes.size(); i++)
  {
    bool isInList(false);
    for (auto it = m_TubeToLabelsMap.begin(); it != m_TubeToLabelsMap.end(); it++)
    {
      if ((this->TubeDescriptorsCompare(it->first.first, m_ActiveTubes[i])) &&
          (labelGroup->labelGroupName.compare(it->first.second) == 0))
      {
        // If tube has a label fromlabel group, deleted if "undefined" is clicked.
        if (isUndefined)
        {
          m_TubeToLabelsMap.erase(it);
          break;
        }
        else
        {
          it->second = label->labelName;
          isInList = true;
        }
      }
    }
    if (!isInList && !isUndefined)
    {
      TubeToLabelGroupType tubeToLabelGroup(m_ActiveTubes[i], labelGroup->labelGroupName);
      m_TubeToLabelsMap.insert(std::pair<TubeToLabelGroupType, std::string>(tubeToLabelGroup, label->labelName));
    }
  }
  this->Modified();
  m_ActiveTubes.clear();
}

void mitk::TubeGraphProperty::SetTubesToLabels(std::map<TubeToLabelGroupType, std::string> tubeToLabelMap)
{
  m_TubeToLabelsMap = tubeToLabelMap;
}

std::map<mitk::TubeGraphProperty::TubeToLabelGroupType, std::string> mitk::TubeGraphProperty::GetTubesToLabels()
{
  return m_TubeToLabelsMap;
}

void mitk::TubeGraphProperty::AddLabelGroup(LabelGroup *labelGroup, unsigned int position)
{
  // Check if the name is unique
  for (auto it = m_LabelGroups.begin(); it != m_LabelGroups.end(); it++)
  {
    if (labelGroup->labelGroupName.compare((*it)->labelGroupName) == 0)
    {
      MITK_ERROR << "The label group must be unique! This name already exists!";
      return;
    }
  }

  // Add the label group at position, if you can, otherwise put it at the end of the vector
  if (position < m_LabelGroups.size() - 1)
  {
    auto it = m_LabelGroups.begin() + position;
    m_LabelGroups.insert(it, labelGroup);
  }
  else
  {
    m_LabelGroups.push_back(labelGroup);

    // when given position is larger then vector size, the label group will be added at the last position
    if (position >= m_LabelGroups.size())
      MITK_INFO << "Position is out of space. So the label group was added on last position: "
                << m_LabelGroups.size() - 1;
  }
}

void mitk::TubeGraphProperty::RemoveLabelGroup(LabelGroup *labelGroup)
{
  // find labelGroup in vector
  auto foundElement = std::find(m_LabelGroups.begin(), m_LabelGroups.end(), labelGroup);
  unsigned int pos = foundElement - m_LabelGroups.begin();

  // found it? delete it!
  if (pos < m_LabelGroups.size())
  {
    // delete every assignment to a tube
    for (auto it = m_TubeToLabelsMap.begin(); it != m_TubeToLabelsMap.end(); it++)
    {
      if (labelGroup->labelGroupName.compare((*it).first.second) == 0)
        m_TubeToLabelsMap.erase(it);
    }

    m_LabelGroups.erase(foundElement);
  }
  else
  {
    MITK_ERROR << "Could not find the label group!";
    return;
  }
}

void mitk::TubeGraphProperty::SetLabelColor(LabelGroup::Label *label, mitk::Color color)
{
  // LabelGroup? Check if it is a label in property class
  if (label)
  {
    label->labelColor = color;
    this->Modified();
  }
}

void mitk::TubeGraphProperty::SetLabelVisibility(LabelGroup::Label *label, bool isVisible)
{
  // LabelGroup? Check if it is a label in property class
  if (label)
  {
    label->isVisible = isVisible;
    this->Modified();
  }
}

void mitk::TubeGraphProperty::RenameLabel(LabelGroup *labelGroup, LabelGroup::Label *label, std::string newName)
{
  // LabelGroup? Check if it is a label in property class
  if (label)
  {
    // rename the label in the assignement vector for tubes
    for (auto it = m_TubeToLabelsMap.begin(); it != m_TubeToLabelsMap.end(); it++)
    {
      // Label group fit?
      if (labelGroup->labelGroupName.compare((*it).first.second) == 0)
        // label fit?
        if (label->labelName.compare((*it).second) == 0)
          // rename it
          (*it).second = newName;
    }

    // rename the label in label group
    label->labelName = newName;
  }
}

mitk::TubeGraphProperty::LabelGroupSetType mitk::TubeGraphProperty::GetLabelGroups()
{
  return m_LabelGroups;
}

unsigned int mitk::TubeGraphProperty::GetNumberOfLabelGroups()
{
  return m_LabelGroups.size();
}

unsigned int mitk::TubeGraphProperty::GetIndexOfLabelGroup(mitk::TubeGraphProperty::LabelGroup *labelGroup)
{
  unsigned int pos = std::find(m_LabelGroups.begin(), m_LabelGroups.end(), labelGroup) - m_LabelGroups.begin();

  if (pos < m_LabelGroups.size())
    return pos;
  // label group is not in property class
  else
  {
    MITK_ERROR << "Could not find such a label group!";
    return m_LabelGroups.size();
  }
}

mitk::TubeGraphProperty::LabelGroup *mitk::TubeGraphProperty::GetLabelGroupByName(std::string labelGroup)
{
  for (auto it = m_LabelGroups.begin(); it != m_LabelGroups.end(); it++)
  {
    if (labelGroup.compare((*it)->labelGroupName) == 0)
      return (*it);
  }

  MITK_ERROR << "Could not find such a label group!";
  return nullptr;
}

mitk::TubeGraphProperty::LabelGroup::Label *mitk::TubeGraphProperty::GetLabelByName(LabelGroup *labelGroup,
                                                                                    std::string labelName)
{
  for (auto it = labelGroup->labels.begin(); it != labelGroup->labels.end(); it++)
  {
    if (labelName.compare((*it)->labelName) == 0)
      return (*it);
  }

  MITK_ERROR << "Could not find such a label!";
  return nullptr;
}

bool mitk::TubeGraphProperty::TubeDescriptorsCompare(const TubeDescriptorType &tube1, const TubeDescriptorType &tube2)
{
  if (((tube1.first == tube2.first) && (tube1.second == tube2.second)) ||
      ((tube1.first == tube2.second) && (tube1.second == tube2.first)))
    return true;
  else
    return false;
}

bool mitk::TubeGraphProperty::IsEqual(const BaseProperty &) const
{ // TODO see ResectionProposalDescriptorProperty
  return true;
}

bool mitk::TubeGraphProperty::Assign(const BaseProperty &)
{ // TODO see ResectionProposalDescriptorProperty
  return true;
}

std::string mitk::TubeGraphProperty::GetValueAsString() const
{ // TODO
  return "";
}

itk::LightObject::Pointer mitk::TubeGraphProperty::InternalClone() const
{
  itk::LightObject::Pointer result(new Self(*this));
  return result;
}
