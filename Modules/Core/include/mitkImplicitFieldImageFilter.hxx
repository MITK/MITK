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
  typename TImageType::Pointer output = this->GetOutput();

  typename itk::MinimumMaximumImageCalculator<TImageType>::Pointer minMaxCalculator = itk::MinimumMaximumImageCalculator<TImageType>::New();
  minMaxCalculator->SetImage(input);
  minMaxCalculator->ComputeMinimum();
  m_MinimumPixelValue = minMaxCalculator->GetMinimum();

  if (!itk::InPlaceImageFilter<TImageType, TImageType>::GetInPlace()) {
    itk::ImageAlgorithm::Copy(input.GetPointer(), output.GetPointer(), input->GetLargestPossibleRegion(), input->GetLargestPossibleRegion());
  }
  if (m_RegionType == ResectionRegionType::INSIDE) {
    output->SetRequestedRegion(this->m_Region);
  }
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
  typename TImageType::Pointer output = this->GetOutput();

  typename TImageType::IndexType index;
  typename TImageType::PointType point;
  double pp2[3];

  itk::ProgressReporter progress(this, threadId, region.GetNumberOfPixels(), 100);

  itk::ImageRegionIteratorWithIndex<TImageType> it(output, region);

  it.GoToBegin();

  while (!it.IsAtEnd()) {
    index = it.GetIndex();
    if (m_RegionType == ResectionRegionType::OUTSIDE) {
      if (!m_Region.IsInside(index)) {
        it.Set(m_MinimumPixelValue);

        ++it;
        progress.CompletedPixel();
        continue;
      }
    }

    if (it.Get() != m_MinimumPixelValue) {
      input->TransformIndexToPhysicalPoint(index, point);

      for (int i = 0; i < 3; i++)
        pp2[i] = point[i];

      double val = m_ImplFunc->EvaluateFunction(pp2);
      if (m_RegionType == ResectionRegionType::INSIDE) {
        if (val <= 0) {
          it.Set(m_MinimumPixelValue);
        }
      }
      else {
        if (val >= 0) {
          it.Set(m_MinimumPixelValue);
        }
      }

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
void ImplicitFieldImageFilter<TImageType>::SetRegionType(ResectionRegionType type)
{
  m_RegionType = type;
}

template< typename TImageType>
ImplicitFieldImageFilter<TImageType>::ImplicitFieldImageFilter()
{
  m_RegionType = ResectionRegionType::INSIDE;
}

template< typename TImageType>
ImplicitFieldImageFilter<TImageType>::~ImplicitFieldImageFilter()
{
}
}

