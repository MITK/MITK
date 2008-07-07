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


namespace mitk
{

  class MITK_CORE_EXPORT ImageRegistrationMethod : public ImageToImageFilter
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
    
  protected:
    ImageRegistrationMethod();
    virtual ~ImageRegistrationMethod();

    template < typename TPixel, unsigned int VImageDimension >
      void GenerateData2( itk::Image<TPixel, VImageDimension>* itkImage1);

    RigidRegistrationObserver::Pointer m_Observer;
    int m_Interpolator;
    Image::Pointer m_ReferenceImage;
  };
}

#endif // MITKIMAGEREGISTRATIONMETHOD_H
