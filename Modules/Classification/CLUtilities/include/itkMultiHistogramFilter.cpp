#ifndef itkMultiHistogramFilter_cpp
#define itkMultiHistogramFilter_cpp

#include <itkMultiHistogramFilter.h>

#include <itkNeighborhoodIterator.h>
#include <itkImageIterator.h>
#include "itkMinimumMaximumImageCalculator.h"

template< class TInputImageType, class TOuputImageType>
itk::MultiHistogramFilter<TInputImageType, TOuputImageType>::MultiHistogramFilter():
m_Offset(-3.0), m_Delta(0.6), m_Bins(11), m_Size(5), m_UseImageIntensityRange(false)
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
  itk::MultiHistogramFilter<TInputImageType, TOuputImageType>::GenerateData()
{
  typedef itk::MinimumMaximumImageCalculator <TInputImageType>
    ImageCalculatorFilterType;

  double offset = m_Offset;// -3.0;
  double delta = m_Delta;// 0.6;

  if (m_UseImageIntensityRange)
  {
    ImageCalculatorFilterType::Pointer imageCalculatorFilter
      = ImageCalculatorFilterType::New();
    imageCalculatorFilter->SetImage(this->GetInput(0));
    imageCalculatorFilter->Compute();

    offset = imageCalculatorFilter->GetMinimum();
    delta = 1.0*(imageCalculatorFilter->GetMaximum() - imageCalculatorFilter->GetMinimum()) / (1.0*m_Bins);
  }

  std::cout << "Offset: " << offset << " delta: " << delta << std::endl;

  typedef itk::NeighborhoodIterator<TInputImageType> IteratorType;
  typedef itk::ConstNeighborhoodIterator<TInputImageType> ConstIteratorType;

  InputImagePointer input = this->GetInput(0);
  for (int i = 0; i < m_Bins; ++i)
  {
    CreateOutputImage(input, this->GetOutput(i));
  }

  typename TInputImageType::SizeType size; size.Fill(m_Size);
  std::vector<IteratorType> iterVector;
  for (int i = 0; i < m_Bins; ++i)
  {
    IteratorType iter(size, this->GetOutput(i), this->GetOutput(i)->GetLargestPossibleRegion());
    iterVector.push_back(iter);
  }

  ConstIteratorType inputIter( size, input, input->GetLargestPossibleRegion());
  while (!inputIter.IsAtEnd())
  {
    for (int i = 0; i < m_Bins; ++i)
    {
      iterVector[i].SetCenterPixel(0);
    }

    for (int i = 0; i < inputIter.Size(); ++i)
    {
      double value = inputIter.GetPixel(i);
      value -=  offset;
      value /= delta;
      int pos = (int)(value);
      pos = std::max(0, std::min(m_Bins-1, pos));
      iterVector[pos].SetCenterPixel(iterVector[pos].GetCenterPixel() + 1);
    }

    for (int i = 0; i < m_Bins; ++i)
    {
      ++(iterVector[i]);
    }
    ++inputIter;
  }
}

template< class TInputImageType, class TOuputImageType>
itk::DataObject::Pointer
  itk::MultiHistogramFilter<TInputImageType, TOuputImageType>::MakeOutput(unsigned int /*idx*/)
{
  DataObject::Pointer output;
  output = ( TOuputImageType::New() ).GetPointer();
  return output.GetPointer();
}

template< class TInputImageType, class TOuputImageType>
void
  itk::MultiHistogramFilter<TInputImageType, TOuputImageType>::CreateOutputImage(InputImagePointer input, OutputImagePointer output)
{
  output->SetRegions(input->GetLargestPossibleRegion());
  output->Allocate();
}

#endif //itkMultiHistogramFilter_cpp