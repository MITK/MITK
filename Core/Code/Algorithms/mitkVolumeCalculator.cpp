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

#include "mitkImageStatisticsHolder.h"
  
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
  if (itkImage->GetLargestPossibleRegion().GetImageDimension() == 3) 
  {
    m_Volume = count / 1000.0 * itkImage->GetSpacing()[0] * itkImage->GetSpacing()[1] * itkImage->GetSpacing()[2];
  } 
  else if (itkImage->GetLargestPossibleRegion().GetImageDimension() == 2) 
  {
    m_Volume = count / 100.0 * itkImage->GetSpacing()[0] * itkImage->GetSpacing()[1];
  }
  m_VoxelCount = count;
}

mitk::VolumeCalculator::VolumeCalculator() 
: m_Image(NULL), 
  m_Threshold(0),
  m_Volume(0)
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
  const_cast<Image*>(m_Image.GetPointer())->SetRequestedRegionToLargestPossibleRegion();
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

void mitk::VolumeCalculator::ComputeVolumeFromImageStatistics()
{
  unsigned int dim = m_Image->GetDimension();

  if(dim == 4)
  {
    m_Volumes.resize(m_Image->GetDimension(3),0);
    Vector3D spacing = m_Image->GetSlicedGeometry()->GetSpacing();

    for(unsigned int t = 0; t < m_Image->GetDimension(3); ++t )
    {
      m_Volumes[t] = m_Image->GetStatistics()->GetCountOfMaxValuedVoxels(t) / 1000.0 * spacing[0] * spacing[1] * spacing[2];
    }
  }
  else if(dim == 3)
  {
    Vector3D spacing = m_Image->GetSlicedGeometry()->GetSpacing();
    m_Volume = m_Image->GetStatistics()->GetCountOfMaxValuedVoxels() / 1000.0 * spacing[0] * spacing[1] * spacing[2];
  }
  else if (dim == 2) 
  {
    Vector3D spacing = m_Image->GetGeometry()->GetSpacing();
    m_Volume = m_Image->GetStatistics()->GetCountOfMaxValuedVoxels() / 100.0 * spacing[0] * spacing[1];
  }
  else itkExceptionMacro(<<"Wrong image dimension...");
}

float mitk::VolumeCalculator::ComputeVolume(Vector3D spacing, unsigned int voxelCount)
{
  return (voxelCount / 1000.0 * spacing[0] * spacing[1] * spacing[2]);
}

