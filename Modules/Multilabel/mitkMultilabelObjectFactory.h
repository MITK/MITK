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

#include <mitkAbstractFileIO.h>

namespace mitk
{
  class MultilabelObjectFactory : public CoreObjectFactoryBase
  {
  public:
    mitkClassMacro(MultilabelObjectFactory, CoreObjectFactoryBase);
    itkFactorylessNewMacro(Self) itkCloneMacro(Self) Mapper::Pointer
      CreateMapper(mitk::DataNode *node, MapperSlotId slotId) override;
    void SetDefaultProperties(mitk::DataNode *node) override;
    const char *GetFileExtensions() override;
    mitk::CoreObjectFactoryBase::MultimapType GetFileExtensionsMap() override;
    const char *GetSaveFileExtensions() override;
    mitk::CoreObjectFactoryBase::MultimapType GetSaveFileExtensionsMap() override;

  protected:
    MultilabelObjectFactory();
    ~MultilabelObjectFactory() override;
    void CreateFileExtensionsMap();
    MultimapType m_FileExtensionsMap;
    MultimapType m_SaveFileExtensionsMap;

  private:
    itk::ObjectFactoryBase::Pointer m_LabelSetImageIOFactory;
    itk::ObjectFactoryBase::Pointer m_LabelSetImageWriterFactory;

    std::vector<mitk::AbstractFileIO *> m_FileIOs;
  };
}

#endif
