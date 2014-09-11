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
#ifndef __itkTractDensityImageFilter_h__
#define __itkTractDensityImageFilter_h__

#include <itkImageSource.h>
#include <itkImage.h>
#include <itkVectorContainer.h>
#include <itkRGBAPixel.h>
#include <mitkFiberBundleX.h>

namespace itk{

/**
* \brief Generates tract density images from input fiberbundles (Calamante 2010).   */

template< class OutputImageType >
class TractDensityImageFilter : public ImageSource< OutputImageType >
{

public:
  typedef TractDensityImageFilter Self;
  typedef ProcessObject Superclass;
  typedef SmartPointer< Self > Pointer;
  typedef SmartPointer< const Self > ConstPointer;

  typedef typename OutputImageType::PixelType OutPixelType;

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)
  itkTypeMacro( TractDensityImageFilter, ImageSource )

  itkSetMacro( UpsamplingFactor, float)                         ///< use higher resolution for ouput image
  itkGetMacro( UpsamplingFactor, float)                         ///< use higher resolution for ouput image
  itkSetMacro( InvertImage, bool)                               ///< voxelvalue = 1-voxelvalue
  itkGetMacro( InvertImage, bool)                               ///< voxelvalue = 1-voxelvalue
  itkSetMacro( BinaryOutput, bool)                              ///< generate binary fiber envelope
  itkGetMacro( BinaryOutput, bool)                              ///< generate binary fiber envelope
  itkSetMacro( OutputAbsoluteValues, bool)                      ///< output absolute values of the number of fibers per voxel
  itkGetMacro( OutputAbsoluteValues, bool)                      ///< output absolute values of the number of fibers per voxel
  itkSetMacro( UseImageGeometry, bool)                          ///< use input image geometry to initialize output image
  itkGetMacro( UseImageGeometry, bool)                          ///< use input image geometry to initialize output image
  itkSetMacro( FiberBundle, mitk::FiberBundleX::Pointer)        ///< input fiber bundle
  itkSetMacro( InputImage, typename OutputImageType::Pointer)   ///< use input image geometry to initialize output image
  itkSetMacro( UseTrilinearInterpolation, bool )

  void GenerateData();

protected:

  itk::Point<float, 3> GetItkPoint(double point[3]);

  TractDensityImageFilter();
  virtual ~TractDensityImageFilter();

  typename OutputImageType::Pointer m_InputImage;           ///< use input image geometry to initialize output image
  mitk::FiberBundleX::Pointer       m_FiberBundle;          ///< input fiber bundle
  float                             m_UpsamplingFactor;     ///< use higher resolution for ouput image
  bool                              m_InvertImage;          ///< voxelvalue = 1-voxelvalue
  bool                              m_BinaryOutput;         ///< generate binary fiber envelope
  bool                              m_UseImageGeometry;     ///< use input image geometry to initialize output image
  bool                              m_OutputAbsoluteValues; ///< do not normalize image values to 0-1
  bool                              m_UseTrilinearInterpolation;
};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkTractDensityImageFilter.cpp"
#endif

#endif // __itkTractDensityImageFilter_h__
