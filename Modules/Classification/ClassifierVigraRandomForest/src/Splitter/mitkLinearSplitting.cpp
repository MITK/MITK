#ifndef mitkLinearSplitting_cpp
#define mitkLinearSplitting_cpp

#include <mitkLinearSplitting.h>

template<class TLossAccumulator>
mitk::LinearSplitting<TLossAccumulator>::LinearSplitting() :
    m_UsePointWeights(false),
    m_UseRandomSplit(false)
{
}

template<class TLossAccumulator>
template <class T>
mitk::LinearSplitting<TLossAccumulator>::LinearSplitting(vigra::ProblemSpec<T> const &ext) :
    m_UsePointWeights(false),
    m_UseRandomSplit(false)
{
    set_external_parameters(ext);
}

template<class TLossAccumulator>
void
mitk::LinearSplitting<TLossAccumulator>::UsePointWeights(bool pointWeight)
{
    m_UsePointWeights = pointWeight;
}

template<class TLossAccumulator>
bool
mitk::LinearSplitting<TLossAccumulator>::IsUsingPointWeights()
{
    return m_UsePointWeights;
}


template<class TLossAccumulator>
void
mitk::LinearSplitting<TLossAccumulator>::UseRandomSplit(bool randomSplit)
{
  m_UseRandomSplit = randomSplit;
}

template<class TLossAccumulator>
bool
mitk::LinearSplitting<TLossAccumulator>::IsUsingRandomSplit()
{
  return m_UseRandomSplit;
}

template<class TLossAccumulator>
void
mitk::LinearSplitting<TLossAccumulator>::SetPointWeights(WeightContainerType weight)
{
    m_PointWeights = weight;
}

template<class TLossAccumulator>
typename mitk::LinearSplitting<TLossAccumulator>::WeightContainerType
mitk::LinearSplitting<TLossAccumulator>::GetPointWeights()
{
    return m_PointWeights;
}

template<class TLossAccumulator>
template <class T>
void
mitk::LinearSplitting<TLossAccumulator>::set_external_parameters(vigra::ProblemSpec<T> const &ext)
{
    m_ExtParameter = ext;
}

template<class TLossAccumulator>
template <class TDataSourceFeature, class TDataSourceLabel, class TDataIterator, class TArray>
void
mitk::LinearSplitting<TLossAccumulator>::operator()(TDataSourceFeature const &column,
                TDataSourceLabel const &labels,
                TDataIterator &begin,
                TDataIterator &end,
                TArray const &regionResponse)
{
    typedef TLossAccumulator LineSearchLoss;
    std::sort(begin, end, vigra::SortSamplesByDimensions<TDataSourceFeature>(column, 0));

    LineSearchLoss left(labels, m_ExtParameter);
    LineSearchLoss right(labels, m_ExtParameter);

    if (m_UsePointWeights)
    {
        left.UsePointWeights(true);
        left.SetPointWeights(m_PointWeights);
        right.UsePointWeights(true);
        right.SetPointWeights(m_PointWeights);
    }

    m_MinimumLoss = right.Init(regionResponse);
    m_MinimumThreshold = *begin;
    m_MinimumIndex = 0;

    vigra::DimensionNotEqual<TDataSourceFeature> compareNotEqual(column, 0);

    if (!m_UseRandomSplit)
    {
      TDataIterator iter = begin;
      // Find the next element that are NOT equal with his neightbour!
      TDataIterator next = std::adjacent_find(iter, end, compareNotEqual);

      while(next != end)
      {
          // Remove  or add the current segment are from the LineSearch
          double rightLoss = right.Decrement(iter, next +1);
          double leftLoss = left.Increment(iter, next +1);
          double currentLoss = rightLoss + leftLoss;

          if (currentLoss < m_MinimumLoss)
          {
              m_BestCurrentCounts[0] = left.Response();
              m_BestCurrentCounts[1] = right.Response();
              m_MinimumLoss = currentLoss;
              m_MinimumIndex = next - begin + 1;
              m_MinimumThreshold = (double(column(*next,0)) + double(column(*(next +1), 0)))/2.0;
          }

          iter = next + 1;
          next = std::adjacent_find(iter, end, compareNotEqual);
      }
    }
    else // If Random split is selected, e.g. ExtraTree behaviour
    {
      int size = end - begin + 1;
      srand(time(NULL));
      int offset = rand() % size;
      TDataIterator iter = begin + offset;

      double rightLoss = right.Decrement(begin, iter+1);
      double leftLoss = left.Increment(begin, iter+1);
      double currentLoss = rightLoss + leftLoss;

      if (currentLoss < m_MinimumLoss)
      {
        m_BestCurrentCounts[0] = left.Response();
        m_BestCurrentCounts[1] = right.Response();
        m_MinimumLoss = currentLoss;
        m_MinimumIndex = offset + 1;
        m_MinimumThreshold = (double(column(*iter,0)) + double(column(*(iter+1), 0)))/2.0;
      }
    }
}

template<class TLossAccumulator>
template <class TDataSourceLabel, class TDataIterator, class TArray>
double
mitk::LinearSplitting<TLossAccumulator>::LossOfRegion(TDataSourceLabel const & labels,
                    TDataIterator &/*begin*/,
                    TDataIterator &/*end*/,
                    TArray const & regionResponse)
{
    typedef TLossAccumulator LineSearchLoss;
    LineSearchLoss regionLoss(labels, m_ExtParameter);
    if (m_UsePointWeights)
    {
        regionLoss.UsePointWeights(true);
        regionLoss.SetPointWeights(m_PointWeights);
    }
    return regionLoss.Init(regionResponse);
}

#endif //mitkLinearSplitting_cpp
