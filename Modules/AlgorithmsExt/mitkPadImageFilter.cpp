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


#include "mitkPadImageFilter.h"
#include "mitkImageCast.h"

#include "itkBinaryThresholdImageFilter.h"
#include "itkConstantPadImageFilter.h"

mitk::PadImageFilter::PadImageFilter()
{
  this->SetNumberOfIndexedInputs(2);
  this->SetNumberOfRequiredInputs(2);
  m_BinaryFilter = false;

  m_PadConstant = -32766;
  m_LowerThreshold = -32766;
  m_UpperThreshold = -32765;
}

mitk::PadImageFilter::~PadImageFilter()
{
}


void mitk::PadImageFilter::GenerateData()
{
  mitk::Image::ConstPointer image = this->GetInput( 0 );
  mitk::Image::ConstPointer referenceImage = this->GetInput( 1 );

  typedef itk::Image< short, 3 > ImageType;
  ImageType::Pointer itkImage = ImageType::New();
  mitk::CastToItkImage( image, itkImage );


  mitk::BaseGeometry *imageGeometry = image->GetGeometry();
  mitk::Point3D origin = imageGeometry->GetOrigin();
  mitk::Vector3D spacing = imageGeometry->GetSpacing();

  mitk::BaseGeometry *referenceImageGeometry = referenceImage->GetGeometry();
  mitk::Point3D referenceOrigin = referenceImageGeometry->GetOrigin();

  double outputOrigin[3];
  unsigned long padLowerBound[3];
  unsigned long padUpperBound[3];

  int i;
  for ( i = 0; i < 3; ++i )
  {
    outputOrigin[i] = referenceOrigin[i];

    padLowerBound[i] = static_cast< unsigned long >
      ((origin[i] - referenceOrigin[i]) / spacing[i] + 0.5);

    padUpperBound[i] = referenceImage->GetDimension( i )
      - image->GetDimension( i ) - padLowerBound[i];
  }

  // The origin of the input image is passed through the filter and used as
  // output origin as well. Hence, it needs to be overwritten accordingly.
  itkImage->SetOrigin( outputOrigin );


  typedef itk::ConstantPadImageFilter< ImageType, ImageType > PadFilterType;
  PadFilterType::Pointer padFilter = PadFilterType::New();
  padFilter->SetInput( itkImage );
  padFilter->SetConstant( m_PadConstant );
  padFilter->SetPadLowerBound( padLowerBound );
  padFilter->SetPadUpperBound( padUpperBound );

  mitk::Image::Pointer outputImage = this->GetOutput();


  // If the Binary flag is set, use an additional binary threshold filter after
  // padding.
  if ( m_BinaryFilter )
  {
    typedef itk::Image< unsigned char, 3 > BinaryImageType;
    typedef itk::BinaryThresholdImageFilter< ImageType, BinaryImageType >
      BinaryFilterType;
    BinaryFilterType::Pointer binaryFilter = BinaryFilterType::New();

    binaryFilter->SetInput( padFilter->GetOutput() );
    binaryFilter->SetLowerThreshold( m_LowerThreshold );
    binaryFilter->SetUpperThreshold( m_UpperThreshold );
    binaryFilter->SetInsideValue( 1 );
    binaryFilter->SetOutsideValue( 0 );
    binaryFilter->Update();

    mitk::CastToMitkImage( binaryFilter->GetOutput(), outputImage );
  }
  else
  {
    padFilter->Update();
    mitk::CastToMitkImage( padFilter->GetOutput(), outputImage );
  }

  outputImage->SetRequestedRegionToLargestPossibleRegion();
}
