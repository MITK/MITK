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

#ifndef mitkPlanePositionManager_h_Included
#define mitkPlanePositionManager_h_Included

#include "mitkCommon.h"
#include "mitkRestorePlanePositionOperation.h"
#include "mitkDataStorage.h"

#include <mitkServiceInterface.h>
#include <mitkPlaneGeometry.h>

class MitkCoreActivator;

namespace mitk
{

/**
    The mitk::PlanePositionManagerService holds and manages a list of certain planepositions.
    To store a new position you need to specify the first slice of your slicestack and the
    slicenumber you want to restore in the mitk::PlanePositionManager::AddNewPlanePosition() function.

    To restore a position call mitk::PlanePositionManagerService::GetPlanePosition(ID) where ID is the position
    in the plane positionlist (returned by AddNewPlanePostion). This will give a mitk::RestorePlanePositionOperation
    which can be executed by the SliceNavigationController of the slicestack.

    \sa QmitkSegmentationView.cpp
  */
 class MITK_CORE_EXPORT PlanePositionManagerService
 {

  public:

   PlanePositionManagerService();
   ~PlanePositionManagerService();

    /**
      \brief Adds a new plane position to the list. If this geometry is identical to one of the list nothing will be added

      \a plane THE FIRST! slice of the slice stack
      \a sliceIndex the slice number of the selected slice
      \return returns the ID i.e. the position in the positionlist. If the PlaneGeometry which is to be added already exists the existing
              ID will be returned.
    */
    unsigned int AddNewPlanePosition(const mitk::PlaneGeometry* plane, unsigned int sliceIndex = 0);

    /**
      \brief Removes the plane at the position \a ID from the list.

      \a ID the plane ID which should be removed, i.e. its position in the list
      \return true if the plane was removed successfully and false if it is an invalid ID
    */
    bool RemovePlanePosition(unsigned int ID);

    /// \brief Clears the complete positionlist
    void RemoveAllPlanePositions();

    /**
      \brief Getter for a specific plane position with a given ID

      \a ID the ID of the plane position
      \return Returns a RestorePlanePositionOperation which can be executed by th SliceNavigationController or NULL for an invalid ID
    */
    mitk::RestorePlanePositionOperation* GetPlanePosition( unsigned int ID);

    /// \brief Getting the number of all stored planes
    unsigned int GetNumberOfPlanePositions();

 private:

   // Disable copy constructor and assignment operator.
   PlanePositionManagerService(const PlanePositionManagerService&);
   PlanePositionManagerService& operator=(const PlanePositionManagerService&);

   std::vector<mitk::RestorePlanePositionOperation*> m_PositionList;
 };
}
MITK_DECLARE_SERVICE_INTERFACE(mitk::PlanePositionManagerService, "org.mitk.PlanePositionManagerService")

#endif
