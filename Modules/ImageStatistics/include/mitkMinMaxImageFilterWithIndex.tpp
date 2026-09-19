#ifndef mitkMinMaxImageFilterWithIndex_tpp
#define mitkMinMaxImageFilterWithIndex_tpp

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
  if (outputRegionForThread.GetNumberOfPixels() == 0)
  {
    return;
  }

  ImageRegionConstIteratorWithIndex< TInputImage > it (this->GetInput(), outputRegionForThread);

  // The first pixel seeds the extrema. Seeding with numeric limits instead would
  // never accept a region whose pixels all equal that limit, which for unsigned
  // types is an all-zero region.
  ThreadExtrema extrema{ it.Get(), it.Get(), it.GetIndex(), it.GetIndex() };

  for (++it; !it.IsAtEnd(); ++it)
  {
    const PixelType value = it.Get();

    if (value < extrema.m_Min)
    {
      extrema.m_Min = value;
      extrema.m_MinIndex = it.GetIndex();
    }
    if (value > extrema.m_Max)
    {
      extrema.m_Max = value;
      extrema.m_MaxIndex = it.GetIndex();
    }
  }

  m_ThreadExtrema[threadId] = extrema;
}

template< typename TInputImage >
void MinMaxImageFilterWithIndex< TInputImage >::BeforeThreadedGenerateData()
{
  m_ThreadExtrema.assign(this->GetNumberOfWorkUnits(), std::nullopt);

  m_Min = std::numeric_limits<PixelType>::max();
  m_Max = std::numeric_limits<PixelType>::lowest();
  m_MinIndex.Fill(0);
  m_MaxIndex.Fill(0);
}

template< typename TInputImage >
void MinMaxImageFilterWithIndex< TInputImage >::AfterThreadedGenerateData()
{
  // The first thread with a region seeds the result, so ties resolve to the
  // lowest thread, i.e. the first pixel in scan order.
  bool first = true;

  for (const auto& threadExtrema : m_ThreadExtrema)
  {
    if (!threadExtrema.has_value())
    {
      continue;
    }

    if (first || threadExtrema->m_Min < m_Min)
    {
      m_Min = threadExtrema->m_Min;
      m_MinIndex = threadExtrema->m_MinIndex;
    }
    if (first || threadExtrema->m_Max > m_Max)
    {
      m_Max = threadExtrema->m_Max;
      m_MaxIndex = threadExtrema->m_MaxIndex;
    }

    first = false;
  }
}
}
#endif
