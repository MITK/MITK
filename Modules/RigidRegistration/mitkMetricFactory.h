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

#ifndef MITKMETRICFACTORY_H
#define MITKMETRICFACTORY_H

#include "itkImageToImageMetric.h"
#include "itkImage.h"
#include "mitkMetricParameters.h"
#include "itkImageMaskSpatialObject.h"

namespace mitk {

  /*!
  \brief This class creates a metric for a rigid registration process.  

  This class will e.g. be instantiated by mitkImageRegistrationMethod and a 
  metric corresponding to the integer value stored in mitkMetricParameters 
  will be created. Therefore SetMetricParameters() has to be called with an instance
  of mitkMetricParameters, which holds all parameter informations for the new
  metric.

  GetMetric() returns the metric which then can be used in combination with a
  transform, an optimizer and an interpolator within a registration pipeline. 


  \ingroup RigidRegistration

  \author Daniel Stein
  */

  template < class TPixelType, unsigned int VImageDimension >
  class MetricFactory : public itk::Object
  {
  public:
    mitkClassMacro(MetricFactory, itk::Object);

    /** Method for creation through the object factory. */
    itkNewMacro(Self);

    typedef typename itk::Image< TPixelType, VImageDimension >  FixedImageType;
    typedef typename itk::Image< unsigned char, VImageDimension >  FixedMaskImageType;
    typedef typename itk::ImageMaskSpatialObject< VImageDimension > FixedImageMaskType;
    typedef typename itk::Image< TPixelType, VImageDimension >  MovingImageType;
    typedef typename itk::Image< unsigned char, VImageDimension >  MovingMaskImageType;
    typedef typename itk::ImageMaskSpatialObject< VImageDimension > MovingImageMaskType;

    typedef typename itk::ImageToImageMetric< FixedImageType, MovingImageType >    MetricType;
    typedef typename MetricType::Pointer                MetricPointer;

    /**
    \brief Returns the metric which then can be used in combination with a transform, an optimizer 
    and an interpolator within a registration pipeline.
    */
    MetricPointer GetMetric( );
    
    /**
    \brief Sets the instance to the metric parameters class which holds all parameters for the new metric.
    */
    void SetMetricParameters(MetricParameters::Pointer metricParameters)
    {
      m_MetricParameters = metricParameters;
    }

    /**
    \brief Returns the instance to the metric parameters class which holds all parameters for the new metric.
    */
    MetricParameters::Pointer GetMetricParameters()
    {
      return m_MetricParameters;
    }

  protected:
    MetricFactory();
    ~MetricFactory() {};

    MetricParameters::Pointer m_MetricParameters;
  };

} // namespace mitk

#include "mitkMetricFactory.txx"

#endif // MITKMETRICFACTORY_H
