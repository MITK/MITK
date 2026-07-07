/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkPlanePositionManager_h
#define mitkPlanePositionManager_h

#include <mitkCommon.h>
#include <mitkDataStorage.h>
#include <mitkRestorePlanePositionOperation.h>

#include <mitkPlaneGeometry.h>
#include <mitkServiceInterface.h>

class MitkCoreActivator;

namespace mitk
{
  /**
   * \brief Service that holds and manages a list of stored plane positions.
   *
   * To store a new position, specify the first slice of your slice stack and the
   * slice number you want to restore via AddNewPlanePosition().
   *
   * To restore a position, call GetPlanePosition(ID) where ID is the position
   * in the plane position list (returned by AddNewPlanePosition()). This returns a
   * mitk::RestorePlanePositionOperation which can be executed by the
   * SliceNavigationController of the slice stack.
   *
   * \sa RestorePlanePositionOperation
   * \sa SliceNavigationController
   */
  class MITKCORE_EXPORT PlanePositionManagerService
  {
  public:
    PlanePositionManagerService();
    ~PlanePositionManagerService();

    /**
     * \brief Add a new plane position to the list.
     *
     * If a geometrically identical plane position already exists in the list,
     * nothing is added and the existing ID is returned.
     *
     * \param plane The first slice of the slice stack (its PlaneGeometry).
     * \param sliceIndex The slice number of the selected slice.
     * \return The ID (position in the list) of the added or existing entry.
     */
    unsigned int AddNewPlanePosition(const mitk::PlaneGeometry *plane, unsigned int sliceIndex = 0);

    /**
     * \brief Remove the plane position at the given ID from the list.
     *
     * \param ID The plane ID to remove (its position in the list).
     * \return true if the plane was removed successfully, false if ID is invalid.
     */
    bool RemovePlanePosition(unsigned int ID);

    /** \brief Remove all plane positions from the list. */
    void RemoveAllPlanePositions();

    /**
     * \brief Get a specific plane position by its ID.
     *
     * \param ID The ID of the plane position to retrieve.
     * \return A RestorePlanePositionOperation that can be executed by the
     *         SliceNavigationController, or nullptr if the ID is invalid.
     */
    mitk::RestorePlanePositionOperation *GetPlanePosition(unsigned int ID);

    /**
     * \brief Get the number of stored plane positions.
     * \return The total count of plane positions in the list.
     */
    unsigned int GetNumberOfPlanePositions();

  private:
    // Disable copy constructor and assignment operator.
    PlanePositionManagerService(const PlanePositionManagerService &);
    PlanePositionManagerService &operator=(const PlanePositionManagerService &);

    std::vector<mitk::RestorePlanePositionOperation *> m_PositionList;
  };
}
MITK_DECLARE_SERVICE_INTERFACE(mitk::PlanePositionManagerService, "org.mitk.PlanePositionManagerService")

#endif
