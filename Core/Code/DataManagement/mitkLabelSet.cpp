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
#include "mitkColormapProperty.h"

#include "itkProcessObject.h"
#include <algorithm>


mitk::LabelSet::LabelSet(int layer) :
m_Owner(""),
m_Name(""),
m_LastModified(""),
m_Layer(layer),
m_ActiveLabel(NULL)
{
  m_LookupTable = mitk::LookupTable::New();
  m_LookupTable->SetActiveColormap(mitk::ColormapProperty::CM_MULTILABEL);
}
/*
mitk::LabelSet::LabelSet(const mitk::LabelSet& other) :
m_Name(other.m_Name),
m_LastModified(other.m_LastModified),
m_Owner(other.m_Owner),
m_LabelContainer(other.m_LabelContainer)
{
  this->m_ActiveLabel = m_LabelContainer[other.GetActiveLabelIndex()];
  this->m_LookupTable = other.GetLookupTable();
}
*/
void mitk::LabelSet::Initialize(const LabelSet* other)
{
  if (!other) return;
  m_Name = other->m_Name;
  m_LastModified = other->m_LastModified;
  m_Owner = other->m_Owner;
  m_LabelContainer = other->m_LabelContainer;
  m_ActiveLabel = m_LabelContainer[other->GetActiveLabelIndex()];
}

itk::LightObject::Pointer
mitk::LabelSet::InternalClone() const
{
  Self::Pointer newLabelSet = new LabelSet(*this);
  newLabelSet->UnRegister();
  return newLabelSet.GetPointer();
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

bool mitk::LabelSet::HasLabel(int index) const
{
  return ( (index >=0) && (index<m_LabelContainer.size()) );
}

void mitk::LabelSet::RemoveAllLabels()
{
  LabelContainerType::iterator _end = m_LabelContainer.end();
  m_LabelContainer.erase(m_LabelContainer.begin()+1,_end);
}

void mitk::LabelSet::SetLabelVisible(int index, bool value)
{
  if (this->HasLabel(index))
  {
    m_LabelContainer[index]->SetVisible(value);
  }
}

bool mitk::LabelSet::GetLabelVisible(int index)
{
  return m_LabelContainer[index]->GetVisible();
}

void mitk::LabelSet::SetLabelSelected(int index, bool value)
{
  if (this->HasLabel(index))
  {
    m_LabelContainer[index]->SetSelected(value);
  }
}

bool mitk::LabelSet::GetLabelSelected(int index)
{
  if (this->HasLabel(index))
    return m_LabelContainer[index]->GetSelected();
  else
    mitkThrow() << "wrong label index";
}

void mitk::LabelSet::SetLabelLocked(int index, bool value)
{
  if (this->HasLabel(index))
    m_LabelContainer[index]->SetLocked(value);
  else
    mitkThrow() << "wrong label index";
}

bool mitk::LabelSet::GetLabelLocked(int index)
{
  if (this->HasLabel(index))
    return m_LabelContainer[index]->GetLocked();
  else
    mitkThrow() << "wrong label index";
}

void mitk::LabelSet::SetLabelOpacity(int index, float value)
{
  if (this->HasLabel(index))
    m_LabelContainer[index]->SetOpacity(value);
  else
    mitkThrow() << "wrong label index";
}

float mitk::LabelSet::GetLabelOpacity(int index)
{
  if (this->HasLabel(index))
    return m_LabelContainer[index]->GetOpacity();
  else
    mitkThrow() << "wrong label index";
}

void mitk::LabelSet::SetLabelVolume(int index, float value)
{
  if (this->HasLabel(index))
    m_LabelContainer[index]->SetVolume(value);
  else
    mitkThrow() << "wrong label index";
}

float mitk::LabelSet::GetLabelVolume(int index)
{
  if (this->HasLabel(index))
    return m_LabelContainer[index]->GetVolume();
  else
    mitkThrow() << "wrong label index";
}

void mitk::LabelSet::SetLabelName(int index, const std::string& name)
{
  if (this->HasLabel(index))
    m_LabelContainer[index]->SetName(name);
  else
    mitkThrow() << "wrong label index";
}

const mitk::Color& mitk::LabelSet::GetLabelColor(int index)
{
  if (this->HasLabel(index))
    return m_LabelContainer[index]->GetColor();
  else
    mitkThrow() << "wrong label index";
}

void mitk::LabelSet::SetLabelColor(int index, const mitk::Color& color)
{
  if (this->HasLabel(index))
    m_LabelContainer[index]->SetColor(color);
  else
    mitkThrow() << "wrong label index";
}

std::string mitk::LabelSet::GetLabelName(int index)
{
  if (this->HasLabel(index))
    return m_LabelContainer[index]->GetName();
  else
    mitkThrow() << "wrong label index";
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
  _it++;
  for(; _it!=_end; ++_it)
    (*_it)->SetVisible(value);
}

int mitk::LabelSet::GetNumberOfLabels() const
{
  return m_LabelContainer.size();
}

void mitk::LabelSet::SetActiveLabel(int index)
{
  if (this->HasLabel(index))
    m_ActiveLabel = m_LabelContainer[index];
  else
    mitkThrow() << "wrong label index";
}

void mitk::LabelSet::RemoveLabel(int index)
{
  if (this->HasLabel(index))
    m_LabelContainer.erase(m_LabelContainer.begin()+index);
  else
    mitkThrow() << "wrong label index";
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
  m_LabelContainer.push_back(newLabel);
  m_ActiveLabel = newLabel;
}

void mitk::LabelSet::AddLabel(const std::string& name, const mitk::Color& color )
{
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
  m_ActiveLabel = newLabel;
}

void mitk::LabelSet::RenameLabel(int index, const std::string& name, const mitk::Color& color)
{
  if (this->HasLabel(index))
  {
    mitk::Label* label = m_LabelContainer[index];
    label->SetName(name);
    label->SetColor(color);
  }
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
  for (; _it!=_end; ++_it, ++value)
  {
    (*_it)->SetIndex(value);
    (*_it)->SetSelected(false);
  }
}

int mitk::LabelSet::GetActiveLabelLayer() const
{
  return this->GetActiveLabel()->GetLayer();
}

unsigned int mitk::LabelSet::GetLabelLayer(int index) const
{
  if (this->HasLabel(index))
    return m_LabelContainer[index]->GetLayer();
  else
    return 0;
}

int mitk::LabelSet::GetActiveLabelIndex() const
{
  return this->GetActiveLabel()->GetIndex();
}

mitk::Label::ConstPointer mitk::LabelSet::GetLabel(int index) const
{
  if (this->HasLabel(index))
    return m_LabelContainer[index].GetPointer();
  else
    return NULL;
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
