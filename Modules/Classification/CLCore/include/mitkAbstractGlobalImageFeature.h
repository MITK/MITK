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


#ifndef mitkAbstractGlobalImageFeature_h
#define mitkAbstractGlobalImageFeature_h

#include <MitkCLCoreExports.h>

#include <mitkBaseData.h>
#include <mitkImage.h>

// Eigen
#include <Eigen/Dense>

// STD Includes

// MITK includes
#include <mitkConfigurationHolder.h>

namespace mitk
{
class MITKCLCORE_EXPORT AbstractGlobalImageFeature : public BaseData
{
public:

  typedef std::vector< std::pair<std::string, double> > FeatureListType;
  typedef std::vector< std::string>                    FeatureNameListType;

  /**
  * \brief Calculates the feature of this abstact interface.
  */
  virtual FeatureListType CalculateFeatures(const Image::Pointer & image, const Image::Pointer &feature) = 0;

  /**
  * \brief Returns a list of the names of all features that are calculated from this class
  */
  virtual FeatureNameListType GetFeatureNames() = 0;

public:

//#ifndef DOXYGEN_SKIP

  virtual void SetRequestedRegionToLargestPossibleRegion() override {};
  virtual bool RequestedRegionIsOutsideOfTheBufferedRegion() override { return true; };
  virtual bool VerifyRequestedRegion() override { return false; };
  virtual void SetRequestedRegion (const itk::DataObject * /*data*/) override {};

  // Override
  virtual bool IsEmpty() const override
  {
    if(IsInitialized() == false)
      return true;
    const TimeGeometry* timeGeometry = const_cast<AbstractGlobalImageFeature*>(this)->GetUpdatedTimeGeometry();
    if(timeGeometry == NULL)
      return true;
    return false;
  }

//#endif // Skip Doxygen

};
}

#endif //mitkAbstractGlobalImageFeature_h
