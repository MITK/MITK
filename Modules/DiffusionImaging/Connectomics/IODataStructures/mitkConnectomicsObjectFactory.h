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


#ifndef MITKCONNECTOMICSOBJECTFACTORY_H
#define MITKCONNECTOMICSOBJECTFACTORY_H

#include "mitkCoreObjectFactory.h"
#include "ConnectomicsExports.h"

namespace mitk {

class Connectomics_EXPORT ConnectomicsObjectFactory : public CoreObjectFactoryBase
{
  public:
    mitkClassMacro(ConnectomicsObjectFactory,CoreObjectFactoryBase);
    itkNewMacro(ConnectomicsObjectFactory);

    virtual Mapper::Pointer CreateMapper(mitk::DataNode* node, MapperSlotId slotId);
    virtual void SetDefaultProperties(mitk::DataNode* node);
    virtual const char* GetFileExtensions();
    virtual mitk::CoreObjectFactoryBase::MultimapType GetFileExtensionsMap();
    virtual const char* GetSaveFileExtensions();
    virtual mitk::CoreObjectFactoryBase::MultimapType GetSaveFileExtensionsMap();
    void RegisterIOFactories();
  protected:
    ConnectomicsObjectFactory(bool registerSelf = true);
  private:
    void CreateFileExtensionsMap();
    std::string m_ExternalFileExtensions;
    std::string m_InternalFileExtensions;
    std::string m_SaveFileExtensions;
    MultimapType m_FileExtensionsMap;
    MultimapType m_SaveFileExtensionsMap;
};

}
// global declaration for simple call by
// applications
void Connectomics_EXPORT RegisterConnectomicsObjectFactory();

#endif
