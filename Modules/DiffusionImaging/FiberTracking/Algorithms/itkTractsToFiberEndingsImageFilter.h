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
#include <mitkFiberBundle.h>

namespace itk{

/**
* \brief Generates image where the pixel values are set according to the number of fibers ending in the voxel.   */

template< class OutputImageType >
class TractsToFiberEndingsImageFilter : public ImageSource< OutputImageType >
{

public:
  typedef TractsToFiberEndingsImageFilter Self;
  typedef ProcessObject Superclass;
  typedef SmartPointer< Self > Pointer;
  typedef SmartPointer< const Self > ConstPointer;

  typedef typename OutputImageType::PixelType OutPixelType;

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)
  itkTypeMacro( TractsToFiberEndingsImageFilter, ImageSource )

  /** Upsampling factor **/
  itkSetMacro( UpsamplingFactor, float)
  itkGetMacro( UpsamplingFactor, float)

  /** Invert Image **/
  itkSetMacro( InvertImage, bool)
  itkGetMacro( InvertImage, bool)

  itkSetMacro( FiberBundle, mitk::FiberBundle::Pointer)
  itkSetMacro( InputImage, typename OutputImageType::Pointer)

  /** Use input image geometry to initialize output image **/
  itkSetMacro( UseImageGeometry, bool)
  itkGetMacro( UseImageGeometry, bool)

  itkSetMacro( BinaryOutput, bool)

  void GenerateData() override;

protected:

  TractsToFiberEndingsImageFilter();
  virtual ~TractsToFiberEndingsImageFilter();

  mitk::FiberBundle::Pointer        m_FiberBundle;          ///< input fiber bundle
  float                             m_UpsamplingFactor;     ///< use higher resolution for ouput image
  bool                              m_InvertImage;          ///< voxelvalue = 1-voxelvalue
  bool                              m_UseImageGeometry;     ///< output image is given other geometry than fiberbundle (input image geometry)
  bool                              m_BinaryOutput;
  typename OutputImageType::Pointer m_InputImage;
};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkTractsToFiberEndingsImageFilter.cpp"
#endif

#endif // __itkTractsToFiberEndingsImageFilter_h__
