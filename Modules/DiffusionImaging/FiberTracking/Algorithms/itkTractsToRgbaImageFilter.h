/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/
#ifndef __itkTractsToRgbaImageFilter_h__
#define __itkTractsToRgbaImageFilter_h__

#include <itkImageSource.h>
#include <itkImage.h>
#include <itkVectorContainer.h>
#include <itkRGBAPixel.h>
#include <mitkFiberBundle.h>

namespace itk{

/**
* \brief Generates RGBA image from the input fibers where color values are set according to the local fiber directions.   */

template< class OutputImageType >
class TractsToRgbaImageFilter : public ImageSource< OutputImageType >
{

public:
  typedef TractsToRgbaImageFilter Self;
  typedef ProcessObject Superclass;
  typedef SmartPointer< Self > Pointer;
  typedef SmartPointer< const Self > ConstPointer;

  typedef typename OutputImageType::PixelType OutPixelType;
  typedef itk::Image<unsigned char, 3> InputImageType;

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)
  itkTypeMacro( TractsToRgbaImageFilter, ImageSource )

  /** Upsampling factor **/
  itkSetMacro( UpsamplingFactor, float)
  itkGetMacro( UpsamplingFactor, float)

  itkSetMacro( FiberBundle, mitk::FiberBundle::Pointer)
  itkSetMacro( InputImage, typename InputImageType::Pointer)

  /** Use input image geometry to initialize output image **/
  itkSetMacro( UseImageGeometry, bool)
  itkGetMacro( UseImageGeometry, bool)


  void GenerateData();

protected:

  TractsToRgbaImageFilter();
  virtual ~TractsToRgbaImageFilter();

  mitk::FiberBundle::Pointer        m_FiberBundle;      ///< input fiber bundle
  float                             m_UpsamplingFactor; ///< use higher resolution for ouput image
  bool                              m_UseImageGeometry; ///< output image is given other geometry than fiberbundle (input image geometry)
  typename InputImageType::Pointer  m_InputImage;
};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkTractsToRgbaImageFilter.cpp"
#endif

#endif // __itkTractsToRgbaImageFilter_h__
