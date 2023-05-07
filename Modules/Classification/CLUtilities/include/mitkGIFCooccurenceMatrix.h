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
    * \brief Calculates features based on the co-occurence matrix.
    *
    * This filter calculates features based on the Co-Occurence Matrix.
    *
    * \warning{ This is a legacy class only. If possible, avoid to use it. Use
    * GIFCooccurenceMatrix2 instead.}
    */
    public:
      mitkClassMacro(GIFCooccurenceMatrix,AbstractGlobalImageFeature);
      itkFactorylessNewMacro(Self);
      itkCloneMacro(Self);

      GIFCooccurenceMatrix();

      FeatureListType CalculateFeatures(const Image* image, const Image* mask, const Image* maskNoNAN) override;
      using Superclass::CalculateFeatures;

      itkGetConstMacro(Ranges, std::vector<double>);
      void SetRanges(std::vector<double> ranges);
      void SetRange(double range);

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
