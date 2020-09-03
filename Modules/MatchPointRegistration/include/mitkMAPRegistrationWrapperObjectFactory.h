/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


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
    DEPRECATED(virtual std::string GetFileExtensions());
    /**
     * @deprecatedSince{2014_10} See mitk::FileWriterRegistry and QmitkIOUtil
     */
    DEPRECATED(virtual mitk::CoreObjectFactoryBase::MultimapType GetFileExtensionsMap());
    /**
     * @deprecatedSince{2014_10} See mitk::FileWriterRegistry and QmitkIOUtil
     */
    DEPRECATED(virtual std::string GetSaveFileExtensions());
    /**
     * @deprecatedSince{2014_10} See mitk::FileWriterRegistry and QmitkIOUtil
     */
    DEPRECATED(virtual mitk::CoreObjectFactoryBase::MultimapType GetSaveFileExtensionsMap());

    mitk::Mapper::Pointer CreateMapper(mitk::DataNode* node, MapperSlotId slotId) override;

  protected:
    MAPRegistrationWrapperObjectFactory();
};

}

#endif
