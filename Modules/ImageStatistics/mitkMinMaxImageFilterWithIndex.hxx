#ifndef MITK_MinMaxImageFilterWithIndex_HXX
#define MITK_MinMaxImageFilterWithIndex_HXX

#include <mitkMinMaxImageFilterWithIndex.h>
#include <limits>

namespace itk
{


template< typename TInputImage >
void MinMaxImageFilterWithIndex< TInputImage >::AllocateOutputs()
{
  // Pass the input through as the output
  typename TInputImage::Pointer image =
    const_cast< TInputImage * >( this->GetInput() );

  this->GraftOutput(image);

  // Nothing that needs to be allocated for the remaining outputs
}

template< typename TInputImage >
void MinMaxImageFilterWithIndex< TInputImage >::ThreadedGenerateData(const RegionType &
                                      outputRegionForThread,
                                      ThreadIdType threadId)
{
  const SizeValueType size0 = outputRegionForThread.GetSize(0);
  if( size0 == 0)
    {
    return;
    }
  PixelType value;

  PixelType threadMin, threadMax;
  IndexType threadMinIndex;
  IndexType threadMaxIndex;
  threadMinIndex.Fill(0);
  threadMaxIndex.Fill(0);

  threadMin = std::numeric_limits<PixelType>::max();
  threadMax = std::numeric_limits<PixelType>::min();

  ImageRegionConstIteratorWithIndex< TInputImage > it (this->GetInput(), outputRegionForThread);

  // do the work
  while ( !it.IsAtEnd() )
  {
    value = it.Get();
    if (value < threadMin)
    {
      threadMin = value;
      threadMinIndex = it.GetIndex();
    }
    if (value > threadMax)
    {
      threadMax = value;
      threadMaxIndex = it.GetIndex();
    }
    ++it;
  }

  m_ThreadMax[threadId] = threadMax;
  m_ThreadMin[threadId] = threadMin;
  m_ThreadMaxIndex[threadId] = threadMaxIndex;
  m_ThreadMinIndex[threadId] = threadMinIndex;
}

template< typename TInputImage >
void MinMaxImageFilterWithIndex< TInputImage >::BeforeThreadedGenerateData()
{
  ThreadIdType numberOfThreads = this->GetNumberOfWorkUnits();
  m_ThreadMin.resize(numberOfThreads);
  m_ThreadMax.resize(numberOfThreads);
  m_ThreadMinIndex.resize(numberOfThreads);
  m_ThreadMaxIndex.resize(numberOfThreads);

  for (unsigned int i =0; i < numberOfThreads; i++)
  {
    m_ThreadMin[i] = std::numeric_limits<PixelType>::max();
    m_ThreadMax[i] = std::numeric_limits<PixelType>::min();
  }

  m_Min = std::numeric_limits<PixelType>::max();
  m_Max = std::numeric_limits<PixelType>::min();

}

template< typename TInputImage >
void MinMaxImageFilterWithIndex< TInputImage >::AfterThreadedGenerateData()
{
  ThreadIdType numberOfThreads = this->GetNumberOfWorkUnits();

  for (ThreadIdType i = 0; i < numberOfThreads; i++)
  {
    if (m_ThreadMin[i] < m_Min)
    {
      m_Min = m_ThreadMin[i];
      m_MinIndex = m_ThreadMinIndex[i];
    }
    if (m_ThreadMax[i] > m_Max)
    {
      m_Max = m_ThreadMax[i];
      m_MaxIndex = m_ThreadMaxIndex[i];
    }
  }
}
}
#endif
