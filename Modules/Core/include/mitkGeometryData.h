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


#ifndef MITKGEOMETRYDATA_H_HEADER_INCLUDED_C19C01E2
#define MITKGEOMETRYDATA_H_HEADER_INCLUDED_C19C01E2

#include "mitkBaseData.h"

namespace mitk {

//##Documentation
//## @brief Data class only having a BaseGeometry but not containing
//## any specific data.
//##
//## Only implements pipeline methods which are abstract in BaseData.
//## @ingroup Geometry
class MITKCORE_EXPORT GeometryData : public BaseData
{
public:
  mitkClassMacro(GeometryData, BaseData);

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  virtual void UpdateOutputInformation() override;

  virtual void SetRequestedRegionToLargestPossibleRegion() override;

  virtual bool RequestedRegionIsOutsideOfTheBufferedRegion() override;

  virtual bool VerifyRequestedRegion() override;

  virtual void SetRequestedRegion( const itk::DataObject *data) override;

  virtual void CopyInformation(const itk::DataObject *data) override;

protected:
  GeometryData();

  virtual ~GeometryData();
};

/**
* @brief Equal Compare two GeometryData objects for equality, returns true if found equal.
* @ingroup MITKTestingAPI
* @param rightHandSide GeometryData to compare.
* @param leftHandSide GeometryData to compare.
* @param eps Epsilon to use for floating point comparison. Most of the time mitk::eps will be sufficient.
* @param verbose Flag indicating if the method should give a detailed console output.
* @return True if every comparison is true, false in any other case.
*/
MITKCORE_EXPORT bool Equal( const mitk::GeometryData& leftHandSide, const mitk::GeometryData& rightHandSide, mitk::ScalarType eps, bool verbose);

} // namespace mitk
#endif /* MITKGEOMETRYDATA_H_HEADER_INCLUDED_C19C01E2 */
