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

/**
  * \brief Skeletonization part of the TBSS pipeline
  *
  * This class takes a 3D image which is typically the mean FA image that is calculated after registration by the FSL TBSS pipeline (see
  * http://fsl.fmrib.ox.ac.uk/fsl/fsl4.0/tbss/index for the user manual) and performs the non-maximum-suppression to create a white matter skeleton.
  *
  *
  * The skeletonization algorithm is described in Smith et al., 2009. http://dx.doi.org/10.1016/j.neuroimage.2006.02.024 )
  */


template < class TInputImage, class TOutputImage >
class SkeletonizationFilter : public ImageToImageFilter<TInputImage, TOutputImage>
{


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

  typedef SmartPointer<Self> Pointer;

  typedef SmartPointer<const Self> ConstPointer;

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  /** \brief Performs the work */
  void GenerateData() override;


  /** \brief Output the gradient image as itkVectorImage
   *
   * Output the gradient image by first converting it to an itk vector image
   */
  GradientImageType::Pointer GetGradientImage();

  /** \brief Output the gradient image as an itkImage containing vector */
  VectorImageType::Pointer GetVectorImage()
  {
    return m_DirectionImage;
  }




protected:

  SkeletonizationFilter();

  ~SkeletonizationFilter() override;

  VectorImageType::Pointer m_DirectionImage;

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
