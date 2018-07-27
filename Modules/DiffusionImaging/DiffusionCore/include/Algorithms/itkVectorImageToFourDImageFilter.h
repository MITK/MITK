/*=========================================================================

  Program:   Insight Segmentation & Registration Toolkit
  Module:    $RCSfile: itkDiffusionTensor3DReconstructionImageFilter.h,v $
  Language:  C++
  Date:      $Date: 2006-03-27 17:01:06 $
  Version:   $Revision: 1.12 $

  Copyright (c) Insight Software Consortium. All rights reserved.
  See ITKCopyright.txt or http://www.itk.org/HTML/Copyright.htm for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/
#ifndef __itkVectorImageToFourDImageFilter_h_
#define __itkVectorImageToFourDImageFilter_h_

#include "itkImageToImageFilter.h"
#include <itkImage.h>
#include <itkVectorImage.h>

namespace itk{

/**
* \brief Flip peaks along specified axes
*/

template< class PixelType >
class VectorImageToFourDImageFilter : public ProcessObject
{

public:

  typedef VectorImageToFourDImageFilter Self;
  typedef SmartPointer<Self>                      Pointer;
  typedef SmartPointer<const Self>                ConstPointer;
  typedef ProcessObject Superclass;

  /** Method for creation through the object factory. */
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  /** Runtime information support. */
  itkTypeMacro(VectorImageToFourDImageFilter, ProcessObject)

  typedef VectorImage<PixelType, 3>          InputImageType;
  typedef Image<PixelType, 4>    OutputImageType;
  typedef typename OutputImageType::RegionType  OutputImageRegionType;

  itkSetMacro(InputImage, typename InputImageType::Pointer)
  itkGetMacro(OutputImage, typename OutputImageType::Pointer)

  void GenerateData();

protected:
  VectorImageToFourDImageFilter();
  ~VectorImageToFourDImageFilter(){}

  typename InputImageType::Pointer m_InputImage;
  typename OutputImageType::Pointer m_OutputImage;

private:
};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkVectorImageToFourDImageFilter.cpp"
#endif

#endif //__itkVectorImageToFourDImageFilter_h_

