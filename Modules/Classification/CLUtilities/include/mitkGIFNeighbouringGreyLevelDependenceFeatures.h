#ifndef mitkGIFNeighbouringGreyLevelDependenceFeatures_h
#define mitkGIFNeighbouringGreyLevelDependenceFeatures_h

#include <mitkAbstractGlobalImageFeature.h>
#include <mitkBaseData.h>
#include <MitkCLUtilitiesExports.h>

#include <eigen3\Eigen\src\Core\Array.h>

namespace mitk
{
  struct NGLDMMatrixHolder
  {
  public:
    NGLDMMatrixHolder(double min, double max, int number, int depenence);

    int IntensityToIndex(double intensity);
    double IndexToMinIntensity(int index);
    double IndexToMeanIntensity(int index);
    double IndexToMaxIntensity(int index);

    double m_MinimumRange;
    double m_MaximumRange;
    double m_Stepsize;
    int m_NumberOfDependences;
    int m_NumberOfBins;
    Eigen::MatrixXd m_Matrix;

    int m_NeighbourhoodSize;
    unsigned long m_NumberOfNeighbourVoxels;
    unsigned long m_NumberOfDependenceNeighbourVoxels;
    unsigned long m_NumberOfNeighbourhoods;
    unsigned long m_NumberOfCompleteNeighbourhoods;
  };

  struct NGLDMMatrixFeatures
  {
    NGLDMMatrixFeatures() :
      LowDependenceEmphasis(0),
      HighDependenceEmphasis(0),
      LowGreyLevelCountEmphasis(0),
      HighGreyLevelCountEmphasis(0),
      LowDependenceLowGreyLevelEmphasis(0),
      LowDependenceHighGreyLevelEmphasis(0),
      HighDependenceLowGreyLevelEmphasis(0),
      HighDependenceHighGreyLevelEmphasis(0),
      GreyLevelNonUniformity(0),
      GreyLevelNonUniformityNormalised(0),
      DependenceCountNonUniformity(0),
      DependenceCountNonUniformityNormalised(0),
      DependenceCountPercentage(0),
      GreyLevelVariance(0),
      DependenceCountVariance(0),
      DependenceCountEntropy(0),
      MeanGreyLevelCount(0),
      MeanDependenceCount(0),
      ExpectedNeighbourhoodSize(0),
      AverageNeighbourhoodSize(0),
      AverageIncompleteNeighbourhoodSize(0),
      PercentageOfCompleteNeighbourhoods(0),
      PercentageOfDependenceNeighbours(0)
    {
    }

  public:
    double LowDependenceEmphasis;
    double HighDependenceEmphasis;
    double LowGreyLevelCountEmphasis;
    double HighGreyLevelCountEmphasis;
    double LowDependenceLowGreyLevelEmphasis;
    double LowDependenceHighGreyLevelEmphasis;
    double HighDependenceLowGreyLevelEmphasis;
    double HighDependenceHighGreyLevelEmphasis;

    double GreyLevelNonUniformity;
    double GreyLevelNonUniformityNormalised;
    double DependenceCountNonUniformity;
    double DependenceCountNonUniformityNormalised;

    double DependenceCountPercentage;
    double GreyLevelVariance;
    double DependenceCountVariance;
    double DependenceCountEntropy;
    double MeanGreyLevelCount;
    double MeanDependenceCount;

    double ExpectedNeighbourhoodSize;
    double AverageNeighbourhoodSize;
    double AverageIncompleteNeighbourhoodSize;
    double PercentageOfCompleteNeighbourhoods;
    double PercentageOfDependenceNeighbours;

  };


  class MITKCLUTILITIES_EXPORT GIFNeighbouringGreyLevelDependenceFeature : public AbstractGlobalImageFeature
  {
    public:
      mitkClassMacro(GIFNeighbouringGreyLevelDependenceFeature, AbstractGlobalImageFeature)
      itkFactorylessNewMacro(Self)
      itkCloneMacro(Self)

      GIFNeighbouringGreyLevelDependenceFeature();

      /**
      * \brief Calculates the Cooccurence-Matrix based features for this class.
      */
      virtual FeatureListType CalculateFeatures(const Image::Pointer & image, const Image::Pointer &feature) override;

      /**
      * \brief Returns a list of the names of all features that are calculated from this class
      */
      virtual FeatureNameListType GetFeatureNames() override;

      itkGetConstMacro(Range,double);
      itkSetMacro(Range, double);
    itkGetConstMacro(Alpha, int);
    itkSetMacro(Alpha, int);
    itkGetConstMacro(Bins, int);
    itkSetMacro(Bins, int);

    virtual void CalculateFeaturesUsingParameters(const Image::Pointer & feature, const Image::Pointer &mask, const Image::Pointer &maskNoNAN, FeatureListType &featureList);
    virtual void AddArguments(mitkCommandLineParser &parser);


    struct GIFNeighbouringGreyLevelDependenceFeatureConfiguration
    {
      double range;
      unsigned int direction;
      int alpha;

      double MinimumIntensity;
      bool UseMinimumIntensity;
      double MaximumIntensity;
      bool UseMaximumIntensity;
      int Bins;
    };

    private:
    double m_Range;
    int m_Alpha;
    int m_Bins;
  };

}
#endif //mitkGIFNeighbouringGreyLevelDependenceFeature_h
