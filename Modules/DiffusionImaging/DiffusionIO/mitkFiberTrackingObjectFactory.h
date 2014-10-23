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

#ifndef MITKFIBERTRACKINGOBJECTFACTORY_H
#define MITKFIBERTRACKINGOBJECTFACTORY_H

#include "mitkCoreObjectFactory.h"

//modernized fiberbundle datastrucutre
#include "mitkFiberBundleX.h"
#include "mitkFiberBundleXMapper3D.h"
#include "mitkFiberBundleXMapper2D.h"

//#include "mitkFiberBundleXThreadMonitorMapper3D.h"
//#include "mitkFiberBundleXThreadMonitor.h"

namespace mitk {

class FiberTrackingObjectFactory : public CoreObjectFactoryBase
{
  public:
    mitkClassMacro(FiberTrackingObjectFactory,CoreObjectFactoryBase)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    ~FiberTrackingObjectFactory();

    virtual Mapper::Pointer CreateMapper(mitk::DataNode* node, MapperSlotId slotId);

    virtual void SetDefaultProperties(mitk::DataNode* node);

    virtual const char* GetFileExtensions();

    virtual mitk::CoreObjectFactoryBase::MultimapType GetFileExtensionsMap();

    virtual const char* GetSaveFileExtensions();

    virtual mitk::CoreObjectFactoryBase::MultimapType GetSaveFileExtensionsMap();

    void RegisterIOFactories();

protected:
    FiberTrackingObjectFactory();
  private:
    void CreateFileExtensionsMap();
    std::string m_ExternalFileExtensions;
    std::string m_InternalFileExtensions;
    std::string m_SaveFileExtensions;
    MultimapType m_FileExtensionsMap;
    MultimapType m_SaveFileExtensionsMap;

};

}


#endif // MITKFIBERTRACKINGOBJECTFACTORY_H
