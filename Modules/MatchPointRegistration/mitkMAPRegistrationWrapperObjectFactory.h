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


#ifndef __mitkMAPRegistrationWrapperObjectFactory_h
#define __mitkMAPRegistrationWrapperObjectFactory_h

#include <mitkCoreObjectFactory.h>
#include "MitkMatchPointRegistrationExports.h"

namespace mitk {

class MAPRegistrationWrapperObjectFactory : public mitk::CoreObjectFactoryBase
{
  public:
    mitkClassMacro(MAPRegistrationWrapperObjectFactory,CoreObjectFactoryBase);
    itkFactorylessNewMacro(Self);
    itkCloneMacro(Self);

    ~MAPRegistrationWrapperObjectFactory() override;

    void SetDefaultProperties(mitk::DataNode* node) override;

    /**
     * @deprecatedSince{2014_10} See mitk::FileWriterRegistry and QmitkIOUtil
     */
    DEPRECATED(virtual const char* GetFileExtensions());
    /**
     * @deprecatedSince{2014_10} See mitk::FileWriterRegistry and QmitkIOUtil
     */
    DEPRECATED(virtual mitk::CoreObjectFactoryBase::MultimapType GetFileExtensionsMap());
    /**
     * @deprecatedSince{2014_10} See mitk::FileWriterRegistry and QmitkIOUtil
     */
    DEPRECATED(virtual const char* GetSaveFileExtensions());
    /**
     * @deprecatedSince{2014_10} See mitk::FileWriterRegistry and QmitkIOUtil
     */
    DEPRECATED(virtual mitk::CoreObjectFactoryBase::MultimapType GetSaveFileExtensionsMap());

    mitk::Mapper::Pointer CreateMapper(mitk::DataNode* node, MapperSlotId slotId) override;

  protected:
    MAPRegistrationWrapperObjectFactory();
  private:
    void CreateFileExtensionsMap();
    std::string m_ExternalFileExtensions;
    std::string m_InternalFileExtensions;
    std::string m_SaveFileExtensions;
    MultimapType m_FileExtensionsMap;
    MultimapType m_SaveFileExtensionsMap;

    itk::ObjectFactoryBase::Pointer m_RegistrationWrapperIOFactory;
};

}

#endif
