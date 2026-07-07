/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#ifndef mitkBoundingObjectGroup_h
#define mitkBoundingObjectGroup_h

#include <MitkDataTypesExtExports.h>
#include <deque>
#include <mitkBoundingObject.h>

namespace mitk
{
  /**
   * \brief Group object that contains several mitk::BoundingObject instances.
   *
   * Calculates a combined bounding box that encloses all sub-bounding objects.
   * Supports Constructive Solid Geometry (CSG) operations: Union,
   * Intersection, and Difference, which control how IsInside() evaluates
   * membership.
   *
   * Positive bounding objects are pushed to the front of the internal deque,
   * negative ones to the back.
   *
   * \sa BoundingObject, Cone, Cuboid, Cylinder, Ellipsoid
   * \ingroup Data
   */
  class MITKDATATYPESEXT_EXPORT BoundingObjectGroup : public mitk::BoundingObject
  {
  public:
    /**
     * \brief CSG combination mode for the group.
     */
    enum CSGMode
    {
      Union,        ///< A point is inside if it is inside at least one positive and no negative object.
      Intersection, ///< A point is inside only if it is inside every object.
      Difference    ///< Same logic as Union (positive minus negative volumes).
    };

    mitkClassMacro(BoundingObjectGroup, mitk::BoundingObject);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    /** \brief Recompute the combined bounding box from all sub-objects. */
    void UpdateOutputInformation() override;

    /**
     * \brief Test whether a point is inside the group according to the current CSGMode.
     *
     * \param[in] p The point in world coordinates.
     * \return true if the point satisfies the CSG condition.
     */
    bool IsInside(const mitk::Point3D &p) const override;

    /**
     * \brief Replace the internal collection of bounding objects.
     *
     * \param[in] boundingObjects The new deque of bounding objects.
     */
    void SetBoundingObjects(const std::deque<mitk::BoundingObject::Pointer> boundingObjects);

    /**
     * \brief Get a copy of the internal bounding object collection.
     *
     * \return A deque of BoundingObject smart pointers.
     */
    std::deque<mitk::BoundingObject::Pointer> GetBoundingObjects();

    /** \brief Set the CSG combination mode (Union, Intersection, or Difference). */
    itkSetMacro(CSGMode, mitk::BoundingObjectGroup::CSGMode);
    /** \brief Get the current CSG combination mode. */
    itkGetMacro(CSGMode, mitk::BoundingObjectGroup::CSGMode);

    /**
     * \brief Add a bounding object to the group.
     *
     * Positive objects are inserted at the front, negative objects at the back.
     * Triggers an update of the combined bounding box.
     *
     * \param[in] boundingObject The bounding object to add.
     */
    void AddBoundingObject(mitk::BoundingObject::Pointer boundingObject);

    /**
     * \brief Remove a bounding object from the group.
     *
     * Triggers an update of the combined bounding box.
     *
     * \param[in] boundingObject The bounding object to remove.
     */
    void RemoveBoundingObject(mitk::BoundingObject::Pointer boundingObject);

    /**
     * \brief Get the number of bounding objects in this group.
     *
     * \return The current count.
     */
    unsigned int GetCount() const;

    /**
     * \brief Get the geometry for the given time step.
     *
     * \param[in] t The time step (default is 0).
     * \return The geometry at the given time step.
     */
    mitk::BaseGeometry *GetGeometry(int t = 0) const;

    /**
     * \brief Verify that the requested region is valid.
     *
     * \return true if the group contains at least one bounding object.
     */
    bool VerifyRequestedRegion() override;

  protected:
    BoundingObjectGroup();
    ~BoundingObjectGroup() override;

    std::deque<mitk::BoundingObject::Pointer> m_BoundingObjects;
    unsigned int m_Counter;
    CSGMode m_CSGMode;
  };
}
#endif
