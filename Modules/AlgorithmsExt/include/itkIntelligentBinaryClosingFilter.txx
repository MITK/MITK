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
#ifndef _itkIntelligentBinaryClosingFilter_txx
#define _itkIntelligentBinaryClosingFilter_txx

#include "itkIntelligentBinaryClosingFilter.h"
#include <vector>

namespace itk
{
  template <class TInputImage, class TOutputImage>
  IntelligentBinaryClosingFilter<TInputImage, TOutputImage>::IntelligentBinaryClosingFilter(): m_ClosingRadius(4.0), m_SurfaceRatio(70)
  {
    m_ErodeImageFilter = BinaryErodeImageFilterType::New();
    m_DilateImageFilter = BinaryDilateImageFilterType::New();
    m_SubtractImageFilter = SubtractImageFilterType::New();
    m_ConnectedComponentImageFilter = ConnectedComponentImageFilterType::New();
    m_RelabelComponentImageFilter = RelabelComponentImageFilterType::New();
    m_RelabelComponentImageFilter->SetInPlace(true);
    m_BorderDetectionDilateFilter = DilateComponentImageFilterType::New();
  }

  template <class TInputImage, class TOutputImage>
  void IntelligentBinaryClosingFilter<TInputImage, TOutputImage>::GenerateData()
  {
    const InputImageType *input = this->GetInput();
    // Allocate the output image.
    typename OutputImageType::Pointer output = this->GetOutput();
    output->SetRequestedRegion(input->GetRequestedRegion());
    output->SetBufferedRegion(input->GetBufferedRegion());
    output->SetLargestPossibleRegion(input->GetLargestPossibleRegion());
    output->Allocate();

    // set up structuring element for closing
    StructuringElementType seClosing;
    itk::SizeValueType radius[ImageDimension];
    const typename InputImageType::SpacingType spacing = input->GetSpacing();
    for (unsigned int d = 0; d < ImageDimension; d++)
    { // closing works in voxel coordinates, so use spacing (and add 0.5 for correct rounding - cast just truncates)
      radius[d] = (unsigned long)(m_ClosingRadius / spacing[d] + 0.5);
    }
    MITK_INFO << "    Closing kernel size = [" << radius[0] << ", " << radius[1] << ", " << radius[2] << "]"
              << std::endl;
    seClosing.SetRadius(radius);
    seClosing.CreateStructuringElement();

    // closing
    m_DilateImageFilter->SetInput(this->GetInput());
    m_DilateImageFilter->SetKernel(seClosing);
    m_DilateImageFilter->SetDilateValue(1);
    m_ErodeImageFilter->SetInput(m_DilateImageFilter->GetOutput());
    m_ErodeImageFilter->SetKernel(seClosing);
    m_ErodeImageFilter->SetErodeValue(1);

    // subtraction
    m_SubtractImageFilter->SetInput1(m_ErodeImageFilter->GetOutput());
    m_SubtractImageFilter->SetInput2(this->GetInput());

    // connected components
    m_ConnectedComponentImageFilter->SetInput(m_SubtractImageFilter->GetOutput());
    m_RelabelComponentImageFilter->SetInput(m_ConnectedComponentImageFilter->GetOutput());
    m_RelabelComponentImageFilter->Update();

    // set up structuring element for border voxel detection
    StructuringElementType seBorder;
    for (auto &radiu : radius)
    {
      radiu = 1;
    }
    seBorder.SetRadius(radius);
    seBorder.CreateStructuringElement();

    // dilate all components to detect border voxels
    m_BorderDetectionDilateFilter->SetInput(m_RelabelComponentImageFilter->GetOutput());
    m_BorderDetectionDilateFilter->SetKernel(seBorder);
    m_BorderDetectionDilateFilter->Update();

    // count volumes and border voxels for all components
    OutputIteratorType itComp(m_RelabelComponentImageFilter->GetOutput(),
                              m_RelabelComponentImageFilter->GetOutput()->GetLargestPossibleRegion());
    OutputIteratorType itBorder(m_BorderDetectionDilateFilter->GetOutput(),
                                m_BorderDetectionDilateFilter->GetOutput()->GetLargestPossibleRegion());
    ConstInputIteratorType itIn(input, input->GetLargestPossibleRegion());

    std::vector<unsigned int> volume(m_RelabelComponentImageFilter->GetNumberOfObjects() + 1, 0);
    std::vector<unsigned int> border(m_RelabelComponentImageFilter->GetNumberOfObjects() + 1, 0);
    std::vector<unsigned int> adjacent(m_RelabelComponentImageFilter->GetNumberOfObjects() + 1, 0);
    typename OutputImageType::ValueType borderId, compId;
    for (itComp.GoToBegin(), itBorder.GoToBegin(), itIn.GoToBegin(); !itComp.IsAtEnd(); ++itComp, ++itBorder, ++itIn)
    {
      borderId = itBorder.Get();
      if (borderId != 0)
      {
        compId = itComp.Get();
        if (compId != 0)
        {
          volume[compId]++;
        }
        else
        {
          // this is border country
          border[borderId]++;
          if (itIn.Get() != 0)
            adjacent[borderId]++;
        }
      }
    }

    // calculate ratios
    std::vector<float> ratio(m_RelabelComponentImageFilter->GetNumberOfObjects() + 1, 0);
    MITK_INFO << "    " << m_RelabelComponentImageFilter->GetNumberOfObjects() << " components found" << std::endl;
    for (unsigned int i = 0; i < ratio.size(); i++)
    {
      if (border[i] != 0)
        ratio[i] = 100.0 * (float)(adjacent[i]) / (float)(border[i]);
    }

    // fill output
    OutputIteratorType itOut(output, output->GetLargestPossibleRegion());
    for (itOut.GoToBegin(), itIn.GoToBegin(), itComp.GoToBegin(); !itOut.IsAtEnd(); ++itOut, ++itIn, ++itComp)
    {
      if (itIn.Get() != 0)
      {
        itOut.Set(1);
      }
      else
      {
        compId = itComp.Get();
        if (ratio[compId] > m_SurfaceRatio)
          itOut.Set(1);
        else
          itOut.Set(0);
      }
    }
  }

  template <class TInputImage, class TOutputImage>
  void IntelligentBinaryClosingFilter<TInputImage, TOutputImage>::PrintSelf(std::ostream &os, Indent indent) const
  {
    Superclass::PrintSelf(os, indent);
  }

} // end namespace itk

#endif
