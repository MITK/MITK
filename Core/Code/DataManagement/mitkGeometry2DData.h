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


#ifndef MITKGEOMETRY2DDATA_H_HEADER_INCLUDED_C19C01E2
#define MITKGEOMETRY2DDATA_H_HEADER_INCLUDED_C19C01E2

#include <MitkExports.h>
#include "mitkBaseData.h"
#include "mitkGeometryData.h"
#include "mitkPlaneGeometry.h"

namespace mitk {

//##Documentation
//## @brief Data class containing PlaneGeometry objects
//## @ingroup Geometry
//##
class MITK_CORE_EXPORT Geometry2DData : public GeometryData
{
public:
  mitkClassMacro(Geometry2DData, GeometryData);

  itkNewMacro(Self);

  //##Documentation
  //## @brief Set the reference to a PlaneGeometry that is stored
  //## by the object
  //##
  //## @warning Accepts only instances of PlaneGeometry or sub-classes.
  virtual void SetGeometry(mitk::BaseGeometry *geometry);

  //##Documentation
  //## @brief Set the reference to the PlaneGeometry that is stored
  //## by the object
  virtual void SetGeometry2D(mitk::PlaneGeometry* geometry2d);
  //##Documentation
  //## @brief Get the reference to the PlaneGeometry that is stored
  //## by the object
  virtual mitk::PlaneGeometry * GetGeometry2D() const
  {
    return static_cast<mitk::PlaneGeometry *>(GetGeometry());
  };

  virtual void UpdateOutputInformation();

  virtual void SetRequestedRegionToLargestPossibleRegion();

  virtual bool RequestedRegionIsOutsideOfTheBufferedRegion();

  virtual bool VerifyRequestedRegion();

  virtual void SetRequestedRegion( const itk::DataObject *data);

  virtual void CopyInformation(const itk::DataObject *data);

protected:
  Geometry2DData();

  virtual ~Geometry2DData();
};

} // namespace mitk
#endif /* MITKGEOMETRY2DDATA_H_HEADER_INCLUDED_C19C01E2 */
