#ifndef mitkPUImpurityLoss_cpp
#define mitkPUImpurityLoss_cpp

#include <mitkPUImpurityLoss.h>
#include <mitkAdditionalRFData.h>

template <class TLossFunction, class TLabelContainer, class TWeightContainer>
template <class T>
mitk::PUImpurityLoss<TLossFunction, TLabelContainer, TWeightContainer>::PUImpurityLoss(TLabelContainer const &labels,
  vigra::ProblemSpec<T> const &ext,
  AdditionalRFDataAbstract *data) :
    m_UsePointWeights(false),
    m_Labels(labels),
    //m_Kappa(ext.kappa_), // Not possible due to data type
    m_Counts(ext.class_count_, 0.0),
    m_PUCounts(ext.class_count_, 0.0),
    m_ClassWeights(ext.class_weights_),
    m_TotalCount(0.0),
    m_PUTotalCount(0.0),
    m_ClassCount(ext.class_count_)
{
  mitk::PURFData * purfdata = dynamic_cast<PURFData *> (data);
  //const PURFProblemSpec<T> *problem = static_cast<const PURFProblemSpec<T> * > (&ext);
  m_Kappa = vigra::ArrayVector<double>(purfdata->m_Kappa);
}

template <class TLossFunction, class TLabelContainer, class TWeightContainer>
void
mitk::PUImpurityLoss<TLossFunction, TLabelContainer, TWeightContainer>::Reset()
{
    m_Counts.init(0);
    m_TotalCount = 0.0;
}

template <class TLossFunction, class TLabelContainer, class TWeightContainer>
void
mitk::PUImpurityLoss<TLossFunction, TLabelContainer, TWeightContainer>::UpdatePUCounts()
{
  m_PUTotalCount = 0;
  for (int i = 1; i < m_ClassCount; ++i)
  {
    m_PUCounts[i] = m_Kappa[i] * m_Counts[i];
    m_PUTotalCount += m_PUCounts[i];
  }
  m_PUCounts[0] = std::max(0.0, m_TotalCount - m_PUTotalCount);
  m_PUTotalCount += m_PUCounts[0];
}

template <class TLossFunction, class TLabelContainer, class TWeightContainer>
template <class TDataIterator>
double
mitk::PUImpurityLoss<TLossFunction, TLabelContainer, TWeightContainer>::Increment(TDataIterator begin, TDataIterator end)
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
    UpdatePUCounts();
    return m_LossFunction(m_PUCounts, m_ClassWeights, m_PUTotalCount);
}

template <class TLossFunction, class TLabelContainer, class TWeightContainer>
template <class TDataIterator>
double
mitk::PUImpurityLoss<TLossFunction, TLabelContainer, TWeightContainer>::Decrement(TDataIterator begin, TDataIterator end)
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
    UpdatePUCounts();
    return m_LossFunction(m_PUCounts, m_ClassWeights, m_PUTotalCount);
}

template <class TLossFunction, class TLabelContainer, class TWeightContainer>
template <class TArray>
double
mitk::PUImpurityLoss<TLossFunction, TLabelContainer, TWeightContainer>::Init(TArray initCounts)
{
    Reset();
    std::copy(initCounts.begin(), initCounts.end(), m_Counts.begin());
    m_TotalCount = std::accumulate(m_Counts.begin(), m_Counts.end(), 0.0);
    return m_LossFunction(m_Counts, m_ClassWeights, m_TotalCount);
}

template <class TLossFunction, class TLabelContainer, class TWeightContainer>
vigra::ArrayVector<double> const&
mitk::PUImpurityLoss<TLossFunction, TLabelContainer, TWeightContainer>::Response()
{
    return m_Counts;
}

template <class TLossFunction, class TLabelContainer, class TWeightContainer>
void
mitk::PUImpurityLoss<TLossFunction, TLabelContainer, TWeightContainer>::UsePointWeights(bool useWeights)
{
    m_UsePointWeights = useWeights;
}

template <class TLossFunction, class TLabelContainer, class TWeightContainer>
bool
mitk::PUImpurityLoss<TLossFunction, TLabelContainer, TWeightContainer>::IsUsingPointWeights()
{
    return m_UsePointWeights;
}

template <class TLossFunction, class TLabelContainer, class TWeightContainer>
void
mitk::PUImpurityLoss<TLossFunction, TLabelContainer, TWeightContainer>::SetPointWeights(TWeightContainer weight)
{
    m_PointWeights = weight;
}

template <class TLossFunction, class TLabelContainer, class TWeightContainer>
typename mitk::PUImpurityLoss<TLossFunction, TLabelContainer, TWeightContainer>::WeightContainerType
mitk::PUImpurityLoss<TLossFunction, TLabelContainer, TWeightContainer>::GetPointWeights()
{
    return m_PointWeights;
}


#endif // mitkImpurityLoss_cpp


