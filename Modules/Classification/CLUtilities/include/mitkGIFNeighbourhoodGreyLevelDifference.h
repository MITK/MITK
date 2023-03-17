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
  class MITKCLUTILITIES_EXPORT GIFNeighbourhoodGreyLevelDifference : public AbstractGlobalImageFeature
  {
  public:
    mitkClassMacro(GIFNeighbourhoodGreyLevelDifference,AbstractGlobalImageFeature);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    GIFNeighbourhoodGreyLevelDifference();

    FeatureListType CalculateFeatures(const Image* image, const Image* mask, const Image* maskNoNAN) override;
    using Superclass::CalculateFeatures;

    itkGetConstMacro(Ranges, std::vector<double>);
    void SetRanges(std::vector<double> ranges);
    void SetRange(double range);

    itkGetConstMacro(UseCTRange, bool);
    itkSetMacro(UseCTRange, bool);

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
