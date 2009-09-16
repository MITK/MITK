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


#ifndef _MITK_LABELEDIMAGEVOLUMECALCULATOR_H
#define _MITK_LABELEDIMAGEVOLUMECALCULATOR_H

#include <itkObject.h>

#include "mitkImage.h"
#include "mitkImageTimeSelector.h"

#include <itkImage.h>


namespace mitk
{

/**
 * \brief Class for calculating the volume (or area) for each label in a
 * labeled image.
 *
 * Labels are expected to be of an unsigned integer type.
 *
 * TODO: Extend class for time resolved images
 */
class MITKEXT_CORE_EXPORT LabeledImageVolumeCalculator : public itk::Object
{
public:

  typedef std::vector< double > VolumeVector;
  typedef std::vector< Point3D > PointVector;

  mitkClassMacro( LabeledImageVolumeCalculator, itk::Object );

  itkSetConstObjectMacro( Image, mitk::Image );
  

  virtual void Calculate();


  double GetVolume( unsigned int label ) const;

  const Point3D &GetCentroid( unsigned int label ) const;


  const VolumeVector &GetVolumes() const;

  const PointVector &GetCentroids() const;

  
protected:

  LabeledImageVolumeCalculator();
  
  virtual ~LabeledImageVolumeCalculator();


  template < typename TPixel, unsigned int VImageDimension >
  void _InternalCalculateVolumes(
    itk::Image< TPixel, VImageDimension > *image, 
    LabeledImageVolumeCalculator *volumeCalculator, 
    Geometry3D *geometry );


  ImageTimeSelector::Pointer m_InputTimeSelector;


  Image::ConstPointer m_Image;


  VolumeVector m_VolumeVector;
  PointVector m_CentroidVector;

  Point3D m_DummyPoint;

};

}

#endif // #define _MITK_LABELEDIMAGEVOLUMECALCULATOR_H
