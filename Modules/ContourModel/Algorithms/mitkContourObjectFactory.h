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

#ifndef SEGMENTATIONOBJECTFACTORY_H_INCLUDED
#define SEGMENTATIONOBJECTFACTORY_H_INCLUDED

#include "mitkCoreObjectFactoryBase.h"
#include "ContourModelExports.h"

namespace mitk {

  class ContourModel_EXPORT ContourObjectFactory : public CoreObjectFactoryBase
  {
  public:
    mitkClassMacro(ContourObjectFactory,CoreObjectFactoryBase);
    itkNewMacro(ContourObjectFactory);
    virtual Mapper::Pointer CreateMapper(mitk::DataNode* node, MapperSlotId slotId);
    virtual void SetDefaultProperties(mitk::DataNode* node);
    virtual const char* GetFileExtensions();
    virtual mitk::CoreObjectFactoryBase::MultimapType GetFileExtensionsMap();
    virtual const char* GetSaveFileExtensions();
    virtual mitk::CoreObjectFactoryBase::MultimapType GetSaveFileExtensionsMap();
    void RegisterIOFactories();
  protected:
    ContourObjectFactory();
    void CreateFileExtensionsMap();
    MultimapType m_FileExtensionsMap;
    MultimapType m_SaveFileExtensionsMap;
  };

}

#endif
