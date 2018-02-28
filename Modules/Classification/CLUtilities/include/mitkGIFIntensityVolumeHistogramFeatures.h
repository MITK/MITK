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

#ifndef mitkGIFIntensityVolumeHistogramFeatures_h
#define mitkGIFIntensityVolumeHistogramFeatures_h

#include <mitkAbstractGlobalImageFeature.h>
#include <mitkBaseData.h>
#include <MitkCLUtilitiesExports.h>

namespace mitk
{
  /**
  * \brief Calculates the Intensity Volume Histogram features
  *
  * This class can be used to calculate the volume histogram and features that are calculated from
  * it. It is based on the intensity-volume histogram (IVH) which describes the relation between the
  * grey level index i (and the corresponding intensity \f§x_i\f$) and the volume fraction \f$f\f$ that
  * with an intensity that is equal or greater than \f$x_i\f$. This feature is original proposed in
  * El Naqa et al. Exploring feature-based approaches in PET images for prediciting cancer treatment outcomes.
  * Pattern recognition 2009.
  *
  * This feature calculator is activated by the option "<b>-intensity-volume-histogram</b>" or "<b>-ivoh</b>".
  * Beside the configuration of the histogram, which follows the describtion given in AbstractGlobalImageFeature
  * there are no additional parameters to configure this feature. Remark that, different from other features,
  * the number of bins is 1000 by default and not 256.
  *
  * The features are calculated based on a mask. It is assumed that the mask is
  * of the type of an unsigned short image and all voxels with an value of greater than zero
  * are treated as masked.
  *
  * The resulting features are:
  * - <b>Intensity Volume Histogram::Volume fration at 0.10 intensity</b>: The volume fraction with an intensity
  * of greater or equal to 10% of the maximum intensity.
  * - <b>Intensity Volume Histogram::Volume fration at 0.90 intensity</b>: The volume fraction with an intensity
  * of greater or equal to 90% of the maximum intensity.
  * - <b>Intensity Volume Histogram::Intensity at 0.10 volume</b>: The highest intensity so that at least
  * 10% of the masked image area has the same or higher intensity.
  * - <b>Intensity Volume Histogram::Intensity at 0.90 volume</b>: The highest intensity so that at least
  * 10% of the masked image area has the same or higher intensity.
  * - <b>Intensity Volume Histogram::Difference volume fration at 0.10 and 0.90 intensity</b>: The difference
  * between the volume fraction at 10% intensity and 90% intensity.
  * - <b>Intensity Volume Histogram::Difference intensity at 0.10 and 0.90 volume</b>: The intensity difference
  * between the intenstiy of 90% of the volume and 10% volume.
  * - <b>Intensity Volume Histogram::Area under IVH curve</b>: If the IVH is interpreted as curve, this value represents
  * the area under the curve. It is calculated using the bin indexes rather than the intensity values.
  */
  class MITKCLUTILITIES_EXPORT GIFIntensityVolumeHistogramFeatures : public AbstractGlobalImageFeature
  {
  public:
    mitkClassMacro(GIFIntensityVolumeHistogramFeatures, AbstractGlobalImageFeature);
      itkFactorylessNewMacro(Self)
      itkCloneMacro(Self)

      GIFIntensityVolumeHistogramFeatures();

    /**
    * \brief Calculates the Cooccurence-Matrix based features for this class.
    */
    virtual FeatureListType CalculateFeatures(const Image::Pointer & image, const Image::Pointer &feature);

    /**
    * \brief Returns a list of the names of all features that are calculated from this class
    */
    virtual FeatureNameListType GetFeatureNames();
    virtual void AddArguments(mitkCommandLineParser &parser);

    virtual void CalculateFeaturesUsingParameters(const Image::Pointer & feature, const Image::Pointer &mask, const Image::Pointer &maskNoNAN, FeatureListType &featureList);

    virtual std::string GetCurrentFeatureEncoding() override;

  private:
  };
}
#endif //mitkGIFIntensityVolumeHistogramFeatures_h
