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
#ifndef __itkSplitVectorImageFilter_h_
#define __itkSplitVectorImageFilter_h_

#include "itkImageToImageFilter.h"
#include <itkImage.h>
#include <itkVectorImage.h>

namespace itk{

/**
* \brief Flip peaks along specified axes
*/

template< class PixelType >
class SplitVectorImageFilter : public ProcessObject
{

public:

  typedef SplitVectorImageFilter Self;
  typedef SmartPointer<Self>                      Pointer;
  typedef SmartPointer<const Self>                ConstPointer;
  typedef ProcessObject Superclass;

  /** Method for creation through the object factory. */
  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  /** Runtime information support. */
  itkTypeMacro(SplitVectorImageFilter, ProcessObject)

  typedef VectorImage<PixelType, 3>   InputImageType;
  typedef Image<PixelType, 3>    OutputImageType;
  typedef typename OutputImageType::RegionType  OutputImageRegionType;

  itkSetMacro(InputImage, typename InputImageType::Pointer)
  std::vector< typename OutputImageType::Pointer > GetOutputImages(){ return m_OutputImages; }
  void GenerateData();

protected:
  SplitVectorImageFilter();
  ~SplitVectorImageFilter(){}

  typename InputImageType::Pointer m_InputImage;
  std::vector< typename OutputImageType::Pointer > m_OutputImages;

private:
};

}

#ifndef ITK_MANUAL_INSTANTIATION
#include "itkSplitVectorImageFilter.cpp"
#endif

#endif //__itkSplitVectorImageFilter_h_

