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

namespace mitk {

  class MultilabelObjectFactory : public CoreObjectFactoryBase
  {
  public:
    mitkClassMacro(MultilabelObjectFactory, CoreObjectFactoryBase);
    itkFactorylessNewMacro(Self)
    itkCloneMacro(Self)
    virtual Mapper::Pointer CreateMapper(mitk::DataNode* node, MapperSlotId slotId) override;
    virtual void SetDefaultProperties(mitk::DataNode* node) override;
    virtual const char* GetFileExtensions() override;
    virtual mitk::CoreObjectFactoryBase::MultimapType GetFileExtensionsMap() override;
    virtual const char* GetSaveFileExtensions() override;
    virtual mitk::CoreObjectFactoryBase::MultimapType GetSaveFileExtensionsMap() override;

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
