#ifndef mitkThresholdSplit_cpp
#define mitkThresholdSplit_cpp

#include <mitkThresholdSplit.h>

template<class TColumnDecisionFunctor, class TFeatureCalculator, class TTag>
mitk::ThresholdSplit<TColumnDecisionFunctor, TFeatureCalculator, TTag>::ThresholdSplit() :
  m_CalculatingFeature(false),
  m_UseWeights(false),
  m_UseRandomSplit(false),
  m_Precision(0.0),
  m_MaximumTreeDepth(1000)
{
}

//template<class TColumnDecisionFunctor, class TFeatureCalculator, class TTag>
//mitk::ThresholdSplit<TColumnDecisionFunctor, TFeatureCalculator, TTag>::ThresholdSplit(const ThresholdSplit & /*other*/)/*:
//  m_CalculatingFeature(other.IsCalculatingFeature()),
//  m_UseWeights(other.IsUsingPointBasedWeights()),
//  m_UseRandomSplit(other.IsUsingRandomSplit()),
//  m_Precision(other.GetPrecision()),
//  m_MaximumTreeDepth(other.GetMaximumTreeDepth()),
//  m_FeatureCalculator(other.GetFeatureCalculator()),
//  m_Weights(other.GetWeights())*/
//{
//}

template<class TColumnDecisionFunctor, class TFeatureCalculator, class TTag>
void
mitk::ThresholdSplit<TColumnDecisionFunctor, TFeatureCalculator, TTag>::SetFeatureCalculator(TFeatureCalculator processor)
{
  m_FeatureCalculator = processor;
}

template<class TColumnDecisionFunctor, class TFeatureCalculator, class TTag>
TFeatureCalculator
mitk::ThresholdSplit<TColumnDecisionFunctor, TFeatureCalculator, TTag>::GetFeatureCalculator() const
{
  return m_FeatureCalculator;
}

template<class TColumnDecisionFunctor, class TFeatureCalculator, class TTag>
void
mitk::ThresholdSplit<TColumnDecisionFunctor, TFeatureCalculator, TTag>::SetCalculatingFeature(bool calculate)
{
  m_CalculatingFeature = calculate;
}

template<class TColumnDecisionFunctor, class TFeatureCalculator, class TTag>
bool
mitk::ThresholdSplit<TColumnDecisionFunctor, TFeatureCalculator, TTag>::IsCalculatingFeature() const
{
  return m_CalculatingFeature;
}

template<class TColumnDecisionFunctor, class TFeatureCalculator, class TTag>
void
mitk::ThresholdSplit<TColumnDecisionFunctor, TFeatureCalculator, TTag>::UsePointBasedWeights(bool weightsOn)
{
  m_UseWeights = weightsOn;
  bgfunc.UsePointWeights(weightsOn);
}

template<class TColumnDecisionFunctor, class TFeatureCalculator, class TTag>
bool
mitk::ThresholdSplit<TColumnDecisionFunctor, TFeatureCalculator, TTag>::IsUsingPointBasedWeights() const
{
  return m_UseWeights;
}

template<class TColumnDecisionFunctor, class TFeatureCalculator, class TTag>
void
mitk::ThresholdSplit<TColumnDecisionFunctor, TFeatureCalculator, TTag>::SetPrecision(double value)
{
  m_Precision = value;
}

template<class TColumnDecisionFunctor, class TFeatureCalculator, class TTag>
double
mitk::ThresholdSplit<TColumnDecisionFunctor, TFeatureCalculator, TTag>::GetPrecision() const
{
  return m_Precision;
}

template<class TColumnDecisionFunctor, class TFeatureCalculator, class TTag>
void
mitk::ThresholdSplit<TColumnDecisionFunctor, TFeatureCalculator, TTag>::SetMaximumTreeDepth(int value)
{
  m_MaximumTreeDepth = value;
}

template<class TColumnDecisionFunctor, class TFeatureCalculator, class TTag>
int
mitk::ThresholdSplit<TColumnDecisionFunctor, TFeatureCalculator, TTag>::GetMaximumTreeDepth() const
{
  return m_MaximumTreeDepth;
}

template<class TColumnDecisionFunctor, class TFeatureCalculator, class TTag>
void
mitk::ThresholdSplit<TColumnDecisionFunctor, TFeatureCalculator, TTag>::SetWeights(vigra::MultiArrayView<2, double> weights)
{
  m_Weights = weights;
  bgfunc.UsePointWeights(m_UseWeights);
  bgfunc.SetPointWeights(weights);
}

template<class TColumnDecisionFunctor, class TFeatureCalculator, class TTag>
vigra::MultiArrayView<2, double>
mitk::ThresholdSplit<TColumnDecisionFunctor, TFeatureCalculator, TTag>::GetWeights() const
{
  return m_Weights;
}

template<class TColumnDecisionFunctor, class TFeatureCalculator, class TTag>
double
mitk::ThresholdSplit<TColumnDecisionFunctor, TFeatureCalculator, TTag>::minGini() const
{
  return min_gini_[bestSplitIndex];
}

template<class TColumnDecisionFunctor, class TFeatureCalculator, class TTag>
int
mitk::ThresholdSplit<TColumnDecisionFunctor, TFeatureCalculator, TTag>::bestSplitColumn() const
{
  return splitColumns[bestSplitIndex];
}

template<class TColumnDecisionFunctor, class TFeatureCalculator, class TTag>
double
mitk::ThresholdSplit<TColumnDecisionFunctor, TFeatureCalculator, TTag>::bestSplitThreshold() const
{
  return min_thresholds_[bestSplitIndex];
}

template<class TColumnDecisionFunctor, class TFeatureCalculator, class TTag>
template<class T>
void
mitk::ThresholdSplit<TColumnDecisionFunctor, TFeatureCalculator, TTag>::set_external_parameters(vigra::ProblemSpec<T> const & in)
{
  SB::set_external_parameters(in);
  bgfunc.set_external_parameters( SB::ext_param_);
  int featureCount_ = SB::ext_param_.column_count_;
  splitColumns.resize(featureCount_);
  for(int k=0; k<featureCount_; ++k)
    splitColumns[k] = k;
  min_gini_.resize(featureCount_);
  min_indices_.resize(featureCount_);
  min_thresholds_.resize(featureCount_);
}

template<class TColumnDecisionFunctor, class TFeatureCalculator, class TTag>
template<class T, class C, class T2, class C2, class Region, class Random>
int
mitk::ThresholdSplit<TColumnDecisionFunctor, TFeatureCalculator, TTag>::findBestSplit(vigra::MultiArrayView<2, T, C> features,
                                                                                      vigra::MultiArrayView<2, T2, C2> labels,
                                                                                      Region & region,
                                                                                      vigra::ArrayVector<Region>& childRegions,
                                                                                      Random & randint)
{
  typedef typename Region::IndexIterator IndexIteratorType;

  if (m_CalculatingFeature)
  {
    // Do some very fance stuff here!!

    // This is not so simple as it might look! We need to
    // remember which feature has been used to be able to
    // use it for testing again!!
    // There, no Splitting class is used!!
  }

  bgfunc.UsePointWeights(m_UseWeights);
  bgfunc.UseRandomSplit(m_UseRandomSplit);

  vigra::detail::Correction<TTag>::exec(region, labels);
  // Create initial class count.
  for(std::size_t i = 0; i < region.classCounts_.size(); ++i)
  {
    region.classCounts_[i] = 0;
  }
  double regionSum = 0;
  for (typename Region::IndexIterator iter = region.begin(); iter != region.end(); ++iter)
  {
    double probability = 1.0;
    if (m_UseWeights)
    {
      probability = m_Weights(*iter, 0);
    }
    region.classCounts_[labels(*iter,0)] += probability;
    regionSum += probability;
  }
  region.classCountsIsValid = true;
  vigra::ArrayVector<double> vec;

  // Is pure region?
  region_gini_ = bgfunc.LossOfRegion(labels,
                                     region.begin(),
                                     region.end(),
                                     region.classCounts());
  if (region_gini_ <= m_Precision * regionSum) // Necessary to fix wrong calculation of Gini-Index
  {
    return this->makeTerminalNode(features, labels, region, randint);
  }

  // Randomize the order of columns
  for (int i = 0; i < SB::ext_param_.actual_mtry_; ++i)
  {
    std::swap(splitColumns[i],
              splitColumns[i+ randint(features.shape(1) - i)]);
  }

  // find the split with the best evaluation value
  bestSplitIndex = 0;
  double currentMiniGini = region_gini_;
  int numberOfTrials = features.shape(1);
  for (int k = 0; k < numberOfTrials; ++k)
  {
    bgfunc(columnVector(features, splitColumns[k]),
           labels,
           region.begin(), region.end(),
           region.classCounts());
    min_gini_[k] = bgfunc.GetMinimumLoss();
    min_indices_[k] = bgfunc.GetMinimumIndex();
    min_thresholds_[k] = bgfunc.GetMinimumThreshold();

    // removed classifier test section, because not necessary
    if (bgfunc.GetMinimumLoss() < currentMiniGini)
    {
      currentMiniGini = bgfunc.GetMinimumLoss();
      childRegions[0].classCounts() = bgfunc.GetBestCurrentCounts()[0];
      childRegions[1].classCounts() = bgfunc.GetBestCurrentCounts()[1];
      childRegions[0].classCountsIsValid = true;
      childRegions[1].classCountsIsValid = true;

      bestSplitIndex = k;
      numberOfTrials = SB::ext_param_.actual_mtry_;
    }
  }

  //If only a small improvement, make terminal node...
  if(vigra::closeAtTolerance(currentMiniGini, region_gini_))
  {
    return  this->makeTerminalNode(features, labels, region, randint);
  }

  vigra::Node<vigra::i_ThresholdNode> node(SB::t_data, SB::p_data);
  SB::node_ = node;
  node.threshold() = min_thresholds_[bestSplitIndex];
  node.column() = splitColumns[bestSplitIndex];

  // partition the range according to the best dimension
  vigra::SortSamplesByDimensions<vigra::MultiArrayView<2, T, C> >
      sorter(features, node.column(), node.threshold());
  IndexIteratorType bestSplit =
      std::partition(region.begin(), region.end(), sorter);
  // Save the ranges of the child stack entries.
  childRegions[0].setRange(   region.begin()  , bestSplit       );
  childRegions[0].rule = region.rule;
  childRegions[0].rule.push_back(std::make_pair(1, 1.0));
  childRegions[1].setRange(   bestSplit       , region.end()    );
  childRegions[1].rule = region.rule;
  childRegions[1].rule.push_back(std::make_pair(1, 1.0));

  return vigra::i_ThresholdNode;

  return 0;
}

//template<class TRegion, class TRegionIterator, class TLabelHolder, class TWeightsHolder>
//static void UpdateRegionCounts(TRegion & region, TRegionIterator begin, TRegionIterator end, TLabelHolder labels, TWeightsHolder weights)
//{
//    if(std::accumulate(region.classCounts().begin(),
//                        region.classCounts().end(), 0.0) != region.size())
//    {
//        RandomForestClassCounter<   LabelT,
//                                    ArrayVector<double> >
//            counter(labels, region.classCounts());
//        std::for_each(  region.begin(), region.end(), counter);
//        region.classCountsIsValid = true;
//    }
//}
//
//template<class TRegion, class TLabel, class TWeights>
//static void exec(Region & region, LabelT & labels)
//{
//    if(std::accumulate(region.classCounts().begin(),
//                        region.classCounts().end(), 0.0) != region.size())
//    {
//        RandomForestClassCounter<   LabelT,
//                                    ArrayVector<double> >
//            counter(labels, region.classCounts());
//        std::for_each(  region.begin(), region.end(), counter);
//        region.classCountsIsValid = true;
//    }
//}

#endif //mitkThresholdSplit_cpp
