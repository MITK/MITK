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

#ifndef MITKIMAGEREGISTRATIONMETHOD_H
#define MITKIMAGEREGISTRATIONMETHOD_H

#include "itkImageRegistrationMethod.h"
#include "itkSingleValuedNonLinearOptimizer.h"

#include "mitkImageToImageFilter.h"
#include "mitkImageAccessByItk.h"
#include "mitkRigidRegistrationObserver.h"
#include "mitkCommon.h"
#include "mitkOptimizerParameters.h"
#include "mitkTransformParameters.h"
#include "mitkMetricParameters.h"


namespace mitk
{


  /*!
  \brief Main class for the rigid registration pipeline.




  \ingroup RigidRegistration

  \author Daniel Stein
  */
  class MITKEXT_CORE_EXPORT ImageRegistrationMethod : public ImageToImageFilter
  {

  public:
    
    mitkClassMacro(ImageRegistrationMethod, ImageToImageFilter);

    itkNewMacro(Self);

    static const int LINEARINTERPOLATOR = 0;
    static const int NEARESTNEIGHBORINTERPOLATOR = 1;

    typedef itk::SingleValuedNonLinearOptimizer OptimizerType;

    void SetObserver(RigidRegistrationObserver::Pointer observer);

    void SetInterpolator(int interpolator);
    
    virtual void GenerateData();

    virtual void SetReferenceImage( Image::Pointer fixedImage);

    void SetOptimizerParameters(OptimizerParameters::Pointer optimizerParameters)
    {
      m_OptimizerParameters = optimizerParameters;
    }

    OptimizerParameters::Pointer GetOptimizerParameters()
    {
      return m_OptimizerParameters;
    }

    void SetTransformParameters(TransformParameters::Pointer transformParameters)
    {
      m_TransformParameters = transformParameters;      
    }

    TransformParameters::Pointer GetTransformParameters()
    {
      return m_TransformParameters;
    }

    void SetMetricParameters(MetricParameters::Pointer metricParameters)
    {
      m_MetricParameters = metricParameters;
    }

    MetricParameters::Pointer GetMetricParameters()
    {
      return m_MetricParameters;
    }

  protected:
    ImageRegistrationMethod();
    virtual ~ImageRegistrationMethod();

    template < typename TPixel, unsigned int VImageDimension >
      void GenerateData2( itk::Image<TPixel, VImageDimension>* itkImage1);

    RigidRegistrationObserver::Pointer m_Observer;
    int m_Interpolator;
    Image::Pointer m_ReferenceImage;

  private:
    OptimizerParameters::Pointer m_OptimizerParameters;
    TransformParameters::Pointer m_TransformParameters;
    MetricParameters::Pointer m_MetricParameters;

  };
}

#endif // MITKIMAGEREGISTRATIONMETHOD_H
