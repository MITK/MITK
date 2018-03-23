#ifndef mitkGIFGreyLevelSizeZone_h
#define mitkGIFGreyLevelSizeZone_h

#include <mitkAbstractGlobalImageFeature.h>
#include <mitkBaseData.h>
#include <MitkCLUtilitiesExports.h>

#include <Eigen/src/Core/Array.h>

namespace mitk
{
  class MITKCLUTILITIES_EXPORT GIFGreyLevelSizeZone : public AbstractGlobalImageFeature
  {
    /**
    * \brief Calculates the Grey level size zone based features.
    *
    * Grey level size zone based features are similar to Grey Level Cooccurence features. But instead
    * of measuring the similarity within a given neighbourhood, the size of areas with the same intensity
    * is assessed. For this, a matrix is created that gives the number of areas \f$ m_{x,s} \f$ with the intensity \f$ x \f$ and
    * the size of \f$ s \f$. Each area is specified as connected voxels with the given intensity.
    *
    * The image is quantified prior to the calculation of the features. This reduces the number of
    * available intensity values. Instead of using the pure intensity value, the features are
    * calculated using the number of the bins as intensity value \f$ x_i \f$. The parameter of the
    * quantification of the image can be controlled using the general binning parameters as defined
    * in AbstractGlobalImageFeature.
    *
    * By default, the calculation is based on a 26 neighourhood for 3D and a 8 neighbourhood in 2D. It is further
    * possible to exclude directions from the calculation, e.g. calculating the feature in 2D, even if a
    * 3D image is passed. This is controlled by  determine the
    * dimensionality of the neighbourhood using direction-related commands as described in AbstractGlobalImageFeature.
    * No other options are possible beside these two options.
    *
    * This feature calculator is activated by the option <b>-grey-level-sizezone</b> or <b>-glsz</b>.
    *
    * The features are calculated based on a mask. It is assumed that the mask is
    * of the type of an unsigned short image. All voxels with the value 1 are treated as masked.
    *
    * Several values are definied for the definition of the features. \f$ N_v \f$ is the number of masked voxels,
    * \f$N_s \f$ is the number of different zones, \f$ m_{x,\cdot} = \sum_s m{x,s} \f$ is the number of all areas
    * with a given intensity value, and likewise \f$ m_{\cdot, s} = \sum_x m{x,s} \f$ is the number of all areas
    * with a given size. The features are then defined as:
    * - <b>Grey Level Size Zone::Small Zone Emphasis</b>:
    * \f[ \textup{Small Zone Emphasis}= \frac{1}{N_s} \sum_s { \frac{m_{\cdot, s}}{s^2} } \f]
    * - <b>Grey Level Size Zone::Large Zone Emphasis</b>:
    * \f[ \textup{Large Zone Emphasis}= \frac{1}{N_s} \sum_s { m_{\cdot, s} s^2} \f]
    * - <b>Grey Level Size Zone::Low Grey Level Zone Emphasis</b>:
    * \f[ \textup{Low Grey Level Zone Emphasis}= \frac{1}{N_s} \sum_x { \frac{m_{x,\cdot}}{x^2} } \f]
    * - <b>Grey Level Size Zone::High Grey Level Zone Emphasis</b>:
    * \f[ \textup{High Grey Level Zone Emphasis}= \frac{1}{N_s} \sum_x { m_{x,\cdot} x^2} \f]
    * - <b>Grey Level Size Zone::Small Zone Low Grey Level Emphasis</b>:
    * \f[ \textup{Small Zone Low Grey Level Emphasis}= \frac{1}{N_s} \sum_x \sum_s { \frac{m_{x,s}}{x^2 s^2} } \f]
    * - <b>Grey Level Size Zone::Small Zone High Grey Level Emphasis</b>:
    * \f[ \textup{Small Zone High Grey Level Emphasis}= \frac{1}{N_s} \sum_x \sum_s { \frac{x^2 m_{x,s}}{s^2} } \f]
    * - <b>Grey Level Size Zone::Large Zone Low Grey Level Emphasis</b>:
    * \f[ \textup{Large Zone Low Grey Level Emphasis}= \frac{1}{N_s} \sum_x \sum_s { \frac{s^2 m_{x,s}}{x^2} } \f]
    * - <b>Grey Level Size Zone::Large Zone High Grey Level Emphasis</b>:
    * \f[ \textup{Large Zone High Grey Level Emphasis}= \frac{1}{N_s} \sum_x \sum_s { x^2 s^2 m_{x,s} } \f]
    * - <b>Grey Level Size Zone::Grey Level Non-Uniformity</b>:
    * \f[ \textup{Grey Level Non-Uniformity}= \frac{1}{N_s} \sum_x m_{x,\cdot}^2 \f]
    * - <b>Grey Level Size Zone::Grey Level Non-Uniformity Normalized</b>:
    * \f[ \textup{Grey Level Non-Uniformity Normalized}= \frac{1}{N_s^2} \sum_x m_{x,\cdot}^2 \f]
    * - <b>Grey Level Size Zone::Zone Size Non-Uniformity</b>:
    * \f[ \textup{Zone Size Non-Uniformity}= \frac{1}{N_s} \sum_s m_{\cdot, s}^2 \f]
    * - <b>Grey Level Size Zone::Zone Size Non-Uniformity Normalized</b>:
    * \f[ \textup{Zone Size Non-Uniformity Normalized}= \frac{1}{N_s^2} \sum_s m_{\cdot, s}^2 \f]
    * - <b>Grey Level Size Zone::Zone Percentage</b>: The ratio of realized areas to the theoretical limit of zones:
    * \f[ \textup{Zone Percentage}= \frac{N_s}{N_v} \f]
    * - <b>Grey Level Size Zone::Grey Level Mean</b>:
    * \f[ \textup{Grey Level Mean} = \mu_x = \frac{1}{N_s} \sum_x x m_{x, \cdot} \f]
    * - <b>Grey Level Size Zone::Grey Level Variance</b>:
    * \f[ \textup{Grey Level Variance} = \frac{1}{N_s} \sum_x (x -mu_x)^2 m_{x, \cdot} \f]
    * - <b>Grey Level Size Zone::Zone Size Mean</b>:
    * \f[ \textup{Zone Size Mean} = \mu_s = \frac{1}{N_s} \sum_s s m_{\cdot, s} \f]
    * - <b>Grey Level Size Zone::Grey Level Variance</b>:
    * \f[ \textup{Grey Level Variance} = \frac{1}{N_s} \sum_s (s -mu_s)^2 m_{\cdot, s} \f]
    * - <b>Grey Level Size Zone::Zone Size Entropy</b>: This feature would be equivalent with
    * the Grey Level Entropy, which is therefore not included. It is based on the likelihood
    * for a given intensity- size combination \f$ p_{x,s} = \frac{m_{x,s}}{N_s} \f$. :
    * \f[ \textup{Zone Size Entropy} = \sum_x \sum_s p_{x,s} \textup{log}_2 \left( p{_x,s} \right) \f]
    */
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
      virtual std::string GetCurrentFeatureEncoding() override;

      struct GIFGreyLevelSizeZoneConfiguration
    {
      unsigned int direction;

      double MinimumIntensity;
      double MaximumIntensity;
      int Bins;
      std::string prefix;
    };
  };

}
#endif //mitkGIFGreyLevelSizeZone_h
