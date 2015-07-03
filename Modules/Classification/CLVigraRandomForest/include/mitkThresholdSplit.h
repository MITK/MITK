#ifndef mitkThresholdSplit_h
#define mitkThresholdSplit_h

#include <vigra/multi_array.hxx>
#include <vigra/random_forest.hxx>

namespace mitk
{
    template<class TColumnDecisionFunctor, class TFeatureCalculator, class TTag = vigra::ClassificationTag>
    class ThresholdSplit: public vigra::SplitBase<TTag>
    {
    public:
        ThresholdSplit();
//        ThresholdSplit(const ThresholdSplit & other);

        void SetFeatureCalculator(TFeatureCalculator processor);
        TFeatureCalculator GetFeatureCalculator() const;

        void SetCalculatingFeature(bool calculate);
        bool IsCalculatingFeature() const;

        void UsePointBasedWeights(bool weightsOn);
        bool IsUsingPointBasedWeights() const;

        void UseRandomSplit(bool split) {m_UseRandomSplit = split;}
        bool IsUsingRandomSplit() const { return m_UseRandomSplit; }

        void SetPrecision(double value);
        double GetPrecision() const;

        void SetMaximumTreeDepth(int value);
        virtual int GetMaximumTreeDepth() const;

        void SetWeights(vigra::MultiArrayView<2, double> weights);
        vigra::MultiArrayView<2, double> GetWeights() const;

        // From vigra::ThresholdSplit
        double minGini() const;
        int bestSplitColumn() const;
        double bestSplitThreshold() const;

        template<class T>
        void set_external_parameters(vigra::ProblemSpec<T> const & in);

        template<class T, class C, class T2, class C2, class Region, class Random>
        int findBestSplit(vigra::MultiArrayView<2, T, C> features,
                          vigra::MultiArrayView<2, T2, C2> labels,
                          Region & region,
                          vigra::ArrayVector<Region>& childRegions,
                          Random & randint);

        double region_gini_;

    private:

        // From vigra::ThresholdSplit
        typedef vigra::SplitBase<TTag> SB;

       // splitter parameters (used by copy constructor)
        bool m_CalculatingFeature;
        bool m_UseWeights;
        bool m_UseRandomSplit;
        double m_Precision;
        int m_MaximumTreeDepth;
        TFeatureCalculator m_FeatureCalculator;
        vigra::MultiArrayView<2, double> m_Weights;

        // variabels to work with
        vigra::ArrayVector<vigra::Int32> splitColumns;
        TColumnDecisionFunctor bgfunc;
        vigra::ArrayVector<double> min_gini_;
        vigra::ArrayVector<std::ptrdiff_t> min_indices_;
        vigra::ArrayVector<double> min_thresholds_;
        int bestSplitIndex;

    };
}


#include <../src/Splitter/mitkThresholdSplit.cpp>
#endif //mitkThresholdSplit_h
