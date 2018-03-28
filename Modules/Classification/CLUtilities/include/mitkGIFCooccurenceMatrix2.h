#ifndef mitkGIFCooccurenceMatrix2_h
#define mitkGIFCooccurenceMatrix2_h

#include <mitkAbstractGlobalImageFeature.h>
#include <mitkBaseData.h>
#include <MitkCLUtilitiesExports.h>

#include <Eigen/src/Core/Array.h>

namespace mitk
{

  /**
  * \brief Calculates features based on the co-occurence matrix.
  *
  * The co-occurence matrix describes the relations between voxels in a specific direction. The elements \f$m_{i,k} \f$ of the
  * matrix count how often a voxel with the intensity \f$i \f$ has a neighbour in a certain direction with the intensity \f$ k \f$.
  * The direction for each matrix is given by a directed vector \f$ \overrightarrow{d} \f$.
  *
  * It is important to calculate the matrices for all possible directions in order to obtain a rotation invariant feature.
  * For the 3D case, this means that there are 26 possible directions. Using the symmetrical properties of the co-occurence
  * matrix, it is then possible to calculate the features in all directions looking at 13 different directions.
  *
  * The standard length of the vector is 1, e.g. looking at direct neighbours. It is possible to look at more
  * distance neighbours. This is achieved using the parameter <b>range</b>  which defines the distance between
  * two neighbouring voxels in number of voxels. The default value for this is 1. It can be changes using the Method
  * SetRange() or by passing the option <b>cooc2::range</b>.
  *
  * There are two possible ways of combining the information obtained from the multiple directions. The first option
  * is to calculate a common matrix for all directions and then use this matrix to calculate the describing features.
  * The second method is to calculate a matrix for each direction, obtain the features and then report the mean and
  * standard value of these features. Both mehtods are calcuated by this filters and reported, distinguisehd by either
  * an "Overall" if a single matrix is used, a "Mean" for the mean Value, or an "Std.Dev." for the standard deviation.
  *
  * The connected areas are based on the binned image, the binning parameters can be set via the default
  * parameters as described in AbstractGlobalImageFeature. The intensity used for the calculation is
  * always equal to the bin number. It is also possible to determine the
  * dimensionality of the neighbourhood using direction-related commands as described in AbstractGlobalImageFeature.
  * No other options are possible beside these two options.
  *
  * This feature calculator is activated by the option <b>-cooccurence2</b> or <b>-cooc2</b>.
  *
  * The features are calculated based on a mask. It is assumed that the mask is
  * of the type of an unsigned short image. All voxels with the value 1 are treated as masked.
  *
  * The following features are defined. We always give the notation for the overall matrix feature
  * although those for the mean and std.dev. are basically equal. In the name, <Range> is replace
  * by the distance of the neighbours. For the definitions of the feature, the probability of each
  * intensity pair (i,k) \f$ p_{i,k} = \frac{m_{i,k}}{\sum_i \sum_k m_{i,k}} \f$.
  *
  * In addition, the marginal sum \f$ p_{i,\cdot} = p_{\cdot,k=i} = \sum_k p_{i,k} \f$, which is
  * identical for both axis due to the symetrical nature of the matrix. Furthermore, the diagonal and
  * cross diagnoal features are used:
  * \f[ p_{i-k}(l) = \sum_i \sum_k p_{i,k} \delta(l - \| i -k \| ) \enspace \enspace l = 0, \dots, N_g -1  \f]
  * \f[ p_{i+k}(l) = \sum_i \sum_k p_{i,k} \delta(l - ( i + k ) ) \enspace \enspace l = 2, \dots, 2 N_g \f]
  * Here, \f$ \delta(x) \f$ is the dirac function, which is one for \f$x=0 \f$ and zero otherwise.
  * - <b>Co-occurenced Based Features (<Range>)::Overall Joint Maximum</b>:
  * \f[ \textup{Joint Maximum}=  \textup{max}(p_{i,k}) \f]
  * - <b>Co-occurenced Based Features (<Range>)::Overall Joint Average</b>:
  * \f[ \textup{Joint Average} = \mu_{ja} = \sum_i \sum_k i p_{i,k} \f]
  * - <b>Co-occurenced Based Features (<Range>)::Overall Joint Variance</b>:
  * \f[ \textup{Joint Variance} = \sum_i \sum_k (i - \mu_{ja})^2 p_{i,k} \f]
  * - <b>Co-occurenced Based Features (<Range>)::Overall Joint Entropy</b>:
  * \f[ \textup{Joint Entropy} = e_j =  - \sum_i \sum_k p_{i,k} \textup{log}_2 p_{i,k} \f]
  * - <b>Co-occurenced Based Features (<Range>)::Overall Row Maximum</b>:
  * \f[ \textup{Row Maximum}=  \textup{max}(p_{i,\cdot}) \f]
  * - <b>Co-occurenced Based Features (<Range>)::Overall Row Average</b>:
  * \f[ \textup{Row Average} = \mu_{ra} = \sum_i i p_{i,\cdot} \f]
  * - <b>Co-occurenced Based Features (<Range>)::Overall Row Variance</b>:
  * \f[ \textup{Row Variance} = \sigma^2_{i, \cdot} = \sum_i (i - \mu_{ra})^2 p_{i,\cdot} \f]
  * - <b>Co-occurenced Based Features (<Range>)::Overall Row Entropy</b>:
  * \f[ \textup{Row Entropy} = e_r = - \sum_i p_{i,\cdot} \textup{log}_2 p_{i,\cdot} \f]
  * - <b>Co-occurenced Based Features (<Range>)::Overall First Row-Column Entropy</b>:
  * \f[ \textup{First Row-Column Entropy} = e_1 = - \sum_i \sum_k p_{i,k} \textup{log}_2 ( p_{i,\cdot} p_{\cdot,k}) \f]
  * - <b>Co-occurenced Based Features (<Range>)::Overall Second Row-Column Entropy</b>:
  * \f[ \textup{Second Row-Column Entropy} = e_2 = - \sum_i \sum_k p_{i,\cdot} p_{\cdot,k} \textup{log}_2 ( p_{i,\cdot} p_{\cdot,k}) \f]
  * - <b>Co-occurenced Based Features (<Range>)::Overall Difference Average</b>:
  * \f[ \textup{Difference Average} = \mu_{da} = \sum_l l p_{i-k}(l) \f]
  * - <b>Co-occurenced Based Features (<Range>)::Overall Difference Variance</b>:
  * \f[ \textup{Difference Variance} = \sum_l (i - \mu_{da})^2 p_{i-k}(l) \f]
  * - <b>Co-occurenced Based Features (<Range>)::Overall Difference Entropy</b>:
  * \f[ \textup{Difference Entropy} = - \sum_l p_{i-k}(l) \textup{log}_2 p_{i-k}(l) \f]
  * - <b>Co-occurenced Based Features (<Range>)::Overall Sum Average</b>:
  * \f[ \textup{Sum Average} = \mu_{sa} = \sum_l l p_{i+k}(l) \f]
  * - <b>Co-occurenced Based Features (<Range>)::Overall Sum Variance</b>:
  * \f[ \textup{Sum Variance} = \sum_l (i - \mu_{sa})^2 p_{i+k}(l) \f]
  * - <b>Co-occurenced Based Features (<Range>)::Overall Sum Entropy</b>:
  * \f[ \textup{Sum Entropy} = - \sum_l p_{i+k}(l) \textup{log}_2 p_{i+k}(l) \f]
  * - <b>Co-occurenced Based Features (<Range>)::Overall Angular Second Moment</b>:
  * \f[ \textup{Angular Second Moment} = \sum_i \sum_k p^2_{i,k} \f]
  * - <b>Co-occurenced Based Features (<Range>)::Overall Contrast</b>:
  * \f[ \textup{Contrast} = \sum_i \sum_k (i-k)^2 p_{i,k} \f]
  * - <b>Co-occurenced Based Features (<Range>)::Overall Dissimilarity</b>:
  * \f[ \textup{Dissimilarity} = \sum_i \sum_k \| i-k\|  p^2_{i,k} \f]
  * - <b>Co-occurenced Based Features (<Range>)::Overall Inverse Difference</b>:
  * \f[ \textup{Inverse Difference} = \sum_i \sum_k \frac{p_{i,k}}{1+\| i-k\|} \f]
  * - <b>Co-occurenced Based Features (<Range>)::Overall Inverse Difference Normalized</b>:
  * \f[ \textup{Inverse Difference Normalized} = \sum_i \sum_k \frac{p_{i,k}}{1+\frac{\| i-k\|}{N_g}} \f]
  * - <b>Co-occurenced Based Features (<Range>)::Overall Inverse Difference Moment</b>:
  * \f[ \textup{Inverse Difference Moment} = \sum_i \sum_k \frac{p_{i,k}}{1+ ( i-k )^2} \f]
  * - <b>Co-occurenced Based Features (<Range>)::Overall Inverse Difference Moment Normalized</b>:
  * \f[ \textup{Inverse Difference Moment Normalized} = \sum_i \sum_k \frac{p_{i,k}}{1+\frac{( i-k ) ^2}{N_g}} \f]
  * - <b>Co-occurenced Based Features (<Range>)::Overall Inverse Variance</b>:
  * \f[ \textup{Inverse Difference Moment Normalized} = \sum_i \sum_k \frac{p_{i,k}}{(i-k)^2} \f]
  * - <b>Co-occurenced Based Features (<Range>)::Overall Correlation</b>:
  * \f[ \textup{Correlation} = \frac{1}{\sigma^2_{i,\cdot}} \sum_i \sum_k (i - \mu_{ra})(k - \mu_{ra}) p_{i,k} \f]
  * - <b>Co-occurenced Based Features (<Range>)::Overall Autocorrelation</b>:
  * \f[ \textup{Autocorrelation} = \sum_i \sum_k i k p_{i,k} \f]
  * - <b>Co-occurenced Based Features (<Range>)::Overall Cluster Tendency</b>:
  * \f[ \textup{Cluster Tendency} = \sum_i \sum_k (i + k - 2\mu_{ra})^2 p_{i,k} \f]
  * - <b>Co-occurenced Based Features (<Range>)::Overall Cluster Shade</b>:
  * \f[ \textup{Cluster Shade} = \sum_i \sum_k (i + k - 2\mu_{ra})^3 p_{i,k} \f]
  * - <b>Co-occurenced Based Features (<Range>)::Overall Cluster Prominence</b>:
  * \f[ \textup{Cluster Prominence} = \sum_i \sum_k (i + k - 2\mu_{ra})^4 p_{i,k} \f]
  * - <b>Co-occurenced Based Features (<Range>)::Overall First Measure of Information Correlation</b>:
  * \f[ \textup{First Measure of Information Correlation} = \frac{ e_j- e_1}{e_r} \f]
  * - <b>Co-occurenced Based Features (<Range>)::Overall Second Measure of Information Correlation</b>:
  * \f[ \textup{Second Measure of Information Correlation} = \sqrt{1- \exp(-2 (e_2 - e_j)} \f]
  */
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
      virtual std::string GetCurrentFeatureEncoding() override;

      itkGetConstMacro(Range,double);
      itkSetMacro(Range, double);

    struct GIFCooccurenceMatrix2Configuration
    {
      double range;
      unsigned int direction;

      double MinimumIntensity;
      double MaximumIntensity;
      int Bins;
      std::string prefix;
    };

    private:
      double m_Range;
  };

}
#endif //mitkGIFCooccurenceMatrix2_h
