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


#ifndef BOUNDINGOBJECT_H_HEADER_INCLUDED
#define BOUNDINGOBJECT_H_HEADER_INCLUDED

#include <mitkSurface.h>

namespace mitk {

//##Documentation
//## @brief superclass of all bounding objects (cylinder, cuboid,...)
//##
//## Manages generic functions and provides an interface for IsInside()
//## calculates a generic bounding box
//## @ingroup Data
class MITK_CORE_EXPORT BoundingObject : public mitk::Surface     //BaseData
{
public:
  mitkClassMacro(BoundingObject, mitk::Surface);  

  virtual bool IsInside(const mitk::Point3D& p) const=0;

  virtual mitk::ScalarType GetVolume();
  itkGetMacro(Positive, bool);
  itkSetMacro(Positive, bool);
  itkBooleanMacro(Positive);
  bool ReadXMLData( XMLReader& xmlReader );


  //##Documentation
  //## @brief Sets the Geometry3D of the bounding object to fit the given
  //## geometry.
  //## 
  //## The fit is done once, so if the given geometry changes it will 
  //## \em not effect the bounding object.
  virtual void FitGeometry(Geometry3D* aGeometry3D);
protected:
  BoundingObject();
  virtual ~BoundingObject();

  bool WriteXMLData( XMLWriter& xmlWriter );

  //##Documentation
  //## \brief If \a true, the Boundingobject describes a positive volume, 
  //## if \a false a negative volume.
  //## 
  bool m_Positive;
};

}
#endif /* BOUNDINGOBJECT_H_HEADER_INCLUDED */
