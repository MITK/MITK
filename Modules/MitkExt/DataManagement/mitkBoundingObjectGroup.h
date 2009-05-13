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


#ifndef BOUNDINGOBJECTGROUP_H_HEADER_INCLUDED
#define BOUNDINGOBJECTGROUP_H_HEADER_INCLUDED

#include <mitkBoundingObject.h>

namespace mitk {

//##Documentation
//## @brief group object, that contains several mitk::BoundingObjects
//##
//## Calculates a bounding box that contains all sub-bounding boxes.
//## @ingroup Data
class MITKEXT_CORE_EXPORT BoundingObjectGroup : public mitk::BoundingObject
{
public:
  typedef itk::VectorContainer<unsigned int, mitk::BoundingObject::Pointer> BoundingObjectContainer;

  enum CSGMode
   {
      Union,
      Intersection,
      Difference
   };

  mitkClassMacro(BoundingObjectGroup, mitk::BoundingObject);// itk::VectorContainer<unsigned int ,mitk::BoundingObject::Pointer>);  
  itkNewMacro(Self);

  virtual void UpdateOutputInformation();
  virtual bool IsInside(const mitk::Point3D& p) const;

  itkSetObjectMacro(BoundingObjects, mitk::BoundingObjectGroup::BoundingObjectContainer);
  itkGetObjectMacro(BoundingObjects, mitk::BoundingObjectGroup::BoundingObjectContainer);

  itkSetMacro(CSGMode, mitk::BoundingObjectGroup::CSGMode);
  itkGetMacro(CSGMode, mitk::BoundingObjectGroup::CSGMode);

  void AddBoundingObject(mitk::BoundingObject::Pointer boundingObject);
  unsigned int GetCount() const;
  mitk::Geometry3D *  GetGeometry (int t=0) const;

  virtual bool VerifyRequestedRegion();

protected:
  BoundingObjectGroup();
  virtual ~BoundingObjectGroup();

  mitk::BoundingObjectGroup::BoundingObjectContainer::Pointer m_BoundingObjects;
  unsigned int m_Counter;
  CSGMode m_CSGMode;
};

}
#endif /* BOUNDINGOBJECTGROUP_H_HEADER_INCLUDED */
