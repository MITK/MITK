#ifndef itkMultiHistogramFilter_cpp
#define itkMultiHistogramFilter_cpp

#include <itkMultiHistogramFilter.h>

#include <itkNeighborhoodIterator.h>
#include <itkImageIterator.h>

template< class TInputImageType, class TOuputImageType>
itk::MultiHistogramFilter<TInputImageType, TOuputImageType>::MultiHistogramFilter():
  m_Delta(0.6), m_Offset(-3.0)
{
  this->SetNumberOfRequiredOutputs(11);
  this->SetNumberOfRequiredInputs(0);

  for (unsigned int i = 0; i < 11; ++i)
  {
    this->SetNthOutput( i, this->MakeOutput(i) );
  }
}

template< class TInputImageType, class TOuputImageType>
void
  itk::MultiHistogramFilter<TInputImageType, TOuputImageType>::GenerateData()
{
  double offset = m_Offset;// -3.0;
  double delta = m_Delta;// 0.6;

  typedef itk::NeighborhoodIterator<TInputImageType> IteratorType;
  typedef itk::ConstNeighborhoodIterator<TInputImageType> ConstIteratorType;

  InputImagePointer input = this->GetInput(0);
  CreateOutputImage(input, this->GetOutput(0));
  CreateOutputImage(input, this->GetOutput(1));
  CreateOutputImage(input, this->GetOutput(2));
  CreateOutputImage(input, this->GetOutput(3));
  CreateOutputImage(input, this->GetOutput(4));
  CreateOutputImage(input, this->GetOutput(5));
  CreateOutputImage(input, this->GetOutput(6));
  CreateOutputImage(input, this->GetOutput(7));
  CreateOutputImage(input, this->GetOutput(8));
  CreateOutputImage(input, this->GetOutput(9));
  CreateOutputImage(input, this->GetOutput(10));

  typename TInputImageType::SizeType size; size.Fill(5);
  std::vector<IteratorType> iterVector;
  for (int i = 0; i < 11; ++i)
  {
    IteratorType iter(size, this->GetOutput(i), this->GetOutput(i)->GetLargestPossibleRegion());
    iterVector.push_back(iter);
  }

  ConstIteratorType inputIter( size, input, input->GetLargestPossibleRegion());
  while (!inputIter.IsAtEnd())
  {
    for (unsigned int i = 0; i < 11; ++i)
    {
      iterVector[i].SetCenterPixel(0);
    }

    for (unsigned int i = 0; i < inputIter.Size(); ++i)
    {
      double value = inputIter.GetPixel(i);
      value -=  offset;
      value /= delta;
      auto pos = (int)(value);
      pos = std::max(0, std::min(10, pos));
      iterVector[pos].SetCenterPixel(iterVector[pos].GetCenterPixel() + 1);
    }

    for (unsigned int i = 0; i < 11; ++i)
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
