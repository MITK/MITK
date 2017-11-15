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

    virtual void CalculateFeaturesUsingParameters(const Image::Pointer & feature, const Image::Pointer &mask, const Image::Pointer &maskNoNAN, FeatureListType &featureList);
    virtual void AddArguments(mitkCommandLineParser &parser);

  private:
  };
}
#endif //mitkGIFNeighbourhoodGreyToneDifferenceFeatures_h
