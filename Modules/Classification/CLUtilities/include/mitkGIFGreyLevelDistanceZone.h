#ifndef mitkGIFGreyLevelDistanceZone_h
#define mitkGIFGreyLevelDistanceZone_h

#include <mitkAbstractGlobalImageFeature.h>
#include <mitkBaseData.h>
#include <MitkCLUtilitiesExports.h>

#include <Eigen/src/Core/Array.h>

namespace mitk
{
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
    /**
     * \brief Calculates the Grey Level Distance Zone
     *
     * This class can be used to calculate Grey Level Distance Zone as presented in Thibault et al. 2014.
     *
     * The basic idea behind the Grey Level Distance Zone based features is to count the connected areas
     * with a given intensity value \f$x_i\f$ and a given distance to the border of each segmentation \f$d_i\f$.
     * Several features are then calculated based on a matrix, that gives the number of occurence for each
     * combination of \f$x_i\f$ and \f$ d_i \f$ as \f$m_{x,d}\f$.
     *
     * This feature calculator is activated by the option <b>-grey-level-distance-zone</b> or <b>-gldz</b>.
     *
     * The connected areas are based on the binned image, the binning parameters can be set via the default
     * parameters as described in AbstractGlobalImageFeature. It is also possible to determine the
     * dimensionality of the neighbourhood using direction-related commands as described in AbstractGlobalImageFeature.
     * No other options are possible beside these two options.
     *
     * The features are calculated based on a mask. It is assumed that the mask is
     * of the type of an unsigned short image. It is expected that the image contains only voxels with value 0 and 1,
     * of which all voxels with an value equal to one are treated as masked.
     *
     * The features depend on the distance to the border of the segmentation ROI. In some cases, the border
     * definition might be different from the definition of the masked area, for example, if small openings
     * in the mask should not influence the distance. Due to that, it is possible to submit a second mask,
     * named Morphological Mask to the features that is then used to calculate the distance of each voxel to
     * border of the segmented area. The morpological mask can be either set by the function call SetMorphMask()
     * or by the corresponding global option. (Not parsed by the filter itself, but by the command line tool).
     *
     * Beside the complete matrix, which is represented by its individual elements \f$m_{x,d}\f$, som eadditional
     * values are used for the definition. \f$N_g\f$ is the number of discrete grey levels, \f$ N_d\f$ the number
     * (or maximum value) of possible distances, and  \f$N_s\f$ the total number of zones.
     * \f$m_{x,d}\f$ gives the number of connected areas with the discrete
     * grey level x and distance to the boarder of d. Corresponding, \f$p_{x,d} = \frac{m_{x,d}}{N_s} gives the relativ
     * probability of this matrix cell. \f$ \f$N_v\f$ is the number of voxels. In addition, the marginal
     * sums \f$m_{x,\cdot} = m_x = \sum_d m_{x,d} \f$ , representing the sum of all zones with a given intensity, and
     * sums \f$m_{\cdot, d} = m_d = \sum_x m_{x,d} \f$ , representing the sum of all zones with a given distance, are used.
     * The distance are given as the number of voxels to the border and the voxel intensity is given as the
     * bin number of the binned image, starting with 1.
     *
     * The following features are then defined:
     * - <b>Grey Level Distance Zone::Small Distance Emphasis</b>:
     * \f[ \textup{Small Distance Emphasis}= \frac{1}{N_s} \sum_d \frac{m_d}{d^2} \f]
     * - <b>Grey Level Distance Zone::Large Distance Emphasis</b>:
     * \f[ \textup{Large Distance Emphasis}= \frac{1}{N_s} \sum_d d^2 m_d \f]
     * - <b>Grey Level Distance Zone::Low Grey Level Emphasis</b>:
     * \f[ \textup{Low Grey Level Emphasis}= \frac{1}{N_s} \sum_x  \frac{m_x}{x^2} \f]
     * - <b>Grey Level Distance Zone::High Grey Level Emphasis</b>:
     * \f[ \textup{High Grey Level Emphasis}= \frac{1}{N_s} \sum_x x^2 m_x \f]
     * - <b>Grey Level Distance Zone::Small Distance Low Grey Level Emphasis</b>:
     * \f[ \textup{Small Distance Low Grey Level Emphasis}= \frac{1}{N_s} \sum_x \sum_d \frac{ m_{x,d}}{x^2 d^2}\f]
     * - <b>Grey Level Distance Zone::Small Distance High Grey Level Emphasis</b>:
     * \f[ \textup{Small Distance High Grey Level Emphasis}= \frac{1}{N_s} \sum_x \sum_d \frac{x^2  m_{x,d}}{d^2}\f]
     * - <b>Grey Level Distance Zone::Large Distance Low Grey Level Emphasis</b>:
     * \f[ \textup{Large Distance Low Grey Level Emphasis}= \frac{1}{N_s} \sum_x \sum_d \frac{d^2 m_{x,d}}{x^2}\f]
     * - <b>Grey Level Distance Zone::Large Distance High Grey Level Emphasis</b>:
     * \f[ \textup{Large Distance High Grey Level Emphasis}= \frac{1}{N_s} \sum_x \sum_d \x^2 d^2 m_{x,d} \f]
     * - <b>Grey Level Distance Zone::Grey Level Non-Uniformity</b>:
     * \f[ \textup{Grey Level Non-Uniformity}= \frac{1}{N_s} \sum_x m_x^2 \f]
     * - <b>Grey Level Distance Zone::Grey Level Non-Uniformity Normalized</b>:
     * \f[ \textup{Grey Level Non-Uniformity Normalized}= \frac{1}{N_s^2} \sum_x m_x^2 \f]
     * - <b>Grey Level Distance Zone::Zone Distance Non-Uniformity</b>:
     * \f[ \textup{Grey Level Non-Uniformity}= \frac{1}{N_s} \sum_d m_d^2 \f]
     * - <b>Grey Level Distance Zone::Zone Distance Non-Uniformity Normalized</b>:
     * \f[ \textup{Grey Level Non-Uniformity Normalized}= \frac{1}{N_s^2} \sum_d m_d^2 \f]
     * - <b>Grey Level Distance Zone::Zone Percentage</b>: The ratio of zones to the possible zones:
     * \f[ \textup{Zone Percentage}= \frac{N_s}{N_v} \f]
     * - <b>Grey Level Distance Zone::Grey Level Mean</b>:
     * \f[ \textup{Grey Level Mean}= \mu_x = \sum_x \sum_d x p_{x,d} \f]
     * - <b>Grey Level Distance Zone::Grey Level Variance</b>:
     * \f[ \textup{Grey Level Variance} = \sum_x \sum_d \left(x - \mu_x \right)^2 p_{x,d} \f]
     * - <b>Grey Level Distance Zone::Zone Distance Mean</b>:
     * \f[ \textup{Zone Distance Mean}= \mu_d = \sum_x \sum_d d p_{x,d} \f]
     * - <b>Grey Level Distance Zone::Zone Distance Variance</b>:
     * \f[ \textup{Zone Distance Variance} = \sum_x \sum_d \left(d - \mu_d \right)^2 p_{x,d} \f]
     * - <b>Grey Level Distance Zone::Zone Distance Entropy </b>:
     * \f[ \textup{Zone Distance Entropy} = - \sum_x \sum_d p_{x,d} \textup{log}_2 ( p_{x,d} ) \f]
     * - <b>Grey Level Distance Zone::Grey Level Entropy </b>:
     * \f[ \textup{Grey Level Entropy} = - \sum_x \sum_d p_{x,d} \textup{log}_2 ( p_{x,d} ) \f]
     */
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

      virtual std::string GetCurrentFeatureEncoding() override;

      struct GIFGreyLevelDistanceZoneConfiguration
    {
      mitk::Image::Pointer distanceMask;

      mitk::IntensityQuantifier::Pointer Quantifier;

      unsigned int direction;
      double MinimumIntensity;
      double MaximumIntensity;
      int Bins;
      std::string prefix;
    };

    private:
  };

}
#endif //mitkGIFGreyLevelDistanceZone_h
