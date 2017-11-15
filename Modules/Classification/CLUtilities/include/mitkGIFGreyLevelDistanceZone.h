#ifndef mitkGIFGreyLevelDistanceZone_h
#define mitkGIFGreyLevelDistanceZone_h

#include <mitkAbstractGlobalImageFeature.h>
#include <mitkBaseData.h>
#include <MitkCLUtilitiesExports.h>

#include <Eigen/src/Core/Array.h>

namespace mitk
{
  struct GreyLevelDistanceZoneMatrixHolder
  {
  public:
    GreyLevelDistanceZoneMatrixHolder(double min, double max, int number, int maxSize);

    int IntensityToIndex(double intensity);
    double IndexToMinIntensity(int index);
    double IndexToMeanIntensity(int index);
    double IndexToMaxIntensity(int index);

    double m_MinimumRange;
    double m_MaximumRange;
    double m_Stepsize;
    int m_NumberOfBins;
    int m_MaximumSize;
    int m_NumerOfVoxels;
    Eigen::MatrixXd m_Matrix;

  };

  struct GreyLevelDistanceZoneFeatures
  {
    GreyLevelDistanceZoneFeatures() :
      SmallDistanceEmphasis(0),
      LargeDistanceEmphasis(0),
      LowGreyLevelEmphasis(0),
      HighGreyLevelEmphasis(0),
      SmallDistanceLowGreyLevelEmphasis(0),
      SmallDistanceHighGreyLevelEmphasis(0),
      LargeDistanceLowGreyLevelEmphasis(0),
      LargeDistanceHighGreyLevelEmphasis(0),
      GreyLevelNonUniformity(0),
      GreyLevelNonUniformityNormalized(0),
      ZoneDistanceNonUniformity(0),
      ZoneDistanceNoneUniformityNormalized(0),
      ZonePercentage(0),
      GreyLevelMean(0),
      GreyLevelVariance(0),
      ZoneDistanceMean(0),
      ZoneDistanceVariance(0),
      ZoneDistanceEntropy(0)
    {
    }

  public:
    double SmallDistanceEmphasis;
    double LargeDistanceEmphasis;
    double LowGreyLevelEmphasis;
    double HighGreyLevelEmphasis;
    double SmallDistanceLowGreyLevelEmphasis;
    double SmallDistanceHighGreyLevelEmphasis;
    double LargeDistanceLowGreyLevelEmphasis;
    double LargeDistanceHighGreyLevelEmphasis;
    double GreyLevelNonUniformity;
    double GreyLevelNonUniformityNormalized;
    double ZoneDistanceNonUniformity;
    double ZoneDistanceNoneUniformityNormalized;
    double ZonePercentage;
    double GreyLevelMean;
    double GreyLevelVariance;
    double ZoneDistanceMean;
    double ZoneDistanceVariance;
    double ZoneDistanceEntropy;
  };


  class MITKCLUTILITIES_EXPORT GIFGreyLevelDistanceZone : public AbstractGlobalImageFeature
  {
    public:
      mitkClassMacro(GIFGreyLevelDistanceZone, AbstractGlobalImageFeature);
      itkFactorylessNewMacro(Self)
      itkCloneMacro(Self)

      GIFGreyLevelDistanceZone();

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

      struct GIFGreyLevelDistanceZoneConfiguration
    {
      mitk::Image::Pointer distanceMask;

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
#endif //mitkGIFGreyLevelDistanceZone_h
