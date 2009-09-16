/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-05-12 19:56:03 +0200 (Di, 12 Mai 2009) $
Version:   $Revision: 17179 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#include "mitkLabeledImageVolumeCalculator.h"
#include "mitkImageAccessByItk.h"

#include <itkImageRegionConstIteratorWithIndex.h>

namespace mitk
{

LabeledImageVolumeCalculator::LabeledImageVolumeCalculator()
{
  m_InputTimeSelector = ImageTimeSelector::New();

  m_DummyPoint.Fill( 0.0 );
}


LabeledImageVolumeCalculator::~LabeledImageVolumeCalculator()
{
}


double LabeledImageVolumeCalculator::GetVolume( unsigned int label ) const
{
  if ( label < m_VolumeVector.size() )
    return m_VolumeVector[label];
  else
    return 0.0;
}

const Point3D &LabeledImageVolumeCalculator::GetCentroid( unsigned int label ) const
{
  if ( label < m_CentroidVector.size() )
    return m_CentroidVector[label];
  else
    return m_DummyPoint;
}


const LabeledImageVolumeCalculator::VolumeVector &
LabeledImageVolumeCalculator::GetVolumes() const
{
  return m_VolumeVector;
}


const LabeledImageVolumeCalculator::PointVector &
LabeledImageVolumeCalculator::GetCentroids() const
{
  return m_CentroidVector;
}


void LabeledImageVolumeCalculator::Calculate()
{
  if ( m_Image.IsNull() )
  {
    itkExceptionMacro( << "Image not set!" );
    return;
  }

  m_InputTimeSelector->SetInput( m_Image );

  //const TimeSlicedGeometry *timeSlicedGeometry = m_Image->GetTimeSlicedGeometry();
  //for( unsigned int t = 0; t < timeSlicedGeometry->GetTimeSteps(); ++t )
  //{
  m_InputTimeSelector->SetTimeNr( 0 );
  m_InputTimeSelector->UpdateLargestPossibleRegion();

  AccessByItk_2( 
    m_InputTimeSelector->GetOutput(),
    _InternalCalculateVolumes,
    this,
    m_Image->GetGeometry( 0 ) );
  //}
}


template < typename TPixel, unsigned int VImageDimension >
void LabeledImageVolumeCalculator::_InternalCalculateVolumes(
  itk::Image< TPixel, VImageDimension > *image, 
  LabeledImageVolumeCalculator *volumeCalculator, 
  Geometry3D *geometry )
{
  typedef itk::Image< TPixel, VImageDimension > ImageType;
  typedef typename ImageType::IndexType IndexType;
  typedef itk::ImageRegionConstIteratorWithIndex< ImageType > IteratorType;

  // Reset volume and centroid vectors
  m_VolumeVector.clear();
  m_CentroidVector.clear();

  // Iterate over image and determine number of voxels and centroid
  // per label
  IteratorType it( image, image->GetBufferedRegion() );
  for ( it.GoToBegin(); !it.IsAtEnd(); ++it )
  {
    const IndexType &index = it.GetIndex();
    typename IteratorType::PixelType pixel = it.Get();

    if ( m_VolumeVector.size() <= pixel )
    {
      m_VolumeVector.resize( pixel + 1 );
      m_CentroidVector.resize( pixel + 1 );
    }

    m_VolumeVector[pixel] += 1.0;

    m_CentroidVector[pixel][0] += index[0];
    m_CentroidVector[pixel][1] += index[1];
    m_CentroidVector[pixel][2] += index[2];
  }

  // Calculate voxel volume from spacing
  const Vector3D &spacing = geometry->GetSpacing();
  double voxelVolume = spacing[0] * spacing[1] * spacing[2];

  // Calculate centroid (in world coordinates) and volumes for all labels
  for ( unsigned int i = 0; i < m_VolumeVector.size(); ++i )
  {
    if ( m_VolumeVector[i] > 0.0 )
    {
      m_CentroidVector[i][0] /= m_VolumeVector[i];
      m_CentroidVector[i][1] /= m_VolumeVector[i];
      m_CentroidVector[i][2] /= m_VolumeVector[i];
      geometry->IndexToWorld( m_CentroidVector[i], m_CentroidVector[i] );

      m_VolumeVector[i] *= voxelVolume;
    }
  }
}

}