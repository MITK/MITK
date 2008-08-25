/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/


#ifndef MITKGEOMETRY2DDATA_H_HEADER_INCLUDED_C19C01E2
#define MITKGEOMETRY2DDATA_H_HEADER_INCLUDED_C19C01E2

#include "mitkCommon.h"
#include "mitkBaseData.h"
#include "mitkGeometryData.h"
#include "mitkGeometry2D.h"

namespace mitk {

//##Documentation
//## @brief Data class containing Geometry2D objects
//## @ingroup Geometry
//##
class MITK_CORE_EXPORT Geometry2DData : public GeometryData
{
public:
  mitkClassMacro(Geometry2DData, GeometryData);

  itkNewMacro(Self);

  //##Documentation
  //## @brief Set the reference to a Geometry2D that is stored
  //## by the object
  //##
  //## @warning Accepts only instances of Geometry2D or sub-classes.
  virtual void SetGeometry(mitk::Geometry3D *geometry);

  //##Documentation
  //## @brief Set the reference to the Geometry2D that is stored
  //## by the object
  virtual void SetGeometry2D(mitk::Geometry2D* geometry2d);
  //##Documentation
  //## @brief Get the reference to the Geometry2D that is stored
  //## by the object
  virtual mitk::Geometry2D * GetGeometry2D() const
  {
    return static_cast<mitk::Geometry2D *>(GetGeometry());
  };

  virtual void UpdateOutputInformation();

  virtual void SetRequestedRegionToLargestPossibleRegion();

  virtual bool RequestedRegionIsOutsideOfTheBufferedRegion();

  virtual bool VerifyRequestedRegion();

  virtual void SetRequestedRegion(itk::DataObject *data);

  virtual void CopyInformation(const itk::DataObject *data);

protected:
  Geometry2DData();

  virtual ~Geometry2DData();
};

} // namespace mitk
#endif /* MITKGEOMETRY2DDATA_H_HEADER_INCLUDED_C19C01E2 */
