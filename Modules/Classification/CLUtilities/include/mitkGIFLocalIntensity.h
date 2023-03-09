/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkGIFLocalIntensity_h
#define mitkGIFLocalIntensity_h

#include <mitkAbstractGlobalImageFeature.h>
#include <mitkBaseData.h>
#include <MitkCLUtilitiesExports.h>

namespace mitk
{
  /**
  * \brief Calculates the local intensity features
  *
  * This class can be used to calcualte the local intensity. The local intensity is defined as the
  * mean intensity in a cube with the volume \f$ 1 \textup{cm}^3\f$ which correspond to a radius
  * of approximate 0.62 cm.
  *
  * This feature calculator is activated by the option <b>-local-intensity</b> or <b>-loci</b>.
  *
  * The range that is used to calculate the local intensity can be set using the function <b>SetRange</b>.
  * To set it with parameters set the option <b>loci::range</b> which expects an float value that is
  * interpreted as mm length of the radius. The default value is 6.2.
  *
  * The features are calculated based on a mask. It is assumed that the mask is
  * of the type of an unsigned short image and all voxels with an value of greater than zero
  * are treated as masked.
  *
  * The resulting features are:
  * - <b>Local Intensity::Local Intensity Peak</b>: This value is defined as the local intensity of the
  * voxel with the highest intensity. If there are multiple voxels with the highest intensity, the mean
  * of all local intensites of these voxels are reported.
  * - <b>Local Intensity::Global Intensity Peak</b>: This value is defined as the highest local intensity
  * that occur for all voxels within the mask.
  */
  class MITKCLUTILITIES_EXPORT GIFLocalIntensity : public AbstractGlobalImageFeature
  {
  public:
    mitkClassMacro(GIFLocalIntensity, AbstractGlobalImageFeature);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    GIFLocalIntensity();

    itkGetConstMacro(Range, double);
    itkSetMacro(Range, double);

    FeatureListType CalculateFeatures(const Image* image, const Image* mask, const Image* maskNoNAN) override;
    using Superclass::CalculateFeatures;

    void AddArguments(mitkCommandLineParser& parser) const override;

  protected:
    std::string GenerateLegacyFeatureEncoding(const FeatureID& id) const override;
    FeatureListType DoCalculateFeatures(const Image* image, const Image* mask) override;
    void ConfigureSettingsByParameters(const ParametersType& parameters) override;

  private:
    double m_Range;
  };
}
#endif
