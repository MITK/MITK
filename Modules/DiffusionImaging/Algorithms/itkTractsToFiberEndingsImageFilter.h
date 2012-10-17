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
#ifndef __itkTractsToFiberEndingsImageFilter_h__
#define __itkTractsToFiberEndingsImageFilter_h__

#include <itkImageSource.h>
#include <itkImage.h>
#include <itkVectorContainer.h>
#include <itkRGBAPixel.h>
#include <mitkFiberBundleX.h>

namespace itk{

template< class OutputImageType >
class TractsToFiberEndingsImageFilter : public ImageSource< OutputImageType >
{

public:
  typedef TractsToFiberEndingsImageFilter Self;
  typedef ProcessObject Superclass;
  typedef SmartPointer< Self > Pointer;
  typedef SmartPointer< const Self > ConstPointer;

  typedef typename OutputImageType::PixelType OutPixelType;

  itkNewMacro(Self)
  itkTypeMacro( TractsToFiberEndingsImageFilter, ImageSource )

  /** Upsampling factor **/
  itkSetMacro( UpsamplingFactor, float)
  itkGetMacro( UpsamplingFactor, float)

  /** Invert Image **/
  itkSetMacro( InvertImage, bool)
  itkGetMacro( InvertImage, bool)

  itkSetMacro( FiberBundle, mitk::FiberBundleX::Pointer)
  itkSetMacro( InputImage, typename OutputImageType::Pointer)

  /** Use input image geometry to initialize output image **/
  itkSetMacro( UseImageGeometry, bool)
  itkGetMacro( UseImageGeometry, bool)

  itkSetMacro( BinaryOutput, bool)

  void GenerateData();

protected:

  itk::Point<float, 3> GetItkPoint(double point[3]);

  TractsToFiberEndingsImageFilter();
  virtual ~TractsToFiberEndingsImageFilter();

  mitk::FiberBundleX::Pointer m_FiberBundle;
  float m_UpsamplingFactor;
  bool m_InvertImage;
  bool m_UseImageGeometry;
  bool m_BinaryOutput;
  typename OutputImageType::Pointer m_InputImage;
};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkTractsToFiberEndingsImageFilter.cpp"
#endif

#endif // __itkTractsToFiberEndingsImageFilter_h__
