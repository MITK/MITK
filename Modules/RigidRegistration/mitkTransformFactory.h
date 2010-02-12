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

#ifndef MITKTRANSFORMFACTORY_H
#define MITKTRANSFORMFACTORY_H

#include "itkTransform.h"
#include "MitkRigidRegistrationExports.h"
#include "itkImage.h"
#include "mitkTransformParameters.h"

namespace mitk {

  /*!
  \brief This class creates a transform for a rigid registration process.  

  This class will e.g. be instantiated by mitkImageRegistrationMethod and a 
  transform corresponding to the integer value stored in mitkTransformParameters 
  will be created. Therefore SetTransformParameters() has to be called with an instance
  of mitkTransformParameters, which holds all parameter informations for the new
  transformation.

  GetTransform() returns the transform which then can be used in combination with a
  metric, an optimizer and an interpolator within a registration pipeline. 


  \ingroup RigidRegistration

  \author Daniel Stein
  */

  template < class TPixelType, unsigned int VImageDimension >
  class MITK_RIGIDREGISTRATION_EXPORT TransformFactory : public itk::Object
  {
  public:
    mitkClassMacro(TransformFactory, itk::Object);
  
    /** Method for creation through the object factory. */
    itkNewMacro(Self);

    typedef typename itk::Image< TPixelType, VImageDimension >  FixedImageType;
    typedef typename itk::Image< TPixelType, VImageDimension >  MovingImageType;
    typedef typename itk::Image< TPixelType, 2 >  FixedImage2DType;
    typedef typename itk::Image< TPixelType, 2 >  MovingImage2DType;
    typedef typename itk::Image< TPixelType, 3 >  FixedImage3DType;
    typedef typename itk::Image< TPixelType, 3 >  MovingImage3DType;

    typedef typename itk::Transform< double, VImageDimension, VImageDimension >    TransformType;

    typedef typename TransformType::Pointer TransformPointer;

    /**
    \brief Returns the transform which then can be used in combination with a metric, an optimizer 
    and an interpolator within a registration pipeline.
    */
    TransformPointer GetTransform( );

    /**
    \brief Sets the fixed image which is needed by transform initializer.
    */
    void SetFixedImage(FixedImageType* fixed);

    /**
    \brief Sets the moving image which is needed by transform initializer.
    */
    void SetMovingImage(MovingImageType* moving);

    /**
    \brief Sets the instance to the transform parameters class which holds all parameters for the new transform.
    */
    void SetTransformParameters(TransformParameters::Pointer transformParameters)
    {
      m_TransformParameters = transformParameters;
    }

    /**
    \brief Returns the instance to the transform parameters class which holds all parameters for the new transform.
    */
    TransformParameters::Pointer GetTransformParameters()
    {
      return m_TransformParameters;
    }

  protected:
    TransformFactory();
    ~TransformFactory() {};

    TransformParameters::Pointer m_TransformParameters;
    typename FixedImageType::Pointer m_FixedImage;
    typename MovingImageType::Pointer m_MovingImage;
    typename FixedImage2DType::Pointer m_FixedImage2D;
    typename MovingImage2DType::Pointer m_MovingImage2D;
    typename FixedImage3DType::Pointer m_FixedImage3D;
    typename MovingImage3DType::Pointer m_MovingImage3D;
  };

} // namespace mitk

#include "mitkTransformFactory.txx"

#endif // MITKTRANSFORMFACTORY_H
