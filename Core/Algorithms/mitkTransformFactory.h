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
#include "itkImage.h"

namespace mitk {

  template < class TPixelType, unsigned int VImageDimension >
  class TransformFactory //: public itk::Object
  {
  public:
    TransformFactory();
    ~TransformFactory();
    //mitkClassMacro(MetricFactory<class TFixedImage, class TMovingImage>, RegistrationBase);
  
	  /** Method for creation through the object factory. */
	  //itkNewMacro(Self);

    typedef typename itk::Image< TPixelType, VImageDimension >  FixedImageType;
    typedef typename itk::Image< TPixelType, VImageDimension >  MovingImageType;
    typedef typename itk::Image< TPixelType, 2 >  FixedImage2DType;
    typedef typename itk::Image< TPixelType, 2 >  MovingImage2DType;
    typedef typename itk::Image< TPixelType, 3 >  FixedImage3DType;
    typedef typename itk::Image< TPixelType, 3 >  MovingImage3DType;

    typedef typename itk::Transform< double, VImageDimension, VImageDimension >    TransformType;

    typedef typename TransformType::Pointer TransformPointer;

    TransformPointer GetTransform( );
    void SetFixedImage(FixedImageType* fixed);
    void SetMovingImage(MovingImageType* moving);

  protected:
    //TransformFactory();
    //virtual ~TransformFactory();
      //template < unsigned int VImageDimension >
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
