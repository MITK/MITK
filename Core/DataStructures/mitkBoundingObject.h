/*=========================================================================

  Program:   Medical Imaging & Interaction Toolkit
  Module:    $RCSfile$
  Language:  C++
  Date:      $Date$
  Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/ for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef BOUNDINGOBJECT_H_HEADER_INCLUDED
#define BOUNDINGOBJECT_H_HEADER_INCLUDED

#include <mitkSurface.h>

namespace mitk {

//##Documentation
//## @brief superclass of the different bounding objects (cylinder, cuboid,...)
//## Manages generic functions and provides an interface for IsInside()
//## calculates a generic bounding box
//## @ingroup Data
class BoundingObject : public mitk::Surface     //BaseData
{
public:
  mitkClassMacro(BoundingObject, mitk::Surface);  

  virtual void UpdateOutputInformation();// = 0;
  void SetRequestedRegionToLargestPossibleRegion();
  bool RequestedRegionIsOutsideOfTheBufferedRegion();
  virtual bool VerifyRequestedRegion();
  void SetRequestedRegion(itk::DataObject *data);

  virtual bool IsInside(mitk::Point3D p)=0;

  virtual mitk::ScalarType GetVolume();
  itkGetMacro(Positive, bool);
  itkSetMacro(Positive, bool);
  itkBooleanMacro(Positive);
protected:
  BoundingObject();
  virtual ~BoundingObject();
  bool m_Positive; // If true, the Boundingobject describes a positive volume, if false a negative volume
};

}
#endif /* BOUNDINGOBJECT_H_HEADER_INCLUDED */
