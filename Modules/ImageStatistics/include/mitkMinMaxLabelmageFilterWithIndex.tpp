#ifndef mitkMinMaxLabelmageFilterWithIndex_tpp
#define mitkMinMaxLabelmageFilterWithIndex_tpp

#include <mitkMinMaxLabelmageFilterWithIndex.h>
#include <limits>
#include <utility>

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
  ExtremaMapType threadExtrema;

  ImageRegionConstIteratorWithIndex< TInputImage > it (this->GetInput(), outputRegionForThread);
  ImageRegionConstIteratorWithIndex< TLabelImage > labelit (this->GetLabelInput(), outputRegionForThread);

  for (; !it.IsAtEnd(); ++it, ++labelit)
  {
    const PixelType value = it.Get();

    // The first pixel of a label seeds its extrema. Seeding with numeric limits
    // instead would never accept a label whose pixels all equal that limit, which
    // for unsigned types is an all-zero label.
    auto [extremaIt, inserted] = threadExtrema.try_emplace(labelit.Get(), value, it.GetIndex());

    if (inserted)
    {
      continue;
    }

    LabelExtrema& extrema = extremaIt->second;

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

  m_ThreadExtrema[threadId] = std::move(threadExtrema);
}

template< typename TInputImage, typename TLabelImage >
void MinMaxLabelImageFilterWithIndex< TInputImage, TLabelImage >::BeforeThreadedGenerateData()
{
  m_ThreadExtrema.assign(this->GetNumberOfWorkUnits(), ExtremaMapType());
  m_LabelExtrema.clear();
}

template< typename TInputImage, typename TLabelImage >
void MinMaxLabelImageFilterWithIndex< TInputImage, TLabelImage >::AfterThreadedGenerateData()
{
  m_GlobalMin = std::numeric_limits<PixelType>::max();
  m_GlobalMax = std::numeric_limits<PixelType>::lowest();
  m_GlobalMinIndex.Fill(0);
  m_GlobalMaxIndex.Fill(0);

  // The first thread that saw a label seeds its merged extrema, so ties resolve
  // to the lowest thread, i.e. the first pixel in scan order.
  bool first = true;

  for (const auto& threadExtrema : m_ThreadExtrema)
  {
    for (const auto& [label, extrema] : threadExtrema)
    {
      auto [mergedIt, inserted] = m_LabelExtrema.try_emplace(label, extrema);

      if (!inserted)
      {
        LabelExtrema& merged = mergedIt->second;

        if (extrema.m_Min < merged.m_Min)
        {
          merged.m_Min = extrema.m_Min;
          merged.m_MinIndex = extrema.m_MinIndex;
        }
        if (extrema.m_Max > merged.m_Max)
        {
          merged.m_Max = extrema.m_Max;
          merged.m_MaxIndex = extrema.m_MaxIndex;
        }
      }

      if (first || extrema.m_Min < m_GlobalMin)
      {
        m_GlobalMin = extrema.m_Min;
        m_GlobalMinIndex = extrema.m_MinIndex;
      }
      if (first || extrema.m_Max > m_GlobalMax)
      {
        m_GlobalMax = extrema.m_Max;
        m_GlobalMaxIndex = extrema.m_MaxIndex;
      }

      first = false;
    }
  }
}
}
#endif
