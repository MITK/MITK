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

#include "mitkLabelSetImage.h"

#include "mitkImageAccessByItk.h"
#include "mitkInteractionConst.h"
#include "mitkRenderingManager.h"
#include "mitkColormapProperty.h"
#include "mitkImageCast.h"

#include "mitkLookupTableProperty.h"

#include "itkImageRegionIterator.h"

mitk::LabelSetImage::LabelSetImage()
{
    m_LabelSet = mitk::LabelSet::New();
    mitk::Color color;
    color.SetRed(0.0);
    color.SetGreen(0.0);
    color.SetBlue(0.0);
    mitk::Label::Pointer label = mitk::Label::New();
    label->SetColor(color);
    label->SetName("exterior");
    label->SetExterior(true);
    label->SetOpacity(0.0);
    label->SetLocked(false);
    m_LabelSet->AddLabel(*label);

    mitk::LookupTable::Pointer lut = mitk::LookupTable::New();
    lut->SetActiveColormap(mitk::ColormapProperty::CM_MULTILABEL);

    mitk::LookupTableProperty::Pointer lutProp = mitk::LookupTableProperty::New();
    lutProp->SetLookupTable(lut);

    this->GetPropertyList()->SetProperty( "LookupTable", lutProp );
}

mitk::LabelSetImage::~LabelSetImage()
{
}

void mitk::LabelSetImage::ExecuteOperation(mitk::Operation* operation)
{
// todo
}

void mitk::LabelSetImage::SetLabelSetName(const std::string& name)
{
    this->m_LabelSet->SetName(name);
}

std::string mitk::LabelSetImage::GetLabelSetName()
{
    return this->m_LabelSet->GetName();
}

void mitk::LabelSetImage::SetLabelSetLastModified(const std::string& name)
{
    this->m_LabelSet->SetLastModified(name);
}

std::string mitk::LabelSetImage::GetLabelSetLastModified()
{
    return this->m_LabelSet->GetLastModified();
}

void mitk::LabelSetImage::MergeLabels(int begin, int count, int index)
{
    this->SetActiveLabel( index, false );
    for (int idx = begin; idx<begin+count; ++idx)
    {
        if (idx != index)
        {
            AccessByItk_1(this, MergeLabelsProcessing, index);
        }
    }

   // this->RemoveLabels( begin, count );
    this->Modified();
}

void mitk::LabelSetImage::CalculateLabelVolume(int index)
{
// todo
}

void mitk::LabelSetImage::EraseLabel(int index, bool reorder)
{
  AccessByItk_2(this, EraseLabelProcessing, index, reorder);
  this->Modified();
}

bool mitk::LabelSetImage::Concatenate(mitk::LabelSetImage* other)
{
    const unsigned int* otherDims = other->GetDimensions();
    const unsigned int* thisDims = this->GetDimensions();
    if ( (otherDims[0] != thisDims[0]) || (otherDims[1] != thisDims[1]) || (otherDims[2] != thisDims[2]) )
        return false;

    AccessByItk_1(this, ConcatenateProcessing, other);

    const mitk::LabelSet* ls = other->GetLabelSet();
    for( int i=1; i<ls->GetNumberOfLabels(); i++) // skip exterior
    {
        this->AddLabel( *ls->GetLabel(i) );
    }

    this->Modified();

    return true;
}

void mitk::LabelSetImage::ClearBuffer()
{
  AccessByItk(this, ClearBufferProcessing);
  this->Modified();
}

template < typename LabelSetImageType >
void mitk::LabelSetImage::CenterOfMassProcessing(LabelSetImageType* itkImage, mitk::Point3D& pos)
{
   int activeLabel = this->GetActiveLabelIndex();

   typename typedef itk::ImageRegionConstIteratorWithIndex< LabelSetImageType > IteratorType;
   IteratorType iter( itkImage, itkImage->GetLargestPossibleRegion() );
   iter.GoToBegin();

   typename std::vector< LabelSetImageType::IndexType > indexVector;

   while ( !iter.IsAtEnd() )
   {
    if ( iter.Get() == static_cast<int>(activeLabel) )
    {

      indexVector.push_back(iter.GetIndex());
    }
    ++iter;
   }

   typename itk::ImageRegionConstIteratorWithIndex< LabelSetImageType >::IndexType index;

   index = indexVector.at(indexVector.size()/2);
   pos[0] = index[0];
   pos[1] = index[1];
   pos[2] = index[2];
   this->GetSlicedGeometry()->IndexToWorld(pos, pos);
}

template < typename LabelSetImageType >
void mitk::LabelSetImage::ClearBufferProcessing(LabelSetImageType* itkImage)
{
  itkImage->FillBuffer(0);
}

//AccessTwoImagesFixedDimensionByItk
template < typename LabelSetImageType >
void mitk::LabelSetImage::ConcatenateProcessing(LabelSetImageType* itkTarget, mitk::LabelSetImage* other)
{
//    typedef itk::Image< unsigned char, 3 > ImageType;
    typename LabelSetImageType::Pointer itkSource = LabelSetImageType::New();
    mitk::CastToItkImage( other, itkSource );

    typedef itk::ImageRegionConstIterator< LabelSetImageType > ConstIteratorType;
    typedef itk::ImageRegionIterator< LabelSetImageType >      IteratorType;

    ConstIteratorType sourceIter( itkSource, itkSource->GetLargestPossibleRegion() );
    IteratorType      targetIter( itkTarget, itkTarget->GetLargestPossibleRegion() );

    int numberOfTargetLabels = this->GetNumberOfLabels() - 1; // skip exterior
    sourceIter.GoToBegin();
    targetIter.GoToBegin();

    while (!sourceIter.IsAtEnd())
    {
        int sourceValue = static_cast<int>(sourceIter.Get());
        int targetValue =  static_cast<int>(targetIter.Get());
        if ( (sourceValue != 0) && !this->GetLabelLocked(targetValue) ) // skip exterior and locked labels
        {
            targetIter.Set( sourceValue + numberOfTargetLabels );
        }
        ++sourceIter;
        ++targetIter;
    }
}

template < typename LabelSetImageType >
void mitk::LabelSetImage::EraseLabelProcessing(LabelSetImageType* itkImage, int index, bool reorder)
{
   typedef itk::ImageRegionIterator< LabelSetImageType > IteratorType;

   IteratorType iter( itkImage, itkImage->GetLargestPossibleRegion() );
   iter.GoToBegin();

   int nLabels = this->GetNumberOfLabels();

   while (!iter.IsAtEnd())
   {
      int value = static_cast<int>(iter.Get());

        if (value == index)
        {
            iter.Set( 0 );
        }
        else if (reorder)
        {
            for (int i=index+1; i<nLabels; ++i) // for each index up-to the end
            {
                if (value == i)
                {
                    iter.Set( i-1 );
                }
            }
        }
        ++iter;
   }
}

template < typename LabelSetImageType >
void mitk::LabelSetImage::MergeLabelsProcessing(LabelSetImageType* itkImage, int index)
{
   typedef itk::ImageRegionIterator< LabelSetImageType > IteratorType;

   const mitk::Label* activeLabel = this->m_LabelSet->GetActiveLabel();

   IteratorType iter( itkImage, itkImage->GetLargestPossibleRegion() );
   iter.GoToBegin();

   while (!iter.IsAtEnd())
   {
      if (static_cast<int>(iter.Get()) == index)
      {
        iter.Set( activeLabel->GetIndex() );
      }
      ++iter;
   }
}

void mitk::LabelSetImage::AddLabel(const mitk::Label& label)
{
    this->m_LabelSet->AddLabel(label);
    mitk::LookupTableProperty::Pointer lutProp = dynamic_cast<mitk::LookupTableProperty*>(this->GetPropertyList()->GetProperty( "LookupTable" ));
    double rgba[4];
    const mitk::Color& color = label.GetColor();
    rgba[0] = color.GetRed();
    rgba[1] = color.GetGreen();
    rgba[2] = color.GetBlue();
    rgba[3] = label.GetOpacity();
    // the active label is the one we just added
    lutProp->GetLookupTable()->SetTableValue( m_LabelSet->GetActiveLabelIndex(), rgba );
    AddLabelEvent.Send();
}

void mitk::LabelSetImage::AddLabel(const std::string& name, const mitk::Color& color)
{
    this->m_LabelSet->AddLabel(name, color);
    mitk::LookupTableProperty::Pointer lutProp = dynamic_cast<mitk::LookupTableProperty*>(this->GetPropertyList()->GetProperty( "LookupTable" ));
    double rgba[4];
    rgba[0] = color.GetRed();
    rgba[1] = color.GetGreen();
    rgba[2] = color.GetBlue();
    rgba[3] = m_LabelSet->GetActiveLabel()->GetOpacity();
    lutProp->GetLookupTable()->SetTableValue( m_LabelSet->GetActiveLabelIndex(), rgba );
    AddLabelEvent.Send();
}

const mitk::Color& mitk::LabelSetImage::GetLabelColor(int index)
{
   return this->m_LabelSet->GetLabelColor(index);
}

void mitk::LabelSetImage::SetLabelColor(int index, const mitk::Color& color)
{
    mitk::LookupTableProperty::Pointer lutProp = dynamic_cast<mitk::LookupTableProperty*>(this->GetPropertyList()->GetProperty( "LookupTable" ));
    double rgba[4];
    lutProp->GetLookupTable()->GetTableValue( index, rgba );
    rgba[0] = color.GetRed();
    rgba[1] = color.GetGreen();
    rgba[2] = color.GetBlue();
    rgba[3] = this->m_LabelSet->GetLabelOpacity(index);
    lutProp->GetLookupTable()->SetTableValue( index, rgba );
    this->m_LabelSet->SetLabelColor(index, color);
    ModifyLabelEvent.Send(index);
}

float mitk::LabelSetImage::GetLabelOpacity(int index)
{
    return this->m_LabelSet->GetLabelOpacity(index);
}

float mitk::LabelSetImage::GetLabelVolume(int index)
{
    return this->m_LabelSet->GetLabelVolume(index);
}

void mitk::LabelSetImage::SetLabelOpacity(int index, float value)
{
   mitk::LookupTableProperty::Pointer lutProp = dynamic_cast<mitk::LookupTableProperty*>(this->GetPropertyList()->GetProperty( "LookupTable" ));
   double rgba[4];
   lutProp->GetLookupTable()->GetTableValue(index,rgba);
   rgba[3] = value;
   lutProp->GetLookupTable()->SetTableValue(index,rgba);
   m_LabelSet->SetLabelOpacity(index,value);
   ModifyLabelEvent.Send(index);
}

void mitk::LabelSetImage::SetLabelVolume(int index, float value)
{
   m_LabelSet->SetLabelVolume(index,value);
   ModifyLabelEvent.Send(index);
}

void mitk::LabelSetImage::RenameLabel(int index, const std::string& name, const mitk::Color& color)
{
    m_LabelSet->SetLabelName(index, name);
    m_LabelSet->SetLabelColor(index, color);
    mitk::LookupTableProperty::Pointer lutProp = dynamic_cast<mitk::LookupTableProperty*>(this->GetPropertyList()->GetProperty( "LookupTable" ));
    double rgba[4];
    lutProp->GetLookupTable()->GetTableValue(index,rgba);
    rgba[0] = color.GetRed();
    rgba[1] = color.GetGreen();
    rgba[2] = color.GetBlue();
    lutProp->GetLookupTable()->SetTableValue(index,rgba);
    ModifyLabelEvent.Send(index);
}

void mitk::LabelSetImage::SetLabelName(int index, const std::string& name)
{
    this->m_LabelSet->SetLabelName(index, name);
    ModifyLabelEvent.Send(index);
}

void mitk::LabelSetImage::SetAllLabelsLocked(bool value)
{
    this->m_LabelSet->SetAllLabelsLocked(value);
    AllLabelsModifiedEvent.Send();
}

void mitk::LabelSetImage::SetAllLabelsVisible(bool value)
{
    this->m_LabelSet->SetAllLabelsVisible(value);
    mitk::LookupTableProperty::Pointer lutProp = dynamic_cast<mitk::LookupTableProperty*>(this->GetPropertyList()->GetProperty( "LookupTable" ));
    mitk::LabelSet::LabelContainerConstIteratorType _it = this->m_LabelSet->IteratorBegin();
    mitk::LabelSet::LabelContainerConstIteratorType _end = this->m_LabelSet->IteratorEnd();
    double rgba[4];
    for(; _it!=_end; ++_it)
    {
        lutProp->GetLookupTable()->GetTableValue((*_it)->GetIndex(),rgba);
        rgba[3] = value ? (*_it)->GetOpacity() : 0.0;
        lutProp->GetLookupTable()->SetTableValue((*_it)->GetIndex(),rgba);
    }
    AllLabelsModifiedEvent.Send();
}

void mitk::LabelSetImage::RemoveLabel(int index)
{
    this->EraseLabel(index, true);
    this->m_LabelSet->RemoveLabel(index);
    this->ResetLabels();
    RemoveLabelEvent.Send();
}

void mitk::LabelSetImage::ResetLabels()
{
    this->m_LabelSet->ResetLabels();
    mitk::LookupTableProperty::Pointer lutProp = dynamic_cast<mitk::LookupTableProperty*>(this->GetPropertyList()->GetProperty( "LookupTable" ));
    mitk::LabelSet::LabelContainerConstIteratorType _it = this->m_LabelSet->IteratorBegin();
    mitk::LabelSet::LabelContainerConstIteratorType _end = this->m_LabelSet->IteratorEnd();
    double rgba[4];
    for(; _it!=_end; ++_it)
    {
        const mitk::Color& color = (*_it)->GetColor();
        rgba[0] = color.GetRed();
        rgba[1] = color.GetGreen();
        rgba[2] = color.GetBlue();
        rgba[3] = (*_it)->GetOpacity();
        lutProp->GetLookupTable()->SetTableValue((*_it)->GetIndex(),rgba);
    }
}

void mitk::LabelSetImage::RemoveLabels(std::vector<int>& indexes)
{
    std::sort(indexes.begin(), indexes.end());

    for (int i=0; i<indexes.size(); i++)
    {
        this->EraseLabel(indexes[i]-i, true);
        this->m_LabelSet->RemoveLabel(indexes[i]-i);
        this->ResetLabels();
    }

    RemoveLabelEvent.Send();
}

void mitk::LabelSetImage::EraseLabels(std::vector<int>& indexes)
{
    std::sort(indexes.begin(), indexes.end());

    for (int i=0; i<indexes.size(); i++)
    {
        this->EraseLabel(indexes[i]-i, false);
    }
}

std::string mitk::LabelSetImage::GetLabelName(int index)
{
    return this->m_LabelSet->GetLabelName(index);
}

int mitk::LabelSetImage::GetActiveLabelIndex() const
{
    return this->m_LabelSet->GetActiveLabel()->GetIndex();
}

const mitk::Color& mitk::LabelSetImage::GetActiveLabelColor() const
{
    return this->m_LabelSet->GetActiveLabel()->GetColor();
}

bool mitk::LabelSetImage::GetLabelLocked(int index) const
{
    return this->m_LabelSet->GetLabelLocked(index);
}

bool mitk::LabelSetImage::GetLabelSelected(int index) const
{
    return this->m_LabelSet->GetLabelSelected(index);
}

void mitk::LabelSetImage::SetLabelSet(const mitk::LabelSet& labelset)
{
    *m_LabelSet = labelset;
    this->ResetLabels();
}

bool mitk::LabelSetImage::IsLabelSelected(mitk::Label::Pointer label)
{
    return label->GetSelected();
}

bool mitk::LabelSetImage::GetLabelVisible(int index)
{
    return this->m_LabelSet->GetLabelVisible(index);
}

void mitk::LabelSetImage::SetLabelVisible(int index, bool value)
{
   this->m_LabelSet->SetLabelVisible(index, value);

   mitk::LookupTableProperty::Pointer lutProp = dynamic_cast<mitk::LookupTableProperty*>(this->GetPropertyList()->GetProperty( "LookupTable" ));
   double rgba[4];
   lutProp->GetLookupTable()->GetTableValue(index,rgba);
   rgba[3] = value ? this->m_LabelSet->GetLabelOpacity(index) : 0.0;
   lutProp->GetLookupTable()->SetTableValue(index,rgba);
   ModifyLabelEvent.Send(index);
}

mitk::Point3D mitk::LabelSetImage::GetActiveLabelCenterOfMass()
{
    mitk::Point3D pos;
    pos.Fill(0.0);
    AccessByItk_1(this, CenterOfMassProcessing, pos);
    return pos;
}

void mitk::LabelSetImage::SetActiveLabel(int index, bool sendEvent)
{
    this->m_LabelSet->SetActiveLabel(index);
    if (sendEvent)
        ModifyLabelEvent.Send(index);
}

void mitk::LabelSetImage::SetLabelLocked(int index, bool value)
{
    this->m_LabelSet->SetLabelLocked(index, value);
}

void mitk::LabelSetImage::SetLabelSelected(int index, bool value)
{
    this->m_LabelSet->SetLabelSelected(index, value);
}

void mitk::LabelSetImage::RemoveAllLabels()
{
    this->m_LabelSet->RemoveAllLabels();
    this->ClearBuffer();
    this->Modified();
    RemoveLabelEvent.Send();
}

int mitk::LabelSetImage::GetNumberOfLabels()
{
    return this->m_LabelSet->GetNumberOfLabels();
}
