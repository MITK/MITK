#ifndef mitkGIFGreyLevelSizeZone_h
#define mitkGIFGreyLevelSizeZone_h

#include <mitkAbstractGlobalImageFeature.h>
#include <mitkBaseData.h>
#include <MitkCLUtilitiesExports.h>

#include <Eigen/src/Core/Array.h>

namespace mitk
{
  struct GreyLevelSizeZoneMatrixHolder
  {
  public:
    GreyLevelSizeZoneMatrixHolder(double min, double max, int number, int maxSize);

    int IntensityToIndex(double intensity);
    double IndexToMinIntensity(int index);
    double IndexToMeanIntensity(int index);
    double IndexToMaxIntensity(int index);

    double m_MinimumRange;
    double m_MaximumRange;
    double m_Stepsize;
    int m_NumberOfBins;
    int m_MaximumSize;
    Eigen::MatrixXd m_Matrix;

  };

  struct GreyLevelSizeZoneFeatures
  {
    GreyLevelSizeZoneFeatures() :
      SmallZoneEmphasis(0),
      LargeZoneEmphasis(0),
      LowGreyLevelEmphasis(0),
      HighGreyLevelEmphasis(0),
      SmallZoneLowGreyLevelEmphasis(0),
      SmallZoneHighGreyLevelEmphasis(0),
      LargeZoneLowGreyLevelEmphasis(0),
      LargeZoneHighGreyLevelEmphasis(0),
      GreyLevelNonUniformity(0),
      GreyLevelNonUniformityNormalized(0),
      ZoneSizeNonUniformity(0),
      ZoneSizeNoneUniformityNormalized(0),
      ZonePercentage(0),
      GreyLevelMean(0),
      GreyLevelVariance(0),
      ZoneSizeMean(0),
      ZoneSizeVariance(0),
      ZoneSizeEntropy(0)
    {
    }

  public:
    double SmallZoneEmphasis;
    double LargeZoneEmphasis;
    double LowGreyLevelEmphasis;
    double HighGreyLevelEmphasis;
    double SmallZoneLowGreyLevelEmphasis;
    double SmallZoneHighGreyLevelEmphasis;
    double LargeZoneLowGreyLevelEmphasis;
    double LargeZoneHighGreyLevelEmphasis;
    double GreyLevelNonUniformity;
    double GreyLevelNonUniformityNormalized;
    double ZoneSizeNonUniformity;
    double ZoneSizeNoneUniformityNormalized;
    double ZonePercentage;
    double GreyLevelMean;
    double GreyLevelVariance;
    double ZoneSizeMean;
    double ZoneSizeVariance;
    double ZoneSizeEntropy;
  };


  class MITKCLUTILITIES_EXPORT GIFGreyLevelSizeZone : public AbstractGlobalImageFeature
  {
    public:
      mitkClassMacro(GIFGreyLevelSizeZone, AbstractGlobalImageFeature);
      itkFactorylessNewMacro(Self)
      itkCloneMacro(Self)

      GIFGreyLevelSizeZone();

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

      struct GIFGreyLevelSizeZoneConfiguration
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
#endif //mitkGIFGreyLevelSizeZone_h
