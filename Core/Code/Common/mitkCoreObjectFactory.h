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

#ifndef COREOBJECTFACTORY_H_INCLUDED
#define COREOBJECTFACTORY_H_INCLUDED

#include <set>

#include <MitkExports.h>
#include "mitkCoreObjectFactoryBase.h"
#include "mitkFileWriterWithInformation.h"
#include <mitkLegacyFileReaderService.h>
#include <mitkLegacyFileWriterService.h>
#include <mitkLegacyImageWriterService.h>

namespace mitk {

class Event;

class MITK_CORE_EXPORT CoreObjectFactory : public CoreObjectFactoryBase
{
  public:
    mitkClassMacro(CoreObjectFactory,CoreObjectFactoryBase);
    itkFactorylessNewMacro(CoreObjectFactory);
    virtual Mapper::Pointer CreateMapper(mitk::DataNode* node, MapperSlotId slotId);
    virtual void SetDefaultProperties(mitk::DataNode* node);
    virtual const char* GetFileExtensions();
    virtual MultimapType GetFileExtensionsMap();
    virtual const char* GetSaveFileExtensions();
    virtual MultimapType GetSaveFileExtensionsMap();
    virtual FileWriterList GetFileWriters();
    virtual void MapEvent(const mitk::Event* event, const int eventID);
    virtual void RegisterExtraFactory(CoreObjectFactoryBase* factory);
    virtual void UnRegisterExtraFactory(CoreObjectFactoryBase* factory);
    static Pointer GetInstance();
    void RegisterLegacyReaders(mitk::CoreObjectFactoryBase::Pointer factory);
    void RegisterLegacyWriters(/*mitk::CoreObjectFactoryBase::Pointer factory*/);
    std::string GetDescriptionForExtension(const std::string& extension);
  protected:
    CoreObjectFactory();
    void MergeFileExtensions(MultimapType& fileExtensionsMap, MultimapType inputMap);
    void CreateFileExtensionsMap();
    void CreateSaveFileExtensions();
    typedef std::set<mitk::CoreObjectFactoryBase::Pointer> ExtraFactoriesContainer;
    ExtraFactoriesContainer m_ExtraFactories;
    static FileWriterList m_FileWriters;
    std::string m_FileExtensions;
    MultimapType m_FileExtensionsMap;
    std::string m_SaveFileExtensions;
    MultimapType m_SaveFileExtensionsMap;
    static std::list< mitk::LegacyFileReaderService::Pointer > m_LegacyReaders;
    static std::list< mitk::LegacyFileWriterService::Pointer > m_LegacyWriters;
    static std::list< mitk::LegacyImageWriterService::Pointer > m_LegacyImageWriters;
};

} // namespace mitk

#endif
