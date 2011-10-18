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

#ifndef mitkPlanePositionManager_h_Included
#define mitkPlanePositionManager_h_Included

#include "mitkCommon.h"
//#include "MitkExtExports.h"
#include "mitkRestorePlanePositionOperation.h"
#include "mitkDataStorage.h"
#include <mitkServiceReference.h>
#include <mitkModuleContext.h>
#include <mitkServiceInterface.h>

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
 class MITK_CORE_EXPORT PlanePositionManagerService : public itk::LightObject
 {

  public:

    /**
      \brief Adds a new plane position to the list. If this geometry is identical to one of the list nothing will be added

      \a plane THE FIRST! slice of the slice stack
      \a sliceIndex the slice number of the selected slice
      \return returns the ID i.e. the position in the positionlist. If the PlaneGeometry which is to be added already exists the existing
              ID will be returned.
    */
    unsigned int AddNewPlanePosition(const mitk::Geometry2D* plane, unsigned int sliceIndex = 0);

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

    friend class ::MitkCoreActivator;

 private:


   mitkClassMacro(PlanePositionManagerService, LightObject);

   itkFactorylessNewMacro(PlanePositionManagerService);

   PlanePositionManagerService();
   ~PlanePositionManagerService();

   // Disable copy constructor and assignment operator.
   PlanePositionManagerService(const PlanePositionManagerService&);
   PlanePositionManagerService& operator=(const PlanePositionManagerService&);

   static PlanePositionManagerService* m_Instance;
   std::vector<mitk::RestorePlanePositionOperation*> m_PositionList;
 };
}
MITK_DECLARE_SERVICE_INTERFACE(mitk::PlanePositionManagerService, "org.mitk.PlanePositionManagerService")

#endif
