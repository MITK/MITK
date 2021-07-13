#ifndef MITK_MinMaxLabelImageFilterWithIndex_HXX
#define MITK_MinMaxLabelImageFilterWithIndex_HXX

#include <mitkMinMaxLabelmageFilterWithIndex.h>
#include <limits>

namespace itk
{


template< typename TInputImage, typename TLabelImage >
void MinMaxLabelImageFilterWithIndex< TInputImage, TLabelImage >::AllocateOutputs()
{
  // Pass the input through as the output
  typename TInputImage::Pointer image =
    const_cast< TInputImage * >( this->GetInput() );

  this->GraftOutput(image);

  // Nothing that needs to be allocated for the remaining outputs
}

template< typename TInputImage, typename TLabelImage >
void MinMaxLabelImageFilterWithIndex< TInputImage, TLabelImage >::ThreadedGenerateData(const RegionType &
                                      outputRegionForThread,
                                      ThreadIdType threadId)
{
  const SizeValueType size0 = outputRegionForThread.GetSize(0);
  if( size0 == 0)
    {
    return;
    }
  PixelType value;
  LabelPixelType label;

  ExtremaMapType threadExtrema;
  ExtremaMapTypeIterator threadExtremaIt;

  ImageRegionConstIteratorWithIndex< TInputImage > it (this->GetInput(), outputRegionForThread);
  ImageRegionConstIteratorWithIndex< TLabelImage > labelit (this->GetLabelInput(), outputRegionForThread);

  // do the work
  while ( !it.IsAtEnd() )
  {
    value = it.Get();
    label = labelit.Get();

    threadExtremaIt = threadExtrema.find(label);

    // if label does not exist yet, create a new entry in the map.
    if (threadExtremaIt == threadExtrema.end())
    {
      threadExtremaIt = threadExtrema.insert( MapValueType(label, LabelExtrema()) ).first;
    }

    if (value < (*threadExtremaIt).second.m_Min)
    {
      (*threadExtremaIt).second.m_Min = value;
      (*threadExtremaIt).second.m_MinIndex = it.GetIndex();
    }
    if (value > (*threadExtremaIt).second.m_Max)
    {
      (*threadExtremaIt).second.m_Max = value;
      (*threadExtremaIt).second.m_MaxIndex = it.GetIndex();
    }
    ++it;
    ++labelit;
  }

  m_ThreadExtrema[threadId] = threadExtrema;
}

template< typename TInputImage, typename TLabelImage >
void MinMaxLabelImageFilterWithIndex< TInputImage, TLabelImage >::BeforeThreadedGenerateData()
{
  ThreadIdType numberOfThreads = this->GetNumberOfWorkUnits();
  m_ThreadExtrema.resize(numberOfThreads);

  for (unsigned int i =0; i < numberOfThreads; i++)
  {
    m_ThreadExtrema[i] = ExtremaMapType();
  }
}

template< typename TInputImage, typename TLabelImage >
void MinMaxLabelImageFilterWithIndex< TInputImage, TLabelImage >::AfterThreadedGenerateData()
{
  ThreadIdType numberOfThreads = this->GetNumberOfWorkUnits();

  m_GlobalMin = std::numeric_limits<PixelType>::max();
  m_GlobalMax = std::numeric_limits<PixelType>::min();

  ExtremaMapTypeIterator it;

  for (ThreadIdType i = 0; i < numberOfThreads; i++)
  {
    for (auto&& it2 : m_ThreadExtrema[i])
    {
      it = m_LabelExtrema.find(it2.first);
      if (it == m_LabelExtrema.end())
      {
        it = m_LabelExtrema.insert( MapValueType(it2.first, LabelExtrema()) ).first;
      }

      if (it2.second.m_Min < (*it).second.m_Min)
      {
        (*it).second.m_Min = it2.second.m_Min;
        (*it).second.m_MinIndex = it2.second.m_MinIndex;
        if (it2.second.m_Min < m_GlobalMin)
        {
          m_GlobalMin = it2.second.m_Min;
          m_GlobalMinIndex = it2.second.m_MinIndex;
        }
      }

      if (it2.second.m_Max > (*it).second.m_Max)
      {
        (*it).second.m_Max = it2.second.m_Max;
        (*it).second.m_MaxIndex = it2.second.m_MaxIndex;
        if (it2.second.m_Max > m_GlobalMax)
        {
          m_GlobalMax = it2.second.m_Max;
          m_GlobalMaxIndex = it2.second.m_MaxIndex;
        }
      }
    }
  }
}
}
#endif
