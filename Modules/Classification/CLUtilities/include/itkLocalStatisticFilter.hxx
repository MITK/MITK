#ifndef itkLocalStatisticFilter_cpp
#define itkLocalStatisticFilter_cpp

#include <itkLocalStatisticFilter.h>

#include <itkNeighborhoodIterator.h>
#include <itkImageRegionIterator.h>
#include <itkImageIterator.h>
#include "itkMinimumMaximumImageCalculator.h"

#include <limits>

template< class TInputImageType, class TOuputImageType>
itk::LocalStatisticFilter<TInputImageType, TOuputImageType>::LocalStatisticFilter():
     m_Size(5), m_Bins(5)
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
itk::LocalStatisticFilter<TInputImageType, TOuputImageType>::BeforeThreadedGenerateData()
{
  InputImagePointer input = this->GetInput(0);
  for (int i = 0; i < m_Bins; ++i)
  {
    CreateOutputImage(input, this->GetOutput(i));
  }
}

template< class TInputImageType, class TOuputImageType>
void
itk::LocalStatisticFilter<TInputImageType, TOuputImageType>::ThreadedGenerateData(const OutputImageRegionType & outputRegionForThread, ThreadIdType /*threadId*/)
{
  typedef itk::ImageRegionIterator<TInputImageType> IteratorType;
  typedef itk::ConstNeighborhoodIterator<TInputImageType> ConstIteratorType;

  typename TInputImageType::SizeType size; size.Fill(m_Size);
  InputImagePointer input = this->GetInput(0);

  if (TInputImageType::ImageDimension == 3)
  {
    size[2] = 0;
  }

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

    double min = std::numeric_limits<double>::max();
    double max = std::numeric_limits<double>::lowest();
    double mean = 0;
    double std = 0;

    for (unsigned int i = 0; i < inputIter.Size(); ++i)
    {
      double value = inputIter.GetPixel(i);
      min = std::min<double>(min, value);
      max = std::max<double>(max, value);
      mean += value / inputIter.Size();
      std += (value*value) / inputIter.Size();
    }

    iterVector[0].Value() = min;
    iterVector[1].Value() = max;
    iterVector[2].Value() = mean;
    iterVector[3].Value() = std::sqrt(std - mean*mean);
    iterVector[4].Value() = max-min;

    for (int i = 0; i < m_Bins; ++i)
    {
      ++(iterVector[i]);
    }
    ++inputIter;
  }
}

template< class TInputImageType, class TOuputImageType>
itk::ProcessObject::DataObjectPointer
  itk::LocalStatisticFilter<TInputImageType, TOuputImageType>::MakeOutput(itk::ProcessObject::DataObjectPointerArraySizeType /*idx*/)
{
  itk::ProcessObject::DataObjectPointer output;
  output = ( TOuputImageType::New() ).GetPointer();
  return output;
}

template< class TInputImageType, class TOuputImageType>
void
  itk::LocalStatisticFilter<TInputImageType, TOuputImageType>::CreateOutputImage(InputImagePointer input, OutputImagePointer output)
{
  output->SetRegions(input->GetLargestPossibleRegion());
  output->Allocate();
}

#endif //itkLocalStatisticFilter_cpp
