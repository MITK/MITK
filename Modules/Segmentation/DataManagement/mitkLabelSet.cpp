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

#include "itkProcessObject.h"
#include <algorithm>


mitk::LabelSet::LabelSet() :
m_Layer(0),
m_ActiveLabel(NULL)
{
  m_LookupTable = mitk::LookupTable::New();
  m_LookupTable->SetType(mitk::LookupTable::MULTILABEL);
}

void mitk::LabelSet::Initialize(const LabelSet* other)
{
  if (!other)
    mitkThrow() << "Trying to initialize with NULL input.";
  m_LabelContainer = other->m_LabelContainer;
  m_ActiveLabel = m_LabelContainer[other->GetActiveLabelIndex()];
}

mitk::LabelSet::~LabelSet()
{
  m_LabelContainer.clear();
}

mitk::LabelSet::LabelContainerConstIteratorType mitk::LabelSet::IteratorEnd()
{
  return m_LabelContainer.end();
}

mitk::LabelSet::LabelContainerConstIteratorType mitk::LabelSet::IteratorBegin()
{
  return m_LabelContainer.begin();
}

void mitk::LabelSet::RemoveAllLabels()
{
  LabelContainerType::iterator _end = m_LabelContainer.end();
  m_LabelContainer.erase(m_LabelContainer.begin()+1,_end);
}

void mitk::LabelSet::SetLabelVisible(int index, bool value)
{
  m_LabelContainer[index]->SetVisible(value);
  double rgba[4];
  m_LookupTable->GetTableValue(index,rgba);
  rgba[3] = value ? this->GetLabelOpacity(index) : 0.0;
  m_LookupTable->SetTableValue(index,rgba);
}

bool mitk::LabelSet::GetLabelVisible(int index)
{
  return m_LabelContainer[index]->GetVisible();
}

void mitk::LabelSet::SetLabelSelected(int index, bool value)
{
  m_LabelContainer[index]->SetSelected(value);
}

bool mitk::LabelSet::GetLabelSelected(int index)
{
  return m_LabelContainer[index]->GetSelected();
}

void mitk::LabelSet::SetLabelLocked(int index, bool value)
{
  m_LabelContainer[index]->SetLocked(value);
}

bool mitk::LabelSet::GetLabelLocked(int index)
{
  return m_LabelContainer[index]->GetLocked();
}

void mitk::LabelSet::SetLabelOpacity(int index, float value)
{
  double rgba[4];
  m_LookupTable->GetTableValue(index,rgba);
  rgba[3] = value;
  m_LookupTable->SetTableValue(index,rgba);
  m_LabelContainer[index]->SetOpacity(value);
}

float mitk::LabelSet::GetLabelOpacity(int index)
{
  return m_LabelContainer[index]->GetOpacity();
}

void mitk::LabelSet::SetLabelVolume(int index, float value)
{
  m_LabelContainer[index]->SetVolume(value);
}

float mitk::LabelSet::GetLabelVolume(int index)
{
  return m_LabelContainer[index]->GetVolume();
}

void mitk::LabelSet::SetLabelName(int index, const std::string& name)
{
  m_LabelContainer[index]->SetName(name);
}

const mitk::Color& mitk::LabelSet::GetLabelColor(int index)
{
  return m_LabelContainer[index]->GetColor();
}

void mitk::LabelSet::SetLabelColor(int index, const mitk::Color& color)
{
  m_LabelContainer[index]->SetColor(color);
  double rgba[4];
  m_LookupTable->GetTableValue(index,rgba);
  rgba[0] = color.GetRed();
  rgba[1] = color.GetGreen();
  rgba[2] = color.GetBlue();
  m_LookupTable->SetTableValue(index,rgba);
}

std::string mitk::LabelSet::GetLabelName(int index)
{
  return m_LabelContainer[index]->GetName();
}

void mitk::LabelSet::SetAllLabelsLocked(bool value)
{
  LabelContainerType::iterator _end = m_LabelContainer.end();
  LabelContainerType::iterator _it = m_LabelContainer.begin();
  _it++;
  for(; _it!=_end; ++_it)
    (*_it)->SetLocked(value);
}

void mitk::LabelSet::SetAllLabelsVisible(bool value)
{
  LabelContainerType::iterator _end = m_LabelContainer.end();
  LabelContainerType::iterator _it = m_LabelContainer.begin();
  _it++; // skip first (exterior) label
  double rgba[4];
  for(; _it!=_end; ++_it)
  {
    (*_it)->SetVisible(value);
    m_LookupTable->GetTableValue((*_it)->GetIndex(),rgba);
    rgba[3] = value ? (*_it)->GetOpacity() : 0.0;
    m_LookupTable->SetTableValue((*_it)->GetIndex(),rgba);
  }
}

int mitk::LabelSet::GetNumberOfLabels() const
{
  return m_LabelContainer.size();
}

void mitk::LabelSet::SetLayer(int layer)
{
  m_Layer = layer;
}

void mitk::LabelSet::SetActiveLabel(int index)
{
  m_ActiveLabel = m_LabelContainer[index];
}

void mitk::LabelSet::RemoveLabel(int index)
{
  m_LabelContainer.erase(m_LabelContainer.begin()+index);
}

bool mitk::LabelSet::IsSelected(mitk::Label::Pointer label)
{
  return label->GetSelected();
}

void mitk::LabelSet::RemoveSelectedLabels()
{
  mitk::LabelSet::LabelContainerIteratorType _begin = m_LabelContainer.begin();
  mitk::LabelSet::LabelContainerIteratorType _end = m_LabelContainer.end();
  m_LabelContainer.erase(std::remove_if( _begin, _end, &mitk::LabelSet::IsSelected ), _end);
  this->m_ActiveLabel = m_LabelContainer.front();
}

void mitk::LabelSet::RemoveLabels(int begin, int count)
{
  m_LabelContainer.erase(m_LabelContainer.begin()+begin, m_LabelContainer.begin()+begin+count);
}

void mitk::LabelSet::AddLabel(const mitk::Label& label )
{
  if (m_LabelContainer.size() > 255) return;

  mitk::Label::Pointer newLabel = mitk::Label::New();
  newLabel->SetColor( label.GetColor() );
  newLabel->SetExterior( label.GetExterior() );
  newLabel->SetName( label.GetName() );
  newLabel->SetVisible( label.GetVisible() );
  newLabel->SetLocked( label.GetLocked() );
  newLabel->SetOpacity( label.GetOpacity() );
  newLabel->SetVolume( label.GetVolume() );
  newLabel->SetIndex( m_LabelContainer.size() );
  newLabel->SetLayer( m_Layer );

  const mitk::Color& color = label.GetColor();
  double rgba[4];
  rgba[0] = color.GetRed();
  rgba[1] = color.GetGreen();
  rgba[2] = color.GetBlue();
  rgba[3] = label.GetOpacity();
  m_LookupTable->SetTableValue( m_LabelContainer.size(), rgba );

  m_LabelContainer.push_back(newLabel);
  m_ActiveLabel = newLabel;
}

void mitk::LabelSet::AddLabel(const std::string& name, const mitk::Color& color )
{
  if (m_LabelContainer.size() > 255) return;

  mitk::Label::Pointer newLabel = mitk::Label::New();
  newLabel->SetName(name);
  newLabel->SetColor(color);
  newLabel->SetOpacity(0.4);
  newLabel->SetVolume(0.0);
  newLabel->SetVisible(true);
  newLabel->SetLocked(false);
  newLabel->SetIndex( m_LabelContainer.size());
  newLabel->SetLayer( m_Layer );
  m_LabelContainer.push_back(newLabel);

  double rgba[4];
  rgba[0] = color.GetRed();
  rgba[1] = color.GetGreen();
  rgba[2] = color.GetBlue();
  rgba[3] = newLabel->GetOpacity();
  m_LookupTable->SetTableValue( newLabel->GetIndex(), rgba );

  m_ActiveLabel = newLabel;
}

void mitk::LabelSet::RenameLabel(int index, const std::string& name, const mitk::Color& color)
{
  mitk::Label* label = m_LabelContainer[index];
  label->SetName(name);
  label->SetColor(color);
}

void mitk::LabelSet::PrintSelf(std::ostream &os, itk::Indent indent) const
{
 // os << indent;
 // todo: complete
}

mitk::LookupTable* mitk::LabelSet::GetLookupTable()
{
  return m_LookupTable.GetPointer();
}

void mitk::LabelSet::SetLookupTable( mitk::LookupTable* lut)
{
  m_LookupTable = lut;
}

void mitk::LabelSet::ResetLabels()
{
  LabelContainerType::iterator _end = m_LabelContainer.end();
  LabelContainerType::iterator _it = m_LabelContainer.begin();
  int value = 0;
  double rgba[4];
  for (; _it!=_end; ++_it, ++value)
  {
    (*_it)->SetIndex(value);
    (*_it)->SetSelected(false);
    const mitk::Color& color = (*_it)->GetColor();
    rgba[0] = color.GetRed();
    rgba[1] = color.GetGreen();
    rgba[2] = color.GetBlue();
    rgba[3] = (*_it)->GetOpacity();
    m_LookupTable->SetTableValue((*_it)->GetIndex(),rgba);
  }
}

int mitk::LabelSet::GetActiveLabelLayer() const
{
  return this->GetActiveLabel()->GetLayer();
}

int mitk::LabelSet::GetLabelLayer(int index) const
{
  return m_LabelContainer[index]->GetLayer();
}

int mitk::LabelSet::GetActiveLabelIndex() const
{
  return this->GetActiveLabel()->GetIndex();
}

mitk::Label::ConstPointer mitk::LabelSet::GetLabel(int index) const
{
  return m_LabelContainer[index].GetPointer();
}

void mitk::LabelSet::SetLabelCenterOfMassIndex(int index, const mitk::Point3D& center)
{
  m_LabelContainer[index]->SetCenterOfMassIndex(center);
}

const mitk::Point3D& mitk::LabelSet::GetLabelCenterOfMassIndex(int index)
{
  return m_LabelContainer[index]->GetCenterOfMassIndex();
}

void mitk::LabelSet::SetLabelCenterOfMassCoordinates(int index, const mitk::Point3D& center)
{
  m_LabelContainer[index]->SetCenterOfMassCoordinates(center);
}

const mitk::Point3D& mitk::LabelSet::GetLabelCenterOfMassCoordinates(int index)
{
  return m_LabelContainer[index]->GetCenterOfMassCoordinates();
}
