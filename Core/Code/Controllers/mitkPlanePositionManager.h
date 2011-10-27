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
#include "MitkExtExports.h"
#include "mitkRestorePlanePositionOperation.h"
#include "mitkDataStorage.h"

namespace mitk
{

 class MITK_CORE_EXPORT PlanePositionManager : public itk::Object
 {

  public:

    mitkClassMacro(PlanePositionManager, itk::Object);
    itkNewMacro(Self);

    static PlanePositionManager* GetInstance();

    unsigned int AddNewPlanePosition(const Geometry2D* plane, unsigned int sliceIndex = 0);

    bool RemovePlanePosition(unsigned int ID);

    void RemoveAllPlanePositions();

    RestorePlanePositionOperation* GetPlanePosition( unsigned int ID);

    unsigned int GetNumberOfPlanePositions();

    void DataStorageRemovedNode(const mitk::DataNode* removedNode = NULL);

    void SetDataStorage(mitk::DataStorage* ds);

 protected:

   PlanePositionManager();

   ~PlanePositionManager();

 private:

   static PlanePositionManager* m_Instance;
   std::vector<mitk::RestorePlanePositionOperation*> m_PositionList;
   unsigned int m_ID;
   DataStorage::Pointer m_DataStorage;

 };
}


#endif
