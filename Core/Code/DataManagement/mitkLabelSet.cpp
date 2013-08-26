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


mitk::LabelSet::LabelSet(): m_Owner(""), m_LastModified("00.00.00")
{

}

mitk::LabelSet::LabelSet(const LabelSet& other) : itk::Object()
{
    this->m_LabelContainer = other.m_LabelContainer;
    this->m_ActiveLabel = this->m_LabelContainer[other.GetActiveLabelIndex()];
    this->m_Owner = other.m_Owner;
    this->m_LastModified  = other.m_LastModified;
    this->m_Name = other.m_Name;
}

bool mitk::LabelSet::operator==( const mitk::LabelSet& other ) const
{
  // todo
  return true;
}

bool mitk::LabelSet::operator!=( const mitk::LabelSet& other ) const
{
  return !(*this == other);
}

mitk::LabelSet& mitk::LabelSet::operator=( const mitk::LabelSet& other )
{
  if ( this == &other )
  {
    return * this;
  }
  else
  {
    this->m_LabelContainer = other.m_LabelContainer;
    this->m_ActiveLabel = this->m_LabelContainer[other.GetActiveLabelIndex()];
    this->m_Owner = other.m_Owner;
    this->m_LastModified  = other.m_LastModified;
    this->m_Name = other.m_Name;

    return *this;
  }
}

mitk::LabelSet::~LabelSet()
{
    m_LabelContainer.clear();
}

mitk::LabelSet::LabelContainerConstIteratorType mitk::LabelSet::IteratorEnd()
{
    return this->m_LabelContainer.end();
}

mitk::LabelSet::LabelContainerConstIteratorType mitk::LabelSet::IteratorBegin()
{
    return this->m_LabelContainer.begin();
}

bool mitk::LabelSet::HasLabel(int index) const
{
    return ( (index >=0) && (index<this->m_LabelContainer.size()) );
}

void mitk::LabelSet::RemoveAllLabels()
{
    LabelContainerType::iterator _end = this->m_LabelContainer.end();
    this->m_LabelContainer.erase(this->m_LabelContainer.begin()+1,_end);
}

void mitk::LabelSet::SetLabelVisible(int index, bool value)
{
    if (this->HasLabel(index))
    {
        this->m_LabelContainer[index]->SetVisible(value);
    }
}

bool mitk::LabelSet::GetLabelVisible(int index)
{
    return this->m_LabelContainer[index]->GetVisible();
}

void mitk::LabelSet::SetLabelSelected(int index, bool value)
{
    if (this->HasLabel(index))
    {
        this->m_LabelContainer[index]->SetSelected(value);
    }
}

bool mitk::LabelSet::GetLabelSelected(int index)
{
    return this->m_LabelContainer[index]->GetSelected();
}

void mitk::LabelSet::SetLabelLocked(int index, bool value)
{
    if (this->HasLabel(index))
    {
        this->m_LabelContainer[index]->SetLocked(value);
    }
}

bool mitk::LabelSet::GetLabelLocked(int index)
{
    return this->m_LabelContainer[index]->GetLocked();
}

void mitk::LabelSet::SetLabelOpacity(int index, float value)
{
    if (this->HasLabel(index))
    {
        this->m_LabelContainer[index]->SetOpacity(value);
    }
}

float mitk::LabelSet::GetLabelOpacity(int index)
{
    return this->m_LabelContainer[index]->GetOpacity();
}

void mitk::LabelSet::SetLabelVolume(int index, float value)
{
    if (this->HasLabel(index))
    {
        this->m_LabelContainer[index]->SetVolume(value);
    }
}

float mitk::LabelSet::GetLabelVolume(int index)
{
    return this->m_LabelContainer[index]->GetVolume();
}

void mitk::LabelSet::SetLabelName(int index, const std::string& name)
{
    if (this->HasLabel(index))
    {
        this->m_LabelContainer[index]->SetName(name);
    }
}

const mitk::Color& mitk::LabelSet::GetLabelColor(int index)
{
    return this->m_LabelContainer[index]->GetColor();
}

void mitk::LabelSet::SetLabelColor(int index, const mitk::Color& color)
{
    if (this->HasLabel(index))
    {
        this->m_LabelContainer[index]->SetColor(color);
    }
}

std::string mitk::LabelSet::GetLabelName(int index)
{
    return this->m_LabelContainer[index]->GetName();
}

void mitk::LabelSet::SetAllLabelsLocked(bool value)
{
    LabelContainerType::iterator _end = this->m_LabelContainer.end();
    LabelContainerType::iterator _it = this->m_LabelContainer.begin();
    _it++;
    for(; _it!=_end; ++_it)
    {
        (*_it)->SetLocked(value);
    }
}

void mitk::LabelSet::SetAllLabelsVisible(bool value)
{
    LabelContainerType::iterator _end = this->m_LabelContainer.end();
    LabelContainerType::iterator _it = this->m_LabelContainer.begin();
    _it++;
    for(; _it!=_end; ++_it)
    {
        (*_it)->SetVisible(value);
    }
}

int mitk::LabelSet::GetNumberOfLabels() const
{
    return this->m_LabelContainer.size();
}

void mitk::LabelSet::SetActiveLabel(int index)
{
    if (this->HasLabel(index))
        this->m_ActiveLabel = this->m_LabelContainer[index];
}

void mitk::LabelSet::RemoveLabel(int index)
{
    this->m_LabelContainer.erase(this->m_LabelContainer.begin()+index);
}

bool mitk::LabelSet::IsSelected(mitk::Label::Pointer label)
{
    return label->GetSelected();
}

void mitk::LabelSet::RemoveSelectedLabels()
{
    mitk::LabelSet::LabelContainerIteratorType _begin = this->m_LabelContainer.begin();
    mitk::LabelSet::LabelContainerIteratorType _end = this->m_LabelContainer.end();
    this->m_LabelContainer.erase(std::remove_if( _begin, _end, &mitk::LabelSet::IsSelected ), _end);

    this->m_ActiveLabel = this->m_LabelContainer.front();
}

void mitk::LabelSet::RemoveLabels(int begin, int count)
{
    this->m_LabelContainer.erase(this->m_LabelContainer.begin()+begin, this->m_LabelContainer.begin()+begin+count);
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
    newLabel->SetIndex( this->m_LabelContainer.size() );
    this->m_LabelContainer.push_back(newLabel);
    this->m_ActiveLabel = newLabel;
}

void mitk::LabelSet::AddLabel(const std::string& name, const mitk::Color& color )
{
    mitk::Label::Pointer label = mitk::Label::New();
    label->SetName(name);
    label->SetColor(color);
    label->SetOpacity(0.4);
    label->SetVolume(0.0);
    label->SetVisible(true);
    label->SetLocked(false);
    label->SetIndex( this->m_LabelContainer.size());
    this->m_LabelContainer.push_back(label);
    this->m_ActiveLabel = label;
}

void mitk::LabelSet::RenameLabel(int index, const std::string& name, const mitk::Color& color)
{
    if (this->HasLabel(index))
    {
        mitk::Label* label = this->m_LabelContainer[index];
        label->SetName(name);
        label->SetColor(color);
    }
}

void mitk::LabelSet::PrintSelf(std::ostream &os, itk::Indent indent) const
{
 // os << indent;
 // todo: complete
}

void mitk::LabelSet::ResetLabels()
{
    LabelContainerType::iterator _end = this->m_LabelContainer.end();
    LabelContainerType::iterator _it = this->m_LabelContainer.begin();
    int value = 0;
    for (; _it!=_end; ++_it, ++value)
    {
        (*_it)->SetIndex(value);
        (*_it)->SetSelected(false);
    }
}

int mitk::LabelSet::GetActiveLabelIndex() const
{
    return this->GetActiveLabel()->GetIndex();
}

mitk::Label::ConstPointer mitk::LabelSet::GetLabel(int index) const
{
  if (this->HasLabel(index))
    return this->m_LabelContainer[index].GetPointer();
  else
    return NULL;
}

void mitk::LabelSet::SetLabelCenterOfMassIndex(int index, const mitk::Point3D& center)
{
  this->m_LabelContainer[index]->SetCenterOfMassIndex(center);
}

const mitk::Point3D& mitk::LabelSet::GetLabelCenterOfMassIndex(int index)
{
  return this->m_LabelContainer[index]->GetCenterOfMassIndex();
}

void mitk::LabelSet::SetLabelCenterOfMassCoordinates(int index, const mitk::Point3D& center)
{
  this->m_LabelContainer[index]->SetCenterOfMassCoordinates(center);
}

const mitk::Point3D& mitk::LabelSet::GetLabelCenterOfMassCoordinates(int index)
{
  return this->m_LabelContainer[index]->GetCenterOfMassCoordinates();
}
