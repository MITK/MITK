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

#include "itkImageMaskSpatialObject.h"
#include "mitkRigidRegistrationPreset.h"

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

    typedef itk::SingleValuedNonLinearOptimizer         OptimizerType;
    typedef itk::ImageMaskSpatialObject< 3 >            MaskType;

    mitkClassMacro(ImageRegistrationMethod, ImageToImageFilter);

    itkNewMacro(Self);

    static const int LINEARINTERPOLATOR = 0;
    static const int NEARESTNEIGHBORINTERPOLATOR = 1;

    void SetObserver(RigidRegistrationObserver::Pointer observer);

    void SetInterpolator(int interpolator);

    virtual void GenerateData();

    virtual void SetReferenceImage( Image::Pointer fixedImage);

    virtual void SetFixedMask( Image::Pointer fixedMask);

    virtual void SetMovingMask( Image::Pointer movingMask);    

    void SetOptimizerScales(itk::Array<double> scales); 

    void SetTransform(itk::Object::Pointer transform);

    void SetMetric(itk::Object::Pointer metric);

    void SetOptimizer(itk::Object::Pointer optimizer);

  protected:
    ImageRegistrationMethod();
    virtual ~ImageRegistrationMethod();

    template < typename TPixel, unsigned int VImageDimension >
    void GenerateData2( itk::Image<TPixel, VImageDimension>* itkImage1);

    RigidRegistrationObserver::Pointer m_Observer;
    int m_Interpolator;
    Image::Pointer m_ReferenceImage;
    Image::Pointer m_FixedMask;
    Image::Pointer m_MovingMask;

    virtual void GenerateOutputInformation(){};

  private:
    itk::Object::Pointer m_Transform;
    itk::Object::Pointer m_Metric;
    itk::Object::Pointer m_Optimizer;
    itk::Array<double> m_OptimizerScales;
  };
}

//#include "mitkImageRegistrationMethod.txx"

#endif // MITKIMAGEREGISTRATIONMETHOD_H

