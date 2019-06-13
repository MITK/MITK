#pragma once

#include <itkObjectFactory.h>
#include <itkImageRegionIterator.h>
#include <itkImageRegionConstIterator.h>
#include <itkMinimumMaximumImageCalculator.h>
#include <itkProgressReporter.h>

namespace mitk
{
template< typename TImageType>
void ImplicitFieldImageFilter<TImageType>::BeforeThreadedGenerateData()
{
  typename TImageType::ConstPointer input = this->GetInput();
  typename itk::Image<char, 3>::Pointer output = this->GetOutput();

  output->FillBuffer(0);
  output->SetRequestedRegion(this->m_Region);
}

template< typename TImageType>
void ImplicitFieldImageFilter<TImageType>::ThreadedGenerateData(const OutputImageRegionType & region, itk::ThreadIdType threadId)
{
  ResectionFunction(region, threadId);
}

template< typename TImageType>
void ImplicitFieldImageFilter<TImageType>::ResectionFunction(const OutputImageRegionType & region, itk::ThreadIdType threadId)
{
  typename TImageType::ConstPointer input = this->GetInput();
  typename itk::Image<char, 3>::Pointer output = this->GetOutput();

  typename TImageType::IndexType index;
  typename TImageType::PointType point;
  double pp2[3];

  itk::ProgressReporter progress(this, threadId, region.GetNumberOfPixels(), 100);

  itk::ImageRegionIteratorWithIndex<itk::Image<char, 3>> it(output, region);

  it.GoToBegin();

  while (!it.IsAtEnd()) {
    index = it.GetIndex();

    input->TransformIndexToPhysicalPoint(index, point);

    for (int i = 0; i < 3; i++) {
      pp2[i] = point[i];
    }

    double val = m_ImplFunc->EvaluateFunction(pp2);
    if (val <= 0) {
      it.Set(1);
    }

    ++it;
    progress.CompletedPixel();
  }
}

template< typename TImageType>
void ImplicitFieldImageFilter<TImageType>::SetRegion(ImageRegionType region)
{
  m_Region = region;
}

template< typename TImageType>
void ImplicitFieldImageFilter<TImageType>::SetImplicitFunction(vtkImplicitFunction* f)
{
  m_ImplFunc = f;
}

template< typename TImageType>
ImplicitFieldImageFilter<TImageType>::ImplicitFieldImageFilter()
{
}

template< typename TImageType>
ImplicitFieldImageFilter<TImageType>::~ImplicitFieldImageFilter()
{
}
}

