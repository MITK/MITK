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

#ifndef MULTILABELOBJECTFACTORY_H_INCLUDED
#define MULTILABELOBJECTFACTORY_H_INCLUDED

#include "mitkCoreObjectFactoryBase.h"
#include <MitkMultilabelExports.h>

#include <mitkAbstractFileIO.h>

namespace mitk {

  class MitkMultilabel_EXPORT MultilabelObjectFactory : public CoreObjectFactoryBase
  {
  public:
    mitkClassMacro(MultilabelObjectFactory, CoreObjectFactoryBase);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)
    virtual Mapper::Pointer CreateMapper(mitk::DataNode* node, MapperSlotId slotId);
    virtual void SetDefaultProperties(mitk::DataNode* node);
    virtual const char* GetFileExtensions();
    virtual mitk::CoreObjectFactoryBase::MultimapType GetFileExtensionsMap();
    virtual const char* GetSaveFileExtensions();
    virtual mitk::CoreObjectFactoryBase::MultimapType GetSaveFileExtensionsMap();

  protected:
    MultilabelObjectFactory();
    ~MultilabelObjectFactory();
    void CreateFileExtensionsMap();
    MultimapType m_FileExtensionsMap;
    MultimapType m_SaveFileExtensionsMap;

  private:
    itk::ObjectFactoryBase::Pointer m_LabelSetImageIOFactory;
    itk::ObjectFactoryBase::Pointer m_LabelSetImageWriterFactory;

    std::vector<mitk::AbstractFileIO*> m_FileIOs;
  };

}

#endif
