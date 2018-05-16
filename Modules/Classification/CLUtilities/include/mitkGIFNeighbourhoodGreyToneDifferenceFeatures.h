/*===================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef mitkGIFNeighbourhoodGreyToneDifferenceFeatures_h
#define mitkGIFNeighbourhoodGreyToneDifferenceFeatures_h

#include <mitkAbstractGlobalImageFeature.h>
#include <mitkBaseData.h>
#include <MitkCLUtilitiesExports.h>

namespace mitk
{
  /**
  * \brief Calculates the Neighbourhood Grey Tone Difference Features.
  *
  * This class can be used to calculate Neighbourhood Grey Tone Difference Features which have been introduced in
  * Amadasun and King: Textural features corresponding to textural properties. IEEE Transactions on Systems, Man and Cybernetricy, 1989.
  *
  * The Neighbourhood Grey Tone Difference (NGTD) is based on a table and is calcualted using
  * a defined neighbourhood for each voxel. Within this neighbourhood, the mean intensity of the neighbouring
  * voxel is calculated \f$A_i\f$, i.e. the mean intensity of the neighbourhood excluding the center voxel.
  * Based on this a table with four columns is calculated. The first column represents the voxel
  * value, or in our implementation, the histogram bin index \f$i\f$. The second column represents the
  * number of voxel with this intensity value \f$n_i\f$. The proability for each intensity value \f$p_i\f$, which
  * is equal to the bin probability. And the sum of the absolut differences of the intensity of all voxels with this
  * intensity and the mean intensity of their neighbourhood: \f[s_i = \sum \left \| i- A_i \right \| \f].
  * Additional \f$ N_v\f$ is the number of voxels, \f$ N_g \f$ the number of bins, and \f$N_{g,p}\f$ the number of
  * bins with a non-zero probability.
  *
  * This feature calculator is activated by the option <b>-neighbourhood-grey-tone-difference</b> or <b>-ngtd</b>.
  *
  * The range that is used to calculate the local intensity can be set using the function <b>SetRange</b>.
  * To set it with parameters set the option <b>ngtd::range</b> which expects an int value n that is
  * interpreted as voxel count. The neighbourhood includes symetrical n voxels additional
  * to the center voxel in each directions. The default value for this parameter is 1.
  *
  * The features are calculated based on a mask. It is assumed that the mask is
  * of the type of an unsigned short image and all voxels with an value of greater than zero
  * are treated as masked.
  *
  * For the definition of the features we use the sum, this is always the sum over the bins of the histogram.
  * If the denominator of a feature evaluates to zero, the feature is defined as zero.
  * The resulting features are:
  * - <b>Neighbourhood Grey Tone Difference::Coarsness</b>: The coarsness is defined as :
  * \f[ \textup{Coarsness}= \frac{1}{\sum p_i s_i} \f]
  * - <b>Neighbourhood Grey Tone Difference::Contrast</b>: The contrast is defined as :
  * \f[ \textup{contrast}= \left( \frac{1}{N_{g,p} ( N_{g,p} - 1) } \sum_i \sum_j p_i p_j (i-j)^2 \right) \left( \frac{1}{N_v} \sum s_i  \right) \f]
  * - <b>Neighbourhood Grey Tone Difference::Busyness</b>: for all bins with a non-zero probability
  * \f[ \textup{busyness} = \frac{\sum p_i s_i}{\sum_i \sum_j \left \| i p_i - j p_j \right \| } \f]
  * - <b>Neighbourhood Grey Tone Difference::Complexity</b>: for all bins with a non-zero probability
  * \f[ \textup{complexity} = \frac{1}{N_v} \sum_i \sum_j \left \| i - j \right \| \frac{p_i s_i + p_j s_j}{p_i + p_j} \f]
  * - <b>Neighbourhood Grey Tone Difference::Strength</b>: for all bins with a non-zero probability
  * \f[ \textup{strength} = \frac{\sum_i \sum_j (p_i + p_j) (i + j)^2}{\sum_i s_i } \f]
  */
  class MITKCLUTILITIES_EXPORT GIFNeighbourhoodGreyToneDifferenceFeatures : public AbstractGlobalImageFeature
  {
  public:
    mitkClassMacro(GIFNeighbourhoodGreyToneDifferenceFeatures, AbstractGlobalImageFeature);
      itkFactorylessNewMacro(Self)
      itkCloneMacro(Self)

      GIFNeighbourhoodGreyToneDifferenceFeatures();

    /**
    * \brief Calculates the Cooccurence-Matrix based features for this class.
    */
    virtual FeatureListType CalculateFeatures(const Image::Pointer & image, const Image::Pointer &feature);

    /**
    * \brief Returns a list of the names of all features that are calculated from this class
    */
    virtual FeatureNameListType GetFeatureNames();

    virtual std::string GetCurrentFeatureEncoding() override;

    virtual void CalculateFeaturesUsingParameters(const Image::Pointer & feature, const Image::Pointer &mask, const Image::Pointer &maskNoNAN, FeatureListType &featureList);
    virtual void AddArguments(mitkCommandLineParser &parser);

    itkSetMacro(Range, int);
    itkGetConstMacro(Range, int);

  private:
    int m_Range;
  };
}
#endif //mitkGIFNeighbourhoodGreyToneDifferenceFeatures_h
