#ifndef mitkLinearSplitting_h
#define mitkLinearSplitting_h

#include <vigra/multi_array.hxx>
#include <vigra/random_forest.hxx>
#include <mitkAdditionalRFData.h>

namespace mitk
{
  template<class TLossAccumulator>
  class LinearSplitting
  {
  public:
      typedef typename TLossAccumulator::WeightContainerType TWeightContainer;
      typedef TWeightContainer WeightContainerType;

      LinearSplitting();

      template <class T>
      LinearSplitting(vigra::ProblemSpec<T> const &ext);

      void UsePointWeights(bool pointWeight);
      bool IsUsingPointWeights();

      void UseRandomSplit(bool randomSplit);
      bool IsUsingRandomSplit();

      void SetPointWeights(WeightContainerType weight);
      WeightContainerType GetPointWeights();

      void SetAdditionalData(AdditionalRFDataAbstract* data);
      AdditionalRFDataAbstract* GetAdditionalData() const;

      template <class T>
      void set_external_parameters(vigra::ProblemSpec<T> const &ext);

      template <class TDataSourceFeature,
                class TDataSourceLabel,
                class TDataIterator,
                class TArray>
      void operator()(TDataSourceFeature const &column,
                      TDataSourceLabel const &labels,
                      TDataIterator &begin,
                      TDataIterator &end,
                      TArray const &regionResponse);

      template <class TDataSourceLabel,
                class TDataIterator,
                class TArray>
      double LossOfRegion(TDataSourceLabel const & labels,
                          TDataIterator &begin,
                          TDataIterator &end,
                          TArray const & regionResponse);

      double GetMinimumLoss()
      {
          return m_MinimumLoss;
      }

      double GetMinimumThreshold()
      {
          return m_MinimumThreshold;
      }

      std::ptrdiff_t GetMinimumIndex()
      {
          return m_MinimumIndex;
      }

      vigra::ArrayVector<double>* GetBestCurrentCounts()
      {
          return m_BestCurrentCounts;
      }

  private:
      bool m_UsePointWeights;
      bool m_UseRandomSplit;
      WeightContainerType m_PointWeights;
      // From original code
      vigra::ArrayVector<double> m_ClassWeights;
      vigra::ArrayVector<double> m_BestCurrentCounts[2];
      double m_MinimumLoss;
      double m_MinimumThreshold;
      std::ptrdiff_t m_MinimumIndex;
      vigra::ProblemSpec<> m_ExtParameter;
      AdditionalRFDataAbstract* m_AdditionalData;
  };
}

#include <../src/Splitter/mitkLinearSplitting.cpp>
#endif //mitkLinearSplitting_h
