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

#include <MitkCoreExports.h>
#include "mitkCoreObjectFactoryBase.h"
#include "mitkFileWriterWithInformation.h"
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

    ~CoreObjectFactory();

  protected:

    CoreObjectFactory();
    void MergeFileExtensions(MultimapType& fileExtensionsMap, MultimapType inputMap);
    void CreateFileExtensionsMap();
    void CreateSaveFileExtensions();
    typedef std::set<mitk::CoreObjectFactoryBase::Pointer> ExtraFactoriesContainer;

    ExtraFactoriesContainer m_ExtraFactories;
    std::string m_FileExtensions;
    MultimapType m_FileExtensionsMap;
    std::string m_SaveFileExtensions;
    MultimapType m_SaveFileExtensionsMap;

    itk::ObjectFactoryBase::Pointer m_PointSetIOFactory;
    itk::ObjectFactoryBase::Pointer m_STLFileIOFactory;
    itk::ObjectFactoryBase::Pointer m_VtkSurfaceIOFactory;
    itk::ObjectFactoryBase::Pointer m_VtkImageIOFactory;
    itk::ObjectFactoryBase::Pointer m_VtiFileIOFactory;
    itk::ObjectFactoryBase::Pointer m_ItkImageFileIOFactory;

    itk::ObjectFactoryBase::Pointer m_SurfaceVtkWriterFactory;
    itk::ObjectFactoryBase::Pointer m_PointSetWriterFactory;
    itk::ObjectFactoryBase::Pointer m_ImageWriterFactory;
};

} // namespace mitk

#endif
