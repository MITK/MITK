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

#include "mitkVolumeCalculator.h"
#include "mitkImageAccessByItk.h" 
#include <itkImageRegionConstIterator.h> 
  
template < typename TPixel, unsigned int VImageDimension >
void mitk::VolumeCalculator::InternalCompute(itk::Image< TPixel, VImageDimension >* itkImage)
{
  itk::ImageRegionConstIterator<itk::Image < TPixel, VImageDimension > > imageIt(itkImage, itkImage->GetLargestPossibleRegion() );
  unsigned long int count = 0;

  for (imageIt.GoToBegin(); !imageIt.IsAtEnd(); ++imageIt)
  {
    if ( (int)(imageIt.Get()) >= m_Threshold )
    {
      count++;
    }
  }
  if (m_Image->GetDimension() == 3) 
  {
    Vector3D spacing = m_Image->GetSlicedGeometry()->GetSpacing();
    m_Volume = count / 1000.0 * spacing[0] * spacing[1] * spacing[2];
  } 
  else if (m_Image->GetDimension() == 2) 
  {
    Vector3D spacing = m_Image->GetGeometry()->GetSpacing();
    m_Volume = count / 100.0 * spacing[0] * spacing[1];
  }
  m_VoxelCount = count;
}

mitk::VolumeCalculator::VolumeCalculator() 
: m_Image(NULL), 
  m_Threshold(0)
{
  m_TimeSelector = ImageTimeSelector::New();
}

mitk::VolumeCalculator::~VolumeCalculator()
{
}
    
std::vector<float> mitk::VolumeCalculator::GetVolumes() 
{
  return m_Volumes;
}

void mitk::VolumeCalculator::ComputeVolume()
{
  if (m_Image->GetDimension() == 4) 
  {
    m_TimeSelector->SetInput(m_Image);
    m_Volumes.resize(m_Image->GetDimension(3));
    for (unsigned int timeStep = 0; timeStep<m_Image->GetDimension(3); ++timeStep) 
    {
      m_TimeSelector->SetTimeNr(timeStep);
      m_TimeSelector->Update();
      AccessFixedDimensionByItk(m_TimeSelector->GetOutput(),InternalCompute,3);
      m_Volumes[timeStep] = m_Volume;
    }
  }
  else if (m_Image->GetDimension() == 3) 
  {
    const_cast<Image*>(m_Image.GetPointer())->Update();
    AccessFixedDimensionByItk(m_Image,InternalCompute,3);
  } 
  else if (m_Image->GetDimension() == 2) 
  {
    const_cast<Image*>(m_Image.GetPointer())->Update();
    AccessFixedDimensionByItk(m_Image,InternalCompute,2);
  }
}

