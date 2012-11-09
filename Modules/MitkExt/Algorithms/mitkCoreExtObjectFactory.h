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

#ifndef COREEXTOBJECTFACTORY_H_INCLUDED
#define COREEXTOBJECTFACTORY_H_INCLUDED

#include "mitkCoreObjectFactoryBase.h"
#include "MitkExtExports.h"

namespace mitk {

class MitkExt_EXPORT CoreExtObjectFactory : public CoreObjectFactoryBase
{
  public:
    mitkClassMacro(CoreExtObjectFactory,CoreObjectFactoryBase);
    itkNewMacro(CoreExtObjectFactory);
    virtual Mapper::Pointer CreateMapper(mitk::DataNode* node, MapperSlotId slotId);
    virtual void SetDefaultProperties(mitk::DataNode* node);
    virtual const char* GetFileExtensions();
    virtual mitk::CoreObjectFactoryBase::MultimapType GetFileExtensionsMap();
    virtual const char* GetSaveFileExtensions();
    virtual mitk::CoreObjectFactoryBase::MultimapType GetSaveFileExtensionsMap();
    void RegisterIOFactories();
  protected:
    CoreExtObjectFactory();
    void CreateFileExtensionsMap();
    MultimapType m_FileExtensionsMap;
    MultimapType m_SaveFileExtensionsMap;
};

}
// global declaration for simple call by
// applications
void MitkExt_EXPORT RegisterCoreExtObjectFactory();

#endif

