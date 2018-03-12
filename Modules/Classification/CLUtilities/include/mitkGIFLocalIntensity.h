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
      itkFactorylessNewMacro(Self)
      itkCloneMacro(Self)

      GIFLocalIntensity();

    /**
    * \brief Calculates the Cooccurence-Matrix based features for this class.
    */
    virtual FeatureListType CalculateFeatures(const Image::Pointer & image, const Image::Pointer &feature);

    /**
    * \brief Returns a list of the names of all features that are calculated from this class
    */
    virtual FeatureNameListType GetFeatureNames();

    virtual void CalculateFeaturesUsingParameters(const Image::Pointer & feature, const Image::Pointer &mask, const Image::Pointer &maskNoNAN, FeatureListType &featureList);
    virtual void AddArguments(mitkCommandLineParser &parser);

    virtual std::string GetCurrentFeatureEncoding() override;

    itkGetConstMacro(Range, double);
    itkSetMacro(Range, double);

  private:

    double m_Range;
  };
}
#endif //mitkGIFLocalIntensity_h
