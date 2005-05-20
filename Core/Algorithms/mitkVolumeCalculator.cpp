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

#include <itkImageRegionConstIterator.h>

template < typename TPixel, unsigned int VImageDimension >
void mitk::InternalCompute(itk::Image< TPixel, VImageDimension >* itkImage, mitk::VolumeCalculator* volumeCalculator)
{
  itk::ImageRegionConstIterator<itk::Image < TPixel, VImageDimension > >
  imageIt(itkImage, itkImage->GetLargestPossibleRegion() );
  unsigned int count = 0;

  for (imageIt.GoToBegin(); !imageIt.IsAtEnd(); ++imageIt)
  {
    if ( (int)(imageIt.Get()) > volumeCalculator->m_Threshold )
    {
      count++;
    }
  }
  mitk::Vector3D spacing = volumeCalculator->m_Image->GetSlicedGeometry()->GetSpacing();
  volumeCalculator->m_Volume = count / 1000.0 * spacing[0] * spacing[1] * spacing[2];
}

#include "mitkImageAccessByItk.h"

mitk::VolumeCalculator::VolumeCalculator() : m_Image(NULL), m_Threshold(0)
{
  m_TimeSelector = ImageTimeSelector::New();
}

mitk::VolumeCalculator::~VolumeCalculator()
{}

void mitk::VolumeCalculator::ComputeVolume()
{
  if (m_Image->GetDimension() == 4) {
    m_TimeSelector->SetInput(m_Image);
    m_Volumes.resize(m_Image->GetDimension(3));
    for (unsigned int timeStep = 0; timeStep<m_Image->GetDimension(3); timeStep++) {
      m_TimeSelector->SetTimeNr(timeStep);
      m_TimeSelector->Update();
      AccessFixedDimensionByItk_1(m_TimeSelector->GetOutput(),InternalCompute,3,this);
      m_Volumes[timeStep] = m_Volume;
    }
  } else if (m_Image->GetDimension() == 3) {
    const_cast<mitk::Image*>(m_Image.GetPointer())->Update();
    AccessFixedDimensionByItk_1(m_Image,InternalCompute,3,this);
  } else {
    m_Volume = 0;
  }
}
