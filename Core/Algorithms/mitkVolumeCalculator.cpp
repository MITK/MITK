/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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

#include "mitkVolumeCalculator.h"
#include "mitkImageAccessByItk.h"
#include <itkImageRegionConstIterator.h>

mitk::VolumeCalculator::VolumeCalculator() : m_Image(NULL), m_Threshold(0)
{}

mitk::VolumeCalculator::~VolumeCalculator()
{}

void mitk::VolumeCalculator::ComputeVolume()
{
  AccessByItk(m_Image,InternalCompute);
}

template < typename TPixel, unsigned int VImageDimension >
void mitk::VolumeCalculator::InternalCompute(itk::Image< TPixel, VImageDimension >* itkImage)
{
  itk::ImageRegionConstIterator<itk::Image < TPixel, VImageDimension > >
  imageIt(itkImage, itkImage->GetLargestPossibleRegion() );
  unsigned int count = 0;

  for (imageIt.GoToBegin(); !imageIt.IsAtEnd(); ++imageIt)
  {
    if ( imageIt.Get() > m_Threshold )
    {
      count++;
    }
  }
  mitk::Vector3D spacing = m_Image->GetSlicedGeometry()->GetSpacing();
  m_Volume = count / 1000.0 * spacing[0] * spacing[1] * spacing[2];
}
