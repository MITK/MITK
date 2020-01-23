/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef BOUNDINGOBJECTGROUP_H_HEADER_INCLUDED
#define BOUNDINGOBJECTGROUP_H_HEADER_INCLUDED

#include "MitkDataTypesExtExports.h"
#include <deque>
#include <mitkBoundingObject.h>

namespace mitk
{
  //##Documentation
  //## @brief group object, that contains several mitk::BoundingObjects
  //##
  //## Calculates a bounding box that contains all sub-bounding boxes.
  //## @ingroup Data
  class MITKDATATYPESEXT_EXPORT BoundingObjectGroup : public mitk::BoundingObject
  {
  public:
    enum CSGMode
    {
      Union,
      Intersection,
      Difference
    };

    mitkClassMacro(BoundingObjectGroup,
                   mitk::BoundingObject); // itk::VectorContainer<unsigned int ,mitk::BoundingObject::Pointer>);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

      void UpdateOutputInformation() override;
    bool IsInside(const mitk::Point3D &p) const override;

    void SetBoundingObjects(const std::deque<mitk::BoundingObject::Pointer> boundingObjects);
    std::deque<mitk::BoundingObject::Pointer> GetBoundingObjects();

    itkSetMacro(CSGMode, mitk::BoundingObjectGroup::CSGMode);
    itkGetMacro(CSGMode, mitk::BoundingObjectGroup::CSGMode);

    void AddBoundingObject(mitk::BoundingObject::Pointer boundingObject);
    void RemoveBoundingObject(mitk::BoundingObject::Pointer boundingObject);
    unsigned int GetCount() const;
    mitk::BaseGeometry *GetGeometry(int t = 0) const;

    bool VerifyRequestedRegion() override;

  protected:
    BoundingObjectGroup();
    ~BoundingObjectGroup() override;

    std::deque<mitk::BoundingObject::Pointer> m_BoundingObjects;
    unsigned int m_Counter;
    CSGMode m_CSGMode;
  };
}
#endif /* BOUNDINGOBJECTGROUP_H_HEADER_INCLUDED */
