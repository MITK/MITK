/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkGIFCooccurenceMatrix_h
#define mitkGIFCooccurenceMatrix_h

#include <mitkAbstractGlobalImageFeature.h>
#include <mitkBaseData.h>
#include <MitkCLUtilitiesExports.h>

namespace mitk
{

  class MITKCLUTILITIES_EXPORT GIFCooccurenceMatrix : public AbstractGlobalImageFeature
  {
    /**
    * \brief Calculates features based on the co-occurrence matrix.
    *
    * This filter calculates features based on the Co-Occurrence Matrix.
    *
    * \warning{ This is a legacy class only. If possible, avoid to use it. Use
    * GIFCooccurenceMatrix2 instead.}
    */
    public:
      mitkClassMacro(GIFCooccurenceMatrix,AbstractGlobalImageFeature);
      itkFactorylessNewMacro(Self);
      itkCloneMacro(Self);

      GIFCooccurenceMatrix();

      /**
       * \brief Calculate co-occurrence matrix features for the given image and mask.
       *
       * \param[in] image The input intensity image.
       * \param[in] mask The binary mask defining the region of interest.
       * \param[in] maskNoNAN The mask with NaN voxels excluded.
       * \return A list of computed feature name-value pairs.
       */
      FeatureListType CalculateFeatures(const Image* image, const Image* mask, const Image* maskNoNAN) override;
      using Superclass::CalculateFeatures;

      /** \brief Get the distance ranges used for co-occurrence computation. */
      itkGetConstMacro(Ranges, std::vector<double>);

      /**
       * \brief Set multiple distance ranges for co-occurrence computation.
       * \param[in] ranges Vector of distances between co-occurring voxels.
       */
      void SetRanges(std::vector<double> ranges);

      /**
       * \brief Set a single distance range for co-occurrence computation.
       * \param[in] range The distance between co-occurring voxels.
       */
      void SetRange(double range);

      /**
       * \brief Add command line arguments for configuring this feature class.
       * \param[in,out] parser The command line parser to add arguments to.
       */
      void AddArguments(mitkCommandLineParser& parser) const override;

  protected:

    std::string GenerateLegacyFeatureNamePart(const FeatureID& id) const override;
    std::string GenerateLegacyFeatureEncoding(const FeatureID& id) const override;

    FeatureListType DoCalculateFeatures(const Image* image, const Image* mask) override;

    void ConfigureSettingsByParameters(const ParametersType& parameters) override;

  private:
    std::vector<double> m_Ranges;
  };

}
#endif
