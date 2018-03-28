#ifndef itkMultiHistogramFilter_cpp
#define itkMultiHistogramFilter_cpp

#include <itkMultiHistogramFilter.h>

#include <itkNeighborhoodIterator.h>
#include <itkImageRegionIterator.h>
#include <itkImageIterator.h>
#include "itkMinimumMaximumImageCalculator.h"

template< class TInputImageType, class TOuputImageType>
itk::MultiHistogramFilter<TInputImageType, TOuputImageType>::MultiHistogramFilter():
m_Delta(0.6), m_Offset(-3.0), m_Bins(11), m_Size(5), m_UseImageIntensityRange(false)
{
  this->SetNumberOfRequiredOutputs(m_Bins);
  this->SetNumberOfRequiredInputs(0);

  for (int i = 0; i < m_Bins; ++i)
  {
    this->SetNthOutput( i, this->MakeOutput(i) );
  }
}

template< class TInputImageType, class TOuputImageType>
void
itk::MultiHistogramFilter<TInputImageType, TOuputImageType>::BeforeThreadedGenerateData()
{
  typedef itk::MinimumMaximumImageCalculator <TInputImageType>
    ImageCalculatorFilterType;

  if (m_UseImageIntensityRange)
  {
    typename ImageCalculatorFilterType::Pointer imageCalculatorFilter
      = ImageCalculatorFilterType::New();
    imageCalculatorFilter->SetImage(this->GetInput(0));
    imageCalculatorFilter->Compute();

    m_Offset = imageCalculatorFilter->GetMinimum();
    m_Delta = 1.0*(imageCalculatorFilter->GetMaximum() - imageCalculatorFilter->GetMinimum()) / (1.0*m_Bins);
  }

  InputImagePointer input = this->GetInput(0);
  for (int i = 0; i < m_Bins; ++i)
  {
    CreateOutputImage(input, this->GetOutput(i));
  }
}
template< class TInputImageType, class TOuputImageType>
void
itk::MultiHistogramFilter<TInputImageType, TOuputImageType>::ThreadedGenerateData(const OutputImageRegionType & outputRegionForThread, ThreadIdType /*threadId*/)
{
  double offset = m_Offset;// -3.0;
  double delta = m_Delta;// 0.6;

  typedef itk::ImageRegionIterator<TInputImageType> IteratorType;
  typedef itk::ConstNeighborhoodIterator<TInputImageType> ConstIteratorType;

  typename TInputImageType::SizeType size; size.Fill(m_Size);
  InputImagePointer input = this->GetInput(0);

//  MITK_INFO << "Creating output iterator";
  std::vector<IteratorType> iterVector;
  for (int i = 0; i < m_Bins; ++i)
  {
    IteratorType iter(this->GetOutput(i), outputRegionForThread);
    iterVector.push_back(iter);
  }

  ConstIteratorType inputIter(size, input, outputRegionForThread);
  while (!inputIter.IsAtEnd())
  {
    for (int i = 0; i < m_Bins; ++i)
    {
      iterVector[i].Set(0);
    }

    for (unsigned int i = 0; i < inputIter.Size(); ++i)
    {
      double value = inputIter.GetPixel(i);
      value -=  offset;
      value /= delta;
      auto pos = (int)(value);
      pos = std::max(0, std::min(m_Bins-1, pos));
      iterVector[pos].Value() += 1;// (iterVector[pos].GetCenterPixel() + 1);
    }

    for (int i = 0; i < m_Bins; ++i)
    {
      ++(iterVector[i]);
    }
    ++inputIter;
  }
}

template< class TInputImageType, class TOuputImageType>
itk::ProcessObject::DataObjectPointer
  itk::MultiHistogramFilter<TInputImageType, TOuputImageType>::MakeOutput(itk::ProcessObject::DataObjectPointerArraySizeType /*idx*/)
{
  itk::ProcessObject::DataObjectPointer output;
  output = ( TOuputImageType::New() ).GetPointer();
  return output;
}

template< class TInputImageType, class TOuputImageType>
void
  itk::MultiHistogramFilter<TInputImageType, TOuputImageType>::CreateOutputImage(InputImagePointer input, OutputImagePointer output)
{
  output->SetRegions(input->GetLargestPossibleRegion());
  output->Allocate();
}

#endif //itkMultiHistogramFilter_cpp
