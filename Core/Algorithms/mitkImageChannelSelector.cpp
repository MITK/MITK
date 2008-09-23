/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkImageChannelSelector.h"
#include <itkSmartPointerForwardReference.txx>


mitk::ImageChannelSelector::ImageChannelSelector() : m_ChannelNr(0)
{
}

mitk::ImageChannelSelector::~ImageChannelSelector()
{
}

void mitk::ImageChannelSelector::GenerateOutputInformation()
{
  mitk::Image::ConstPointer input  = this->GetInput();
  mitk::Image::Pointer output = this->GetOutput();

  itkDebugMacro(<<"GenerateOutputInformation()");

  output->Initialize(input->GetPixelType(), input->GetDimension(), input->GetDimensions());

  // initialize geometry
  output->SetPropertyList(input->GetPropertyList()->Clone());  
	output->SetGeometry(dynamic_cast<Geometry3D*>(input->GetTimeSlicedGeometry()->Clone().GetPointer()));
}

void mitk::ImageChannelSelector::GenerateData()
{
  const Image::RegionType& requestedRegion = GetOutput()->GetRequestedRegion();

  //do we really need the complete channel?
  if(requestedRegion.GetSize(3)>1)
    SetChannelItem(GetChannelData(m_ChannelNr), 0);
  else
  //or only a complete volume at a time?
  if(requestedRegion.GetSize(2)>1)
    SetVolumeItem(GetVolumeData(requestedRegion.GetIndex(3), m_ChannelNr), requestedRegion.GetIndex(3), 0);
  else
  //not even a complete volume, so now take just a slice!
    SetSliceItem(GetSliceData(requestedRegion.GetIndex(2), requestedRegion.GetIndex(3), m_ChannelNr), requestedRegion.GetIndex(2), requestedRegion.GetIndex(3), 0);
}

void mitk::ImageChannelSelector::GenerateInputRequestedRegion()
{
  Superclass::GenerateInputRequestedRegion();

  mitk::ImageToImageFilter::InputImagePointer input =
    const_cast< mitk::ImageToImageFilter::InputImageType * > ( this->GetInput() );
  mitk::Image::Pointer output = this->GetOutput();

  Image::RegionType requestedRegion;
  requestedRegion = output->GetRequestedRegion();
  requestedRegion.SetIndex(4, m_ChannelNr);
  requestedRegion.SetSize(4, 1);

  input->SetRequestedRegion( & requestedRegion );
}


