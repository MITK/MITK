#ifndef mitkGIFCooccurenceMatrix2_h
#define mitkGIFCooccurenceMatrix2_h

#include <mitkAbstractGlobalImageFeature.h>
#include <mitkBaseData.h>
#include <MitkCLUtilitiesExports.h>

#include <eigen3/Eigen/src/Core/Array.h>

namespace mitk
{
  struct CoocurenceMatrixHolder
  {
  public:
    CoocurenceMatrixHolder(double min, double max, int number);

    int IntensityToIndex(double intensity);
    double IndexToMinIntensity(int index);
    double IndexToMeanIntensity(int index);
    double IndexToMaxIntensity(int index);

    double m_MinimumRange;
    double m_MaximumRange;
    double m_Stepsize;
    int m_NumberOfBins;
    Eigen::MatrixXd m_Matrix;

  };

  struct CoocurenceMatrixFeatures
  {
    CoocurenceMatrixFeatures():
      JointMaximum(0),
      JointAverage(0),
      JointVariance(0),
      JointEntropy(0),
      RowMaximum(0),
      RowAverage(0),
      RowVariance(0),
      RowEntropy(0),
      FirstRowColumnEntropy(0),
      SecondRowColumnEntropy(0),
      DifferenceAverage(0),
      DifferenceVariance(0),
      DifferenceEntropy(0),
      SumAverage(0),
      SumVariance(0),
      SumEntropy(0),
      AngularSecondMoment(0),
      Contrast(0),
      Dissimilarity(0),
      InverseDifference(0),
      InverseDifferenceNormalised(0),
      InverseDifferenceMoment(0),
      InverseDifferenceMomentNormalised(0),
      InverseVariance(0),
      Correlation(0),
      Autocorrelation(0),
      ClusterTendency(0),
      ClusterShade(0),
      ClusterProminence(0),
      FirstMeasureOfInformationCorrelation(0),
      SecondMeasureOfInformationCorrelation(0)
    {
    }

  public:
    double JointMaximum;
    double JointAverage;
    double JointVariance;
    double JointEntropy;
    double RowMaximum;
    double RowAverage;
    double RowVariance;
    double RowEntropy;
    double FirstRowColumnEntropy;
    double SecondRowColumnEntropy;
    double DifferenceAverage;
    double DifferenceVariance;
    double DifferenceEntropy;
    double SumAverage;
    double SumVariance;
    double SumEntropy;
    double AngularSecondMoment;
    double Contrast;
    double Dissimilarity;
    double InverseDifference;
    double InverseDifferenceNormalised;
    double InverseDifferenceMoment;
    double InverseDifferenceMomentNormalised;
    double InverseVariance;
    double Correlation;
    double Autocorrelation;
    double ClusterTendency;
    double ClusterShade;
    double ClusterProminence;
    double FirstMeasureOfInformationCorrelation;
    double SecondMeasureOfInformationCorrelation;
  };


  class MITKCLUTILITIES_EXPORT GIFCooccurenceMatrix2 : public AbstractGlobalImageFeature
  {
    public:
      mitkClassMacro(GIFCooccurenceMatrix2, AbstractGlobalImageFeature);
      itkFactorylessNewMacro(Self)
      itkCloneMacro(Self)

      GIFCooccurenceMatrix2();

      /**
      * \brief Calculates the Cooccurence-Matrix based features for this class.
      */
      virtual FeatureListType CalculateFeatures(const Image::Pointer & image, const Image::Pointer &feature) override;

      /**
      * \brief Returns a list of the names of all features that are calculated from this class
      */
      virtual FeatureNameListType GetFeatureNames() override;

      virtual void CalculateFeaturesUsingParameters(const Image::Pointer & feature, const Image::Pointer &mask, const Image::Pointer &maskNoNAN, FeatureListType &featureList);
      virtual void AddArguments(mitkCommandLineParser &parser);

      itkGetConstMacro(Range,double);
      itkSetMacro(Range, double);
      itkGetConstMacro(Bins, int);
      itkSetMacro(Bins, int);

    struct GIFCooccurenceMatrix2Configuration
    {
      double range;
      unsigned int direction;

      double MinimumIntensity;
      bool UseMinimumIntensity;
      double MaximumIntensity;
      bool UseMaximumIntensity;
      int Bins;
    };

    private:
      double m_Range;
      int m_Bins;
  };

}
#endif //mitkGIFCooccurenceMatrix2_h
