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


#include "mitkImageTimeSelector.h"


mitk::ImageTimeSelector::ImageTimeSelector() : m_TimeNr(0), m_ChannelNr(0)
{
}


mitk::ImageTimeSelector::~ImageTimeSelector()
{
}

void mitk::ImageTimeSelector::GenerateOutputInformation()
{
  Image::ConstPointer input  = this->GetInput();
  Image::Pointer output = this->GetOutput();

  itkDebugMacro(<<"GenerateOutputInformation()");

  int dim=(input->GetDimension()<3?input->GetDimension():3);
  output->Initialize(input->GetPixelType(), dim, input->GetDimensions());

  if( (unsigned int) m_TimeNr >= input->GetDimension(3) )
  {
    m_TimeNr = input->GetDimension(3)-1;
  }

  // initialize geometry
  mitk::SlicedGeometry3D::Pointer sliced_geo = input->GetSlicedGeometry(m_TimeNr);
  if( sliced_geo.IsNull()  )
  {
     mitkThrow() << "Failed to retrieve SlicedGeometry from input at timestep " << m_TimeNr;
  }

  mitk::SlicedGeometry3D::Pointer sliced_geo_clone = sliced_geo->Clone();
  if( sliced_geo_clone.IsNull()  )
  {
     mitkThrow() << "Failed to clone the retrieved sliced geometry.";
  }

  mitk::BaseGeometry::Pointer geom_3d =  dynamic_cast<BaseGeometry*>(sliced_geo_clone.GetPointer());
  if( geom_3d.IsNotNull() )
  {
    output->SetGeometry(geom_3d.GetPointer() );
  }
  else
  {
    mitkThrow() << "Failed to cast the retrieved SlicedGeometry to a Geometry3D object.";
  }

  output->SetPropertyList(input->GetPropertyList()->Clone());
}

void mitk::ImageTimeSelector::GenerateData()
{
  const Image::RegionType& requestedRegion = this->GetOutput()->GetRequestedRegion();

  mitk::ImageDataItem::Pointer im = this->GetVolumeData(m_TimeNr, m_ChannelNr)->Clone();
  im->SetTimestep(0);
  im->SetManageMemory(0);
  this->SetVolumeItem(im, 0);
}

void mitk::ImageTimeSelector::GenerateInputRequestedRegion()
{
  Superclass::GenerateInputRequestedRegion();

  ImageToImageFilter::InputImagePointer input =
    const_cast< mitk::ImageToImageFilter::InputImageType * > ( this->GetInput() );
  Image::Pointer output = this->GetOutput();

  Image::RegionType requestedRegion;
  requestedRegion = output->GetRequestedRegion();
  requestedRegion.SetIndex(3, m_TimeNr);
  requestedRegion.SetIndex(4, m_ChannelNr);
  requestedRegion.SetSize(3, 1);
  requestedRegion.SetSize(4, 1);

  input->SetRequestedRegion( & requestedRegion );
}
