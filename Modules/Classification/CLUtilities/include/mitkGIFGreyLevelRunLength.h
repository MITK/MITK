#ifndef mitkGIFGreyLevelRunLength_h
#define mitkGIFGreyLevelRunLength_h

#include <mitkAbstractGlobalImageFeature.h>
#include <mitkBaseData.h>
#include <MitkCLUtilitiesExports.h>

namespace mitk
{
  /**
  * \brief Calculates the Run Length based features.
  *
  * Grey Level Run Length based features are calcualted using the Run-Length-Matrix and were originally
  * defined by Galloway (1975). The basic idea behind this feature is to measure the length of
  * lines with similar intensity in an image. This allows to asses line-based structures in an image.
  * For this, the Run-Length-Matrix  created that gives the number of lines \f$ m_{x,l} \f$ with the intensity \f$ x \f$ and
  * the length of \f$ l \f$ with the given intensity.
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
  * This feature calculator is activated by the option <b>-run-length</b> or <b>-rl</b>.
  *
  * The features are calculated based on a mask. It is assumed that the mask is
  * of the same type as the input image. All voxels with a value greater 0.5 are treated as masked.
  *
  * Several values are definied for the definition of the features. \f$ N_v \f$ is the number of masked voxels,
  * \f$N_s \f$ is the number of different lines, \f$ m_{x,\cdot} = \sum_l m{x,l} \f$ is the number of all lines
  * with a given intensity value, and likewise \f$ m_{\cdot, l} = \sum_x m{x,l} \f$ is the number of all lines
  * with a given length. There are two options how to make this feature orientation-invariant. Either calculating a
  * single matrix for all directions and then extracting the features or by calculating a matrix per direction,
  * calculating all features and then reporting the mean and standard deviation for each feature. The result
  * of the first option is given with the extension "Comb.", the results for the second with "Std." and "Means".
  * All three options are always calculated, although we state only one in the following list:
  * - <b>Run Length::Short run emphasis Comb.</b>:
  * \f[ \textup{Short run emphasis}= \frac{1}{N_s} \sum_l { \frac{m_{\cdot, l}}{l^2} } \f]
  * - <b>Run Length::Long run emphasis Comb.</b>:
  * \f[ \textup{Long run emphasis}= \frac{1}{N_s} \sum_l { m_{\cdot, l} l^2} \f]
  * - <b>Run Length::Low grey level run emphasis Comb.</b>:
  * \f[ \textup{Low grey level run emphasis}= \frac{1}{N_s} \sum_x { \frac{m_{x,\cdot}}{x^2} } \f]
  * - <b>Run Length::High grey level run emphasis Comb.</b>:
  * \f[ \textup{High grey level run emphasis}= \frac{1}{N_s} \sum_x { m_{x,\cdot} x^2} \f]
  * - <b>Run Length::Short run low grey level emphasis Comb.</b>:
  * \f[ \textup{Short run low grey level emphasis}= \frac{1}{N_s} \sum_x \sum_l { \frac{m_{x,l}}{x^2 l^2} } \f]
  * - <b>Run Length::Short run high grey level emphasis Comb.</b>:
  * \f[ \textup{Short run high grey level emphasis}= \frac{1}{N_s} \sum_x \sum_l { \frac{x^2 m_{x,s}}{l^2} } \f]
  * - <b>Run Length::Long run low grey level emphasis Comb.</b>:
  * \f[ \textup{Long run low grey level emphasis}= \frac{1}{N_s} \sum_x \sum_l { \frac{l^2 m_{x,l}}{x^2} } \f]
  * - <b>Run Length::Long run high grey level emphasis Comb.</b>:
  * \f[ \textup{Long run high grey level emphasis}= \frac{1}{N_s} \sum_x \sum_l { x^2 l^2 m_{x,l} } \f]
  * - <b>Run Length::Grey level nonuniformity Comb.</b>:
  * \f[ \textup{Grey level nonuniformity}= \frac{1}{N_s} \sum_x m_{x,\cdot}^2 \f]
  * - <b>Run Length::Grey level nonuniformity normalized Comb.</b>:
  * \f[ \textup{Grey level nonuniformity normalized}= \frac{1}{N_s^2} \sum_x m_{x,\cdot}^2 \f]
  * - <b>Run Length::Run length nonuniformity</b>:
  * \f[ \textup{Run length nonuniformity}= \frac{1}{N_s} \sum_l m_{\cdot, l}^2 \f]
  * - <b>Run Length::Run length nonuniformity normalized</b>:
  * \f[ \textup{Run length nonuniformity normalized}= \frac{1}{N_s^2} \sum_l m_{\cdot, l}^2 \f]
  * - <b>Run Length::Run percentage</b>: The ratio of realized runs to the theoretical limit of zones:
  * \f[ \textup{Run percentage}= \frac{N_s}{N_v} \f]
  * - <b>Run Length::Grey level variance</b>:
  * \f[ \textup{Grey level variance} = \frac{1}{N_s} \sum_x (x -mu_x)^2 m_{x, \cdot} \f]
  * - <b>Run Length::Run length variance</b>:
  * \f[ \textup{Run length variance} = \frac{1}{N_s} \sum_l (l -mu_l)^2 m_{\cdot, l} \f]
  * - <b>Run Length::Run length entropy</b>: This feature would be equivalent with
  * the Grey Level Entropy, which is therefore not included. It is based on the likelihood
  * for a given intensity- size combination \f$ p_{x,s} = \frac{m_{x,s}}{N_s} \f$. :
  * \f[ \textup{Run length entropy} = \sum_x \sum_s p_{x,s} \textup{log}_2 \left( p{_x,s} \right) \f]
  * - <b>Run Length::Number of runs</b>: The overall number of realized runs:
  * \f[ \textup{Number of runs} = \sum m_{x, l} \f]
  */
  class MITKCLUTILITIES_EXPORT GIFGreyLevelRunLength : public AbstractGlobalImageFeature
  {
  public:
    mitkClassMacro(GIFGreyLevelRunLength,AbstractGlobalImageFeature)
      itkFactorylessNewMacro(Self)
      itkCloneMacro(Self)

      GIFGreyLevelRunLength();

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

    struct ParameterStruct
    {
      unsigned int m_Direction;

      double MinimumIntensity;
      double MaximumIntensity;
      int Bins;
      std::string featurePrefix;
    };

  private:

  };
}
#endif //mitkGIFGreyLevelRunLength_h
