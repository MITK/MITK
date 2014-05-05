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


#include "mitkImageSliceSelector.h"


void mitk::ImageSliceSelector::GenerateOutputInformation()
{
  mitk::Image::ConstPointer input  = this->GetInput();
  mitk::Image::Pointer output = this->GetOutput();

  itkDebugMacro(<<"GenerateOutputInformation()");

  output->Initialize(input->GetPixelType(), 2, input->GetDimensions());

  if( (unsigned int)m_SliceNr >= input->GetDimension(2) )
  {
    m_SliceNr = input->GetDimension(2)-1;
  }

  if( (unsigned int)m_TimeNr >= input->GetDimension(3) )
  {
    m_TimeNr = input->GetDimension(3)-1;
  }

  // initialize geometry
  output->SetGeometry(dynamic_cast<BaseGeometry*>(input->GetSlicedGeometry(m_TimeNr)->GetPlaneGeometry(m_SliceNr)->Clone().GetPointer()));
  output->SetPropertyList(input->GetPropertyList()->Clone());
}

void mitk::ImageSliceSelector::GenerateData()
{
  SetSliceItem(GetSliceData(m_SliceNr, m_TimeNr, m_ChannelNr), 0);
}

mitk::ImageSliceSelector::ImageSliceSelector() : m_SliceNr(0), m_TimeNr(0), m_ChannelNr(0)
{
}


mitk::ImageSliceSelector::~ImageSliceSelector()
{
}

void mitk::ImageSliceSelector::GenerateInputRequestedRegion()
{
  Superclass::GenerateInputRequestedRegion();

  mitk::ImageToImageFilter::InputImagePointer input =
    const_cast< mitk::ImageToImageFilter::InputImageType * > ( this->GetInput() );
  mitk::Image::Pointer output = this->GetOutput();

  Image::RegionType requestedRegion;
  requestedRegion = output->GetRequestedRegion();
  requestedRegion.SetIndex(2, m_SliceNr);
  requestedRegion.SetIndex(3, m_TimeNr);
  requestedRegion.SetIndex(4, m_ChannelNr);
  requestedRegion.SetSize(2, 1);
  requestedRegion.SetSize(3, 1);
  requestedRegion.SetSize(4, 1);

  input->SetRequestedRegion( & requestedRegion );
}
