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
#include <itkCommand.h>

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
  SetActiveLabel(other->GetActiveLabel()->GetPixelValue());
}

mitk::LabelSet::~LabelSet()
{
  m_LabelContainer.clear();
}

void mitk::LabelSet::OnLabelModified()
{
  ModifyLabelEvent.Send();
  Superclass::Modified();
}

mitk::LabelSet::LabelContainerConstIteratorType mitk::LabelSet::IteratorEnd()
{
  return m_LabelContainer.end();
}

mitk::LabelSet::LabelContainerConstIteratorType mitk::LabelSet::IteratorBegin()
{
  return m_LabelContainer.begin();
}

int mitk::LabelSet::GetNumberOfLabels() const
{
  return m_LabelContainer.size();
}

void mitk::LabelSet::SetLayer(int layer)
{
  m_Layer = layer;
  Modified();
}

void mitk::LabelSet::SetActiveLabel(int pixelValue)
{
  m_ActiveLabel = m_LabelContainer[pixelValue];
  ActiveLabelEvent.Send(pixelValue);
  Modified();
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
  newLabel->SetName( label.GetName() );
  newLabel->SetVisible( label.GetVisible() );
  newLabel->SetLocked( label.GetLocked() );
  newLabel->SetOpacity( label.GetOpacity() );
  newLabel->SetLayer( m_Layer );

  unsigned int pixelValue = -1;
  if(label.GetPixelValue() == -1){
    //find next free
    int i = 1;
    while(m_LabelContainer.empty() == false && m_LabelContainer.find(i) != m_LabelContainer.end() )
      i++;
    newLabel->SetPixelValue( i );
    pixelValue = i;
    MITK_INFO << "Label with corresponding pixelvalue of "  << i << " added to active label set." ;
  }
  else
  {
    newLabel->SetPixelValue( label.GetPixelValue());
    pixelValue = label.GetPixelValue();
  }


  // new map entry
  m_LabelContainer[pixelValue] = newLabel;
  UpdateLookupTable(pixelValue);

  itk::SimpleMemberCommand<LabelSet>::Pointer command = itk::SimpleMemberCommand<LabelSet>::New();
  command->SetCallbackFunction(this, &LabelSet::OnLabelModified);
  newLabel->AddObserver( itk::ModifiedEvent(), command );
  //newLabel->AddObserver(itk::ModifiedEvent(),command);

  SetActiveLabel(newLabel->GetPixelValue());
  AddLabelEvent.Send();
  Modified();
}

void mitk::LabelSet::AddLabel(const std::string& name, const mitk::Color& color )
{
  if (m_LabelContainer.size() > 255) return;

  mitk::Label::Pointer newLabel = mitk::Label::New();
  newLabel->SetName(name);
  newLabel->SetColor(color);
  AddLabel(*newLabel);
}

void mitk::LabelSet::RenameLabel(int pixelValue, const std::string& name, const mitk::Color& color)
{
  mitk::Label* label = GetLabel(pixelValue);
  label->SetName(name);
  label->SetColor(color);
}

mitk::LookupTable* mitk::LabelSet::GetLookupTable()
{
  return m_LookupTable.GetPointer();
}

void mitk::LabelSet::SetLookupTable( mitk::LookupTable* lut)
{
  m_LookupTable = lut;
  Modified();
}

void mitk::LabelSet::PrintSelf(std::ostream &os, itk::Indent indent) const
{
}

void mitk::LabelSet::RemoveLabel(int pixelValue)
{
  LabelContainerType::reverse_iterator it = m_LabelContainer.rbegin();
  int nextActivePixelValue = it->first;

  for(; it != m_LabelContainer.rend(); it++){
    if(it->first == pixelValue)
    {
      it->second->RemoveAllObservers();
      m_LabelContainer.erase(pixelValue);
      break;
    }
    nextActivePixelValue = it->first;
  }

  RemoveLabelEvent.Send();

  if(ExistLabel(nextActivePixelValue))
    SetActiveLabel(nextActivePixelValue);
  else
    SetActiveLabel(m_LabelContainer.rbegin()->first);

  Modified();
}

void mitk::LabelSet::RemoveAllLabels()
{
  LabelContainerConstIteratorType _it = IteratorBegin();
  for(;_it!=IteratorEnd();_it++)
  {
    RemoveLabelEvent.Send();
    m_LabelContainer.erase(_it);
  }
  AllLabelsModifiedEvent.Send();
}

void mitk::LabelSet::SetAllLabelsLocked(bool value)
{
  LabelContainerType::iterator _end = m_LabelContainer.end();
  LabelContainerType::iterator _it = m_LabelContainer.begin();
  _it++;
  for(; _it!=_end; ++_it)
    _it->second->SetLocked(value);
  AllLabelsModifiedEvent.Send();
  Modified();
}

void mitk::LabelSet::SetAllLabelsVisible(bool value)
{
  LabelContainerType::iterator _end = m_LabelContainer.end();
  LabelContainerType::iterator _it = m_LabelContainer.begin();
  _it++;
  double rgba[4];
  for(; _it!=_end; ++_it)
  {
    _it->second->SetVisible(value);
    UpdateLookupTable(_it->first);
  }
  AllLabelsModifiedEvent.Send();
  Modified();
}

void mitk::LabelSet::UpdateLookupTable(int pixelValue)
{
  const mitk::Color& color = GetLabel(pixelValue)->GetColor();

  double rgba[4];
  m_LookupTable->GetTableValue(pixelValue,rgba);
  rgba[0] = color.GetRed();
  rgba[1] = color.GetGreen();
  rgba[2] = color.GetBlue();
  if(GetLabel(pixelValue)->GetVisible())
    rgba[3] = GetLabel(pixelValue)->GetOpacity();
  else
    rgba[3] = 0.0;
  m_LookupTable->SetTableValue(pixelValue,rgba);
}


const mitk::Label * mitk::LabelSet::GetLabel(int pixelValue) const
{
  LabelContainerConstIteratorType it = m_LabelContainer.find(pixelValue);
  return ( it != m_LabelContainer.end() )? it->second.GetPointer(): NULL;
}
