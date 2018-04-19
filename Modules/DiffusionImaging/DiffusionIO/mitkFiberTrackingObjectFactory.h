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

#include <mitkFiberBundle.h>
#include <mitkFiberBundleMapper3D.h>
#include <mitkFiberBundleMapper2D.h>
#include <mitkPeakImageMapper2D.h>
#include <mitkPeakImageMapper3D.h>

namespace mitk {

class FiberTrackingObjectFactory : public CoreObjectFactoryBase
{
  public:
    mitkClassMacro(FiberTrackingObjectFactory,CoreObjectFactoryBase)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    ~FiberTrackingObjectFactory() override;

    Mapper::Pointer CreateMapper(mitk::DataNode* node, MapperSlotId slotId) override;

    void SetDefaultProperties(mitk::DataNode* node) override;

    const char* GetFileExtensions() override;

    mitk::CoreObjectFactoryBase::MultimapType GetFileExtensionsMap() override;

    const char* GetSaveFileExtensions() override;

    mitk::CoreObjectFactoryBase::MultimapType GetSaveFileExtensionsMap() override;

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
