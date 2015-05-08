#ifndef mitkImpurityLoss_cpp
#define mitkImpurityLoss_cpp

#include <mitkImpurityLoss.h>

template <class TLossFunction, class TLabelContainer, class TWeightContainer>
template <class T>
mitk::ImpurityLoss<TLossFunction, TLabelContainer, TWeightContainer>::ImpurityLoss(TLabelContainer const &labels,
             vigra::ProblemSpec<T> const &ext) :
    m_UsePointWeights(false),
    m_Labels(labels),
    m_Counts(ext.class_count_, 0.0),
    m_ClassWeights(ext.class_weights_),
    m_TotalCount(0.0)
{
}

template <class TLossFunction, class TLabelContainer, class TWeightContainer>
void
mitk::ImpurityLoss<TLossFunction, TLabelContainer, TWeightContainer>::Reset()
{
    m_Counts.init(0);
    m_TotalCount = 0.0;
}

template <class TLossFunction, class TLabelContainer, class TWeightContainer>
template <class TDataIterator>
double
mitk::ImpurityLoss<TLossFunction, TLabelContainer, TWeightContainer>::Increment(TDataIterator begin, TDataIterator end)
{
    for (TDataIterator iter = begin; iter != end; ++iter)
    {
        double pointProbability = 1.0;
        if (m_UsePointWeights)
        {
            pointProbability = m_PointWeights(*iter,0);
        }
        m_Counts[m_Labels(*iter,0)] += pointProbability;
        m_TotalCount += pointProbability;
    }
    return m_LossFunction(m_Counts, m_ClassWeights, m_TotalCount);
}

template <class TLossFunction, class TLabelContainer, class TWeightContainer>
template <class TDataIterator>
double
mitk::ImpurityLoss<TLossFunction, TLabelContainer, TWeightContainer>::Decrement(TDataIterator begin, TDataIterator end)
{
    for (TDataIterator iter = begin; iter != end; ++iter)
    {
        double pointProbability = 1.0;
        if (m_UsePointWeights)
        {
            pointProbability = m_PointWeights(*iter,0);
        }
        m_Counts[m_Labels(*iter,0)] -= pointProbability;
        m_TotalCount -= pointProbability;
    }
    return m_LossFunction(m_Counts, m_ClassWeights, m_TotalCount);
}

template <class TLossFunction, class TLabelContainer, class TWeightContainer>
template <class TArray>
double
mitk::ImpurityLoss<TLossFunction, TLabelContainer, TWeightContainer>::Init(TArray initCounts)
{
    Reset();
    std::copy(initCounts.begin(), initCounts.end(), m_Counts.begin());
    m_TotalCount = std::accumulate(m_Counts.begin(), m_Counts.end(), 0.0);
    return m_LossFunction(m_Counts, m_ClassWeights, m_TotalCount);
}

template <class TLossFunction, class TLabelContainer, class TWeightContainer>
vigra::ArrayVector<double> const&
mitk::ImpurityLoss<TLossFunction, TLabelContainer, TWeightContainer>::Response()
{
    return m_Counts;
}

template <class TLossFunction, class TLabelContainer, class TWeightContainer>
void
mitk::ImpurityLoss<TLossFunction, TLabelContainer, TWeightContainer>::UsePointWeights(bool useWeights)
{
    m_UsePointWeights = useWeights;
}

template <class TLossFunction, class TLabelContainer, class TWeightContainer>
bool
mitk::ImpurityLoss<TLossFunction, TLabelContainer, TWeightContainer>::IsUsingPointWeights()
{
    return m_UsePointWeights;
}

template <class TLossFunction, class TLabelContainer, class TWeightContainer>
void
mitk::ImpurityLoss<TLossFunction, TLabelContainer, TWeightContainer>::SetPointWeights(TWeightContainer weight)
{
    m_PointWeights = weight;
}

template <class TLossFunction, class TLabelContainer, class TWeightContainer>
typename mitk::ImpurityLoss<TLossFunction, TLabelContainer, TWeightContainer>::WeightContainerType
mitk::ImpurityLoss<TLossFunction, TLabelContainer, TWeightContainer>::GetPointWeights()
{
    return m_PointWeights;
}


#endif // mitkImpurityLoss_cpp


