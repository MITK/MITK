#ifndef mitkGIFNeighbouringGreyLevelDependenceFeatures_h
#define mitkGIFNeighbouringGreyLevelDependenceFeatures_h

#include <mitkAbstractGlobalImageFeature.h>
#include <mitkBaseData.h>
#include <MitkCLUtilitiesExports.h>

#include <Eigen/src/Core/Array.h>

namespace mitk
{
  /**
  * \brief Calculates the Neighbouring Grey Level Dependence Features
  *
  * The Neighbouring Grey Level Dependence Features were proposed by Sun and Wee (1983) and
  * capture the coarsness of the image texture. They are rotational invariant.
  *
  * The features are calculated on a matrix \f$ m \f$. To obtain the matrix, a neighbourhood
  * around each feature is calculated and the number of voxels within the neighbourhood that
  * are greater than the center voxel plus \f$ \alpha \f$ is counted. This is called the
  * number of dependence voxels. The matrix gives the
  * number of voxels with an intesity \f$ x \f$ and $\f d \f$ dependence neighbourhood voxels.
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
  *
  * In addition to this, the size of the neighbourhood can be controlled by setting the parameter
  * <b>ngld::range</b>. By default it is one. To pass more than one range, separate the ranges with
  * a semicolon. E.g. 1;2;3 would calculate the features for the ranges 1, 2, and 3.
  *
  * This feature calculator is activated by the option <b>-neighbouring-grey-level-dependence</b>
  * or <b>-ngld</b>.
  *
  * The features are calculated based on a mask. It is assumed that the mask is
  * a unsigned short image. All voxels with a value greater 0 are treated as masked.
  *
  * Several values are definied for the definition of the features. \f$ N_v \f$ is the number of masked voxels,
  * \f$N_s \f$ is the number of neighbourhoods, \f$ m_{x,\cdot} = \sum_d m{x,d} \f$ is the number of neighbourhoods
  * with a given intensity value, and likewise \f$ m_{\cdot, d} = \sum_x m{x,d} \f$ is the number of neighbourhoods
  * with a given number of dependence features:
  * - <b>Neighbouring Grey Level Dependence::Low Dependence Emphasis</b>:
  * \f[ \textup{Low dependence emphasis}= \frac{1}{N_s} \sum_d { \frac{m_{\cdot, d}}{d^2} } \f]
  * - <b>Neighbouring Grey Level Dependence::High Dependence Emphasis</b>:
  * \f[ \textup{High dependence emphasis}= \frac{1}{N_s} \sum_d { m_{\cdot, d} d^2} \f]
  * - <b>Neighbouring Grey Level Dependence::Low Grey Level Count Emphasis</b>:
  * \f[ \textup{Low grey level count emphasis}= \frac{1}{N_s} \sum_x { \frac{m_{x,\cdot}}{x^2} } \f]
  * - <b>Neighbouring Grey Level Dependence::High Grey Level Count Emphasis</b>:
  * \f[ \textup{High grey level count emphasis}= \frac{1}{N_s} \sum_x { m_{x,\cdot} x^2} \f]
  * - <b>Neighbouring Grey Level Dependence::Low Dependence Low Grey Level Emphasis</b>:
  * \f[ \textup{Low Dependence Low Grey Level Emphasis}= \frac{1}{N_s} \sum_x \sum_d { \frac{m_{x,d}}{x^2 d^2} } \f]
  * - <b>Neighbouring Grey Level Dependence::Low Dependence High Grey Level Emphasis</b>:
  * \f[ \textup{Low dependence high grey level emphasis}= \frac{1}{N_s} \sum_x \sum_d { \frac{x^2 m_{x,d}}{d^2} } \f]
  * - <b>Neighbouring Grey Level Dependence::High Dependence Low Grey Level Emphasis</b>:
  * \f[ \textup{High Dependence Low Grey Level Emphasis}= \frac{1}{N_s} \sum_x \sum_d { \frac{d^2 m_{x,d}}{x^2} } \f]
  * - <b>Neighbouring Grey Level Dependence::High Dependence High Grey Level Emphasis</b>:
  * \f[ \textup{High dependence high grey level emphasis}= \frac{1}{N_s} \sum_x \sum_d { x^2 d^2 m_{x,d} } \f]
  * - <b>Neighbouring Grey Level Dependence::Grey level nonuniformity</b>:
  * \f[ \textup{Grey level nonuniformity}= \frac{1}{N_s} \sum_x m_{x,\cdot}^2 \f]
  * - <b>Neighbouring Grey Level Dependence::Grey level nonuniformity normalized</b>:
  * \f[ \textup{Grey level nonuniformity normalized}= \frac{1}{N_s^2} \sum_x m_{x,\cdot}^2 \f]
  * - <b>Neighbouring Grey Level Dependence::Dependence Count Nonuniformity</b>:
  * \f[ \textup{Dependence count nonuniformity}= \frac{1}{N_s} \sum_d m_{\cdot, d}^2 \f]
  * - <b>Neighbouring Grey Level Dependence::Dependence Count Nonuniformity Normalized</b>:
  * \f[ \textup{Dependence count nonuniformity normalized}= \frac{1}{N_s^2} \sum_d m_{\cdot, d}^2 \f]
  * - <b>Neighbouring Grey Level Dependence::DEpendence Count Percentage</b> THe number of realized
  * neighbourhoods relativ to the theoretical maximum of realized neighbourhoods. This feature is always
  * one for this implementation as partial neighbourhoods are still considered.
  * - <b>Neighbouring Grey Level Dependence::Grey Level Mean</b>: The mean value of all grey level.
  * \f[ \textup{Grey Level Mean} = \mu_x = \frac{1}{N_s} \sum_x x m_{x,\cdot} \f]
  * - <b>Neighbouring Grey Level Dependence::Grey Level Variance</b>:
  * \f[ \textup{Grey level variance} = \frac{1}{N_s} \sum_x (x -mu_x)^2 m_{x, \cdot} \f]
  * - <b>Neighbouring Grey Level Dependence::Dependence Count Mean</b>: The mean value of all dependence counts.
  * \f[ \textup{Dependence count mean} = \mu_d = \frac{1}{N_s} \sum_d d m_{\cdot,d} \f]
  * - <b>Neighbouring Grey Level Dependence::Dependence Count Variance</b>:
  * \f[ \textup{Dependence count variance} = \frac{1}{N_s} \sum_d (d -mu_d)^2 m_{\cdot, d} \f]
  * - <b>Neighbouring Grey Level Dependence::Dependence Count Entropy</b>: This feature would be equivalent with
  * the Grey Level Entropy, which is therefore not included. It is based on the likelihood
  * for a given intensity- size combination \f$ p_{x,d} = \frac{m_{x,d}}{N_s} \f$. :
  * \f[ \textup{Dependence count entropy} = \sum_x \sum_d p_{x,d} \textup{log}_2 \left( p_{x,d} \right) \f]
  * - <b>Neighbouring Grey Level Dependence::Dependence Count Energy</b>: This feature would be equivalent with
  * the Grey Level Energy, which is therefore not included. It is based on the likelihood
  * for a given intensity- size combination \f$ p_{x,d} = \frac{m_{x,d}}{N_s} \f$. :
  * \f[ \textup{Dependence count energy} = \sum_x \sum_d p_{x,d}^2 \f]
  * - <b>Neighbouring Grey Level Dependence::Expected Neighbourhood Size</b>: The expected size of a
  * full neighbourhood. It depends on the dimension of the area that is looked at.
  * - <b>Neighbouring Grey Level Dependence::Average Neighbourhood Size</b>: The feature calculation
  * allows to consider partially masked neighbourhoods. Due to that, some neighbourhoods might be smaller.
  * This feature gives not the theoretical neighbourhood size but the average realized neighbourhood sizes.
  * - <b>Neighbouring Grey Level Dependence::Average Incomplete Neighbourhood Size</b>: Gives the average
  * size of all neighbourhoods that are not complete.
  * - <b>Neighbouring Grey Level Dependence::Percentage of complete Neighbourhoods</b>: Gives the percentage
  * of all complete neighbourhoods from all realized neighbourhoods.
  * - <b>Neighbouring Grey Level Dependence::Percentage of Dependence Neighbour Voxels</b>: Gives the
  * percentage of voxels in all neighbourhoods compared to the expected number of voxels.
 */
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

      virtual std::string GetCurrentFeatureEncoding() override;

      itkGetConstMacro(Range,double);
      itkSetMacro(Range, double);
    itkGetConstMacro(Alpha, int);
    itkSetMacro(Alpha, int);

    virtual void CalculateFeaturesUsingParameters(const Image::Pointer & feature, const Image::Pointer &mask, const Image::Pointer &maskNoNAN, FeatureListType &featureList);
    virtual void AddArguments(mitkCommandLineParser &parser);


    struct GIFNeighbouringGreyLevelDependenceFeatureConfiguration
    {
      double range;
      unsigned int direction;
      int alpha;

      double MinimumIntensity;
      double MaximumIntensity;
      int Bins;
      std::string FeatureEncoding;
    };

    private:
    double m_Range;
    int m_Alpha;
  };

}
#endif //mitkGIFNeighbouringGreyLevelDependenceFeature_h
