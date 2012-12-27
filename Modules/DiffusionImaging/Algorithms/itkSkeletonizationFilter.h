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

#ifndef ITKSKELETONIZATIONFILTER_H_
#define ITKSKELETONIZATIONFILTER_H_

#include "itkImageToImageFilter.h"
#include "itkImage.h"
#include "mitkImage.h"
#include <itkVectorImage.h>

namespace itk
{
template < class TInputImage, class TOutputImage >
class SkeletonizationFilter : public ImageToImageFilter<TInputImage, TOutputImage>
{
  /*!
  \brief itkSkeletonizationFilter

  \brief Skeletonization algorithm from TBSS (Smith et al. 2006)

  \sa itkImageToImageFilter


  */

public:

  /** Typedef for input ImageType. */
  typedef TInputImage  InputImageType;

  typedef itk::CovariantVector<int,3> VectorType;

  typedef itk::Image<VectorType, 3> VectorImageType;


  /** Typedef for output ImageType. */
  typedef TOutputImage OutputImageType;

  typedef itk::VectorImage<int, 3> GradientImageType;


  /** */
  typedef SkeletonizationFilter Self;

  /** Superclass */
  typedef ImageToImageFilter<InputImageType, OutputImageType> Superclass;

  /** Smart Pointer */
  typedef SmartPointer<Self> Pointer;

  /** Smart Pointer */
  typedef SmartPointer<const Self> ConstPointer;

  /** */
  itkNewMacro( Self);

  /** Generate Data. The image will be divided into a number of pieces, a number of threads
  will be spawned and Threaded GenerateData() will be called in each thread. */
  virtual void GenerateData();


  GradientImageType::Pointer GetGradientImage();

  VectorImageType::Pointer GetVectorImage()
  {
    return m_DirectionImage;
  }




protected:

  /** Constructor */
  SkeletonizationFilter();

  /** Destructor */
  virtual ~SkeletonizationFilter();

  void CalculatePerpendicularDirections();

  VectorImageType::Pointer m_DirectionImage;

  //FloatVectorImageType::Pointer m_FixedDirImage;

  int round(float x)
  {
    if (x>0.0) return ((int) (x+0.5));
    else       return ((int) (x-0.5));
  }


protected:



};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkSkeletonizationFilter.txx"
#endif

#endif
