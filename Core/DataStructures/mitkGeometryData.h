/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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


#ifndef MITKGEOMETRYDATA_H_HEADER_INCLUDED_C19C01E2
#define MITKGEOMETRYDATA_H_HEADER_INCLUDED_C19C01E2

#include "mitkBaseData.h"
#include "mitkGeometry2DData.h"

namespace mitk {

//##Documentation
//## @brief Data class only having a Geometry3D but not containing 
//## any specific data.
//## @ingroup Geometry
//## 
//## Only implements pipeline methods which are abstract in BaseData.
class GeometryData : public Geometry2DData
{
public:
  mitkClassMacro(GeometryData, Geometry2DData);

  itkNewMacro(Self);

  virtual void UpdateOutputInformation();

  virtual void SetRequestedRegionToLargestPossibleRegion();

  virtual bool RequestedRegionIsOutsideOfTheBufferedRegion();

  virtual bool VerifyRequestedRegion();

  virtual void SetRequestedRegion(itk::DataObject *data);

  virtual void CopyInformation(const itk::DataObject *data);

protected:
  GeometryData();

  virtual ~GeometryData();
};

} // namespace mitk
#endif /* MITKGEOMETRYDATA_H_HEADER_INCLUDED_C19C01E2 */
