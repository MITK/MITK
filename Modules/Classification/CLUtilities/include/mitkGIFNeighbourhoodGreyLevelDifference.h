/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkGIFNeighbourhoodGreyLevelDifference_h
#define mitkGIFNeighbourhoodGreyLevelDifference_h

#include <mitkAbstractGlobalImageFeature.h>
#include <mitkBaseData.h>
#include <MitkCLUtilitiesExports.h>

namespace mitk
{
  /**
   * \brief Calculates features based on the neighbourhood grey level difference.
   *
   * This is a legacy class. Consider using GIFNeighbourhoodGreyToneDifferenceFeatures instead.
   *
   * \sa GIFNeighbourhoodGreyToneDifferenceFeatures
   * \sa AbstractGlobalImageFeature
   */
  class MITKCLUTILITIES_EXPORT GIFNeighbourhoodGreyLevelDifference : public AbstractGlobalImageFeature
  {
  public:
    mitkClassMacro(GIFNeighbourhoodGreyLevelDifference,AbstractGlobalImageFeature);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    GIFNeighbourhoodGreyLevelDifference();

    /**
     * \brief Calculate neighbourhood grey level difference features for the given image and mask.
     *
     * \param[in] image The input intensity image.
     * \param[in] mask The binary mask defining the region of interest.
     * \param[in] maskNoNAN The mask with NaN voxels excluded.
     * \return A list of computed feature name-value pairs.
     */
    FeatureListType CalculateFeatures(const Image* image, const Image* mask, const Image* maskNoNAN) override;
    using Superclass::CalculateFeatures;

    /** \brief Get the distance ranges for neighbourhood computation. */
    itkGetConstMacro(Ranges, std::vector<double>);

    /**
     * \brief Set multiple distance ranges for neighbourhood computation.
     * \param[in] ranges Vector of neighbourhood distances.
     */
    void SetRanges(std::vector<double> ranges);

    /**
     * \brief Set a single distance range for neighbourhood computation.
     * \param[in] range The neighbourhood distance.
     */
    void SetRange(double range);

    /** \brief Get whether the CT intensity range is used. */
    itkGetConstMacro(UseCTRange, bool);
    /** \brief Set whether to use the CT intensity range. */
    itkSetMacro(UseCTRange, bool);

    /**
     * \brief Add command line arguments for configuring this feature class.
     * \param[in,out] parser The command line parser to add arguments to.
     */
    void AddArguments(mitkCommandLineParser &parser) const override;

  protected:
    std::string GenerateLegacyFeatureName(const FeatureID& id) const override;

    FeatureListType DoCalculateFeatures(const Image* image, const Image* mask) override;

    void ConfigureSettingsByParameters(const ParametersType& parameters) override;

  private:
    std::vector<double> m_Ranges;
    bool m_UseCTRange;
  };
}
#endif
