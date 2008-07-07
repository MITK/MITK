/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkImageToImageFilter.h,v $
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

#ifndef MITKMETRICFACTORY_H
#define MITKMETRICFACTORY_H

#include "itkImageToImageMetric.h"
#include "itkImage.h"

namespace mitk {

  template < class TPixelType, unsigned int VImageDimension >
  class MetricFactory //: public itk::Object
  {
  public:
    MetricFactory();
    ~MetricFactory();

    typedef typename itk::Image< TPixelType, VImageDimension >  FixedImageType;
    typedef typename itk::Image< TPixelType, VImageDimension >  MovingImageType;

    typedef typename itk::ImageToImageMetric< FixedImageType, MovingImageType >    MetricType;
    typedef typename MetricType::Pointer                MetricPointer;

    MetricPointer GetMetric( );

  protected:
    //MetricFactory();
    //virtual ~MetricFactory();
  };

} // namespace mitk

#include "mitkMetricFactory.txx"

#endif // MITKMETRICFACTORY_H
