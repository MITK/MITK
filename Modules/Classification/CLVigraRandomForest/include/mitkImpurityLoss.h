#ifndef mitkImpurityLoss_h
#define mitkImpurityLoss_h

#include <vigra/multi_array.hxx>
#include <vigra/random_forest.hxx>
#include <mitkAdditionalRFData.h>

namespace mitk
{
    template <class TLossFunction = vigra::GiniCriterion, class TLabelContainer = vigra::MultiArrayView<2, int>, class TWeightContainer = vigra::MultiArrayView<2, double> >
    class ImpurityLoss
    {
    public:
        typedef TLabelContainer LabelContainerType;
        typedef TWeightContainer WeightContainerType;

        template <class T>
        ImpurityLoss(TLabelContainer const &labels,
          vigra::ProblemSpec<T> const &ext,
          AdditionalRFDataAbstract *data);

        void Reset();

        template <class TDataIterator>
        double Increment(TDataIterator begin, TDataIterator end);

        template <class TDataIterator>
        double Decrement(TDataIterator begin, TDataIterator end);

        template <class TArray>
        double Init(TArray initCounts);

        vigra::ArrayVector<double> const& Response();

        void UsePointWeights(bool useWeights);
        bool IsUsingPointWeights();

        void SetPointWeights(TWeightContainer weight);
        WeightContainerType GetPointWeights();

    private:
        bool m_UsePointWeights;
        TWeightContainer m_PointWeights;

        //Variable of origin
        TLabelContainer const& m_Labels;
        vigra::ArrayVector<double> m_Counts;
        vigra::ArrayVector<double> m_ClassWeights;
        double m_TotalCount;
        TLossFunction m_LossFunction;
    };

}

#include <../src/Splitter/mitkImpurityLoss.cpp>

#endif //mitkImpurityLoss_h
