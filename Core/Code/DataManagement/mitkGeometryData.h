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
class MITK_CORE_EXPORT GeometryData : public BaseData
{
public:
  mitkClassMacro(GeometryData, BaseData);

  itkFactorylessNewMacro(Self)
  itkCloneMacro(Self)

  virtual void UpdateOutputInformation();

  virtual void SetRequestedRegionToLargestPossibleRegion();

  virtual bool RequestedRegionIsOutsideOfTheBufferedRegion();

  virtual bool VerifyRequestedRegion();

  virtual void SetRequestedRegion( const itk::DataObject *data);

  virtual void CopyInformation(const itk::DataObject *data);

protected:
  GeometryData();

  virtual ~GeometryData();
};

} // namespace mitk
#endif /* MITKGEOMETRYDATA_H_HEADER_INCLUDED_C19C01E2 */
