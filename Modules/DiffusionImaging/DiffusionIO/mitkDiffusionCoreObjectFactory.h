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

#ifndef DiffusionCoreObjectFactory_H_INCLUDED
#define DiffusionCoreObjectFactory_H_INCLUDED

#include "mitkCoreObjectFactory.h"

namespace mitk {

class DiffusionCoreObjectFactory : public CoreObjectFactoryBase
{
  public:
    mitkClassMacro(DiffusionCoreObjectFactory,CoreObjectFactoryBase)
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)

    ~DiffusionCoreObjectFactory();

    virtual Mapper::Pointer CreateMapper(mitk::DataNode* node, MapperSlotId slotId);

    virtual void SetDefaultProperties(mitk::DataNode* node);

    virtual const char* GetFileExtensions();

    virtual mitk::CoreObjectFactoryBase::MultimapType GetFileExtensionsMap();

    virtual const char* GetSaveFileExtensions();

    virtual mitk::CoreObjectFactoryBase::MultimapType GetSaveFileExtensionsMap();

protected:
    DiffusionCoreObjectFactory();
    void CreateFileExtensionsMap();
    MultimapType m_FileExtensionsMap;
    MultimapType m_SaveFileExtensionsMap;
private:

    std::string m_ExternalFileExtensions;
    std::string m_InternalFileExtensions;
    std::string m_SaveFileExtensions;

    itk::ObjectFactoryBase::Pointer m_NrrdDiffusionImageIOFactory;
    itk::ObjectFactoryBase::Pointer m_NrrdQBallImageIOFactory;

    itk::ObjectFactoryBase::Pointer m_NrrdDiffusionImageWriterFactory;
    itk::ObjectFactoryBase::Pointer m_NrrdQBallImageWriterFactory;
};

}

#endif

