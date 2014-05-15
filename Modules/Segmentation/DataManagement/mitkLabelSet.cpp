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
  m_ActiveLabel = m_LabelContainer[other->GetActiveLabelPixelValue()];
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
  //LabelContainerType::iterator _end = m_LabelContainer.end();
  m_LabelContainer.clear();
}

void mitk::LabelSet::SetLabelVisible(int pixelValue, bool value)
{
  m_LabelContainer[pixelValue]->SetVisible(value);
  double rgba[4];
  m_LookupTable->GetTableValue(pixelValue,rgba);
  rgba[3] = value ? this->GetLabelOpacity(pixelValue) : 0.0;
  m_LookupTable->SetTableValue(pixelValue,rgba);
}

bool mitk::LabelSet::GetLabelVisible(int pixelValue)
{
  return m_LabelContainer[pixelValue]->GetVisible();
}

void mitk::LabelSet::SetLabelSelected(int pixelValue, bool value)
{
  m_LabelContainer[pixelValue]->SetSelected(value);
}

bool mitk::LabelSet::GetLabelSelected(int pixelValue)
{
  return m_LabelContainer[pixelValue]->GetSelected();
}

void mitk::LabelSet::SetLabelLocked(int pixelValue, bool value)
{
  m_LabelContainer[pixelValue]->SetLocked(value);
}

bool mitk::LabelSet::GetLabelLocked(int pixelValue)
{
  return m_LabelContainer[pixelValue]->GetLocked();
}

void mitk::LabelSet::SetLabelOpacity(int pixelValue, float value)
{
  double rgba[4];
  m_LookupTable->GetTableValue(pixelValue,rgba);
  rgba[3] = value;
  m_LookupTable->SetTableValue(pixelValue,rgba);
  m_LabelContainer[pixelValue]->SetOpacity(value);
}

float mitk::LabelSet::GetLabelOpacity(int pixelValue)
{
  return m_LabelContainer[pixelValue]->GetOpacity();
}

void mitk::LabelSet::SetLabelVolume(int pixelValue, float value)
{
  m_LabelContainer[pixelValue]->SetVolume(value);
}

float mitk::LabelSet::GetLabelVolume(int pixelValue)
{
  return m_LabelContainer[pixelValue]->GetVolume();
}

void mitk::LabelSet::SetLabelName(int pixelValue, const std::string& name)
{
  m_LabelContainer[pixelValue]->SetName(name);
}

const mitk::Color& mitk::LabelSet::GetLabelColor(int pixelValue)
{
  return m_LabelContainer[pixelValue]->GetColor();
}

void mitk::LabelSet::SetLabelColor(int pixelValue, const mitk::Color& color)
{
  m_LabelContainer[pixelValue]->SetColor(color);
  double rgba[4];
  m_LookupTable->GetTableValue(pixelValue,rgba);
  rgba[0] = color.GetRed();
  rgba[1] = color.GetGreen();
  rgba[2] = color.GetBlue();
  m_LookupTable->SetTableValue(pixelValue,rgba);
}

std::string mitk::LabelSet::GetLabelName(int pixelValue)
{
  return m_LabelContainer[pixelValue]->GetName();
}

void mitk::LabelSet::SetAllLabelsLocked(bool value)
{
  LabelContainerType::iterator _end = m_LabelContainer.end();
  LabelContainerType::iterator _it = m_LabelContainer.begin();
  _it++;
  for(; _it!=_end; ++_it)
    _it->second->SetLocked(value);
}

void mitk::LabelSet::SetAllLabelsVisible(bool value)
{
  LabelContainerType::iterator _end = m_LabelContainer.end();
  LabelContainerType::iterator _it = m_LabelContainer.begin();
  _it++; // skip first (exterior) label
  double rgba[4];
  for(; _it!=_end; ++_it)
  {
    _it->second->SetVisible(value);
    m_LookupTable->GetTableValue(_it->second->GetPixelValue(),rgba);
    rgba[3] = value ? _it->second->GetOpacity() : 0.0;
    m_LookupTable->SetTableValue(_it->second->GetPixelValue(),rgba);
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

void mitk::LabelSet::SetActiveLabel(int pixelValue)
{
  m_ActiveLabel = m_LabelContainer[pixelValue];
}

void mitk::LabelSet::RemoveLabel(int pixelValue)
{
  LabelContainerConstIteratorType it = m_LabelContainer.find(pixelValue);
  if(it != m_LabelContainer.end())
    m_LabelContainer.erase(it);
}

bool mitk::LabelSet::IsSelected(mitk::Label::Pointer label)
{
  return label->GetSelected();
}

bool mitk::LabelSet::ExistLabel(int pixelValue)
{
  return m_LabelContainer.count(pixelValue) > 0 ? true : false;
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
  if(label.GetPixelValue() == -1)
    newLabel->SetPixelValue( m_LabelContainer.size() );
  else
    newLabel->SetPixelValue( label.GetPixelValue()) ;

  newLabel->SetLayer( m_Layer );

  const mitk::Color& color = label.GetColor();
  double rgba[4];
  rgba[0] = color.GetRed();
  rgba[1] = color.GetGreen();
  rgba[2] = color.GetBlue();
  rgba[3] = label.GetOpacity();
  m_LookupTable->SetTableValue( m_LabelContainer.size(), rgba );

  m_LabelContainer[newLabel->GetPixelValue()] = newLabel;
  m_ActiveLabel = newLabel;
}

void mitk::LabelSet::AddLabel(const std::string& name, const mitk::Color& color )
{
  if (m_LabelContainer.size() > 255) return;

  mitk::Label::Pointer newLabel = mitk::Label::New();
  newLabel->SetName(name);
  newLabel->SetColor(color);
  AddLabel(*newLabel);
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
//  LabelContainerType::iterator _end = m_LabelContainer.end();
//  LabelContainerType::iterator _it = m_LabelContainer.begin();
//  int value = 0;
//  double rgba[4];
//  for (; _it!=_end; ++_it, ++value)
//  {
//    //_it->second->SetPixelValue(value);
//    _it->second->SetSelected(false);
//    const mitk::Color& color = _it->second->GetColor();
//    rgba[0] = color.GetRed();
//    rgba[1] = color.GetGreen();
//    rgba[2] = color.GetBlue();
//    rgba[3] = _it->second->GetOpacity();
//    m_LookupTable->SetTableValue(_it->second->GetPixelValue(),rgba);
//  }
}

int mitk::LabelSet::GetActiveLabelLayer() const
{
  return this->GetActiveLabel()->GetLayer();
}

int mitk::LabelSet::GetLabelLayer(unsigned int pixelValue) const
{
  LabelContainerConstIteratorType it = m_LabelContainer.find(pixelValue);
  return ( it != m_LabelContainer.end() )? it->second->GetLayer() : 0;
}

int mitk::LabelSet::GetActiveLabelPixelValue() const
{
  return this->GetActiveLabel()->GetPixelValue();
}

mitk::Label::ConstPointer mitk::LabelSet::GetLabel(unsigned int pixelValue) const
{
  LabelContainerConstIteratorType it = m_LabelContainer.find(pixelValue);
  return ( it != m_LabelContainer.end() )? it->second.GetPointer(): NULL;
}

void mitk::LabelSet::SetLabelCenterOfMassIndex(int pixelValue, const mitk::Point3D& center)
{
  m_LabelContainer[pixelValue]->SetCenterOfMassIndex(center);
}

const mitk::Point3D& mitk::LabelSet::GetLabelCenterOfMassIndex(int pixelValue)
{
  return m_LabelContainer[pixelValue]->GetCenterOfMassIndex();
}

void mitk::LabelSet::SetLabelCenterOfMassCoordinates(int pixelValue, const mitk::Point3D& center)
{
  m_LabelContainer[pixelValue]->SetCenterOfMassCoordinates(center);
}

const mitk::Point3D& mitk::LabelSet::GetLabelCenterOfMassCoordinates(int pixelValue)
{
  return m_LabelContainer[pixelValue]->GetCenterOfMassCoordinates();
}
