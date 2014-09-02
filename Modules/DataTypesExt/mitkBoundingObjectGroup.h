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
#ifndef BOUNDINGOBJECTGROUP_H_HEADER_INCLUDED
#define BOUNDINGOBJECTGROUP_H_HEADER_INCLUDED

#include <mitkBoundingObject.h>
#include "MitkDataTypesExtExports.h"
#include <deque>

namespace mitk {

  //##Documentation
  //## @brief group object, that contains several mitk::BoundingObjects
  //##
  //## Calculates a bounding box that contains all sub-bounding boxes.
  //## @ingroup Data
  class MitkDataTypesExt_EXPORT BoundingObjectGroup : public mitk::BoundingObject
  {
  public:

    enum CSGMode
    {
      Union,
      Intersection,
      Difference
    };

    mitkClassMacro(BoundingObjectGroup, mitk::BoundingObject);// itk::VectorContainer<unsigned int ,mitk::BoundingObject::Pointer>);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    virtual void UpdateOutputInformation();
    virtual bool IsInside(const mitk::Point3D& p) const;

    void SetBoundingObjects(const std::deque<mitk::BoundingObject::Pointer> boundingObjects);
    std::deque<mitk::BoundingObject::Pointer> GetBoundingObjects();

    itkSetMacro(CSGMode, mitk::BoundingObjectGroup::CSGMode);
    itkGetMacro(CSGMode, mitk::BoundingObjectGroup::CSGMode);

    void AddBoundingObject(mitk::BoundingObject::Pointer boundingObject);
    void RemoveBoundingObject(mitk::BoundingObject::Pointer boundingObject);
    unsigned int GetCount() const;
    mitk::BaseGeometry *  GetGeometry (int t=0) const;

    virtual bool VerifyRequestedRegion();

  protected:
    BoundingObjectGroup();
    virtual ~BoundingObjectGroup();

    std::deque<mitk::BoundingObject::Pointer> m_BoundingObjects;
    unsigned int m_Counter;
    CSGMode m_CSGMode;
  };

}
#endif /* BOUNDINGOBJECTGROUP_H_HEADER_INCLUDED */
