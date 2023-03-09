/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#ifndef mitkRegEvaluationObjectFactory_h
#define mitkRegEvaluationObjectFactory_h

#include <mitkCoreObjectFactory.h>
#include "MitkMatchPointRegistrationExports.h"

namespace mitk {

  /** Factory that registers everything (the mapper) needed for handling
   RegEvaluationObject instances in MITK.*/
  class RegEvaluationObjectFactory : public mitk::CoreObjectFactoryBase
  {
  public:
    mitkClassMacro(RegEvaluationObjectFactory,CoreObjectFactoryBase);
    itkNewMacro(RegEvaluationObjectFactory);

    ~RegEvaluationObjectFactory() override;

    void SetDefaultProperties(mitk::DataNode* node) override;
    std::string GetFileExtensions() override;
    mitk::CoreObjectFactoryBase::MultimapType GetFileExtensionsMap() override;
    std::string GetSaveFileExtensions() override;
    mitk::CoreObjectFactoryBase::MultimapType GetSaveFileExtensionsMap() override;
    mitk::Mapper::Pointer CreateMapper(mitk::DataNode* node, MapperSlotId slotId) override;
    void RegisterIOFactories();
  protected:
    std::string m_FileExtensions;
    RegEvaluationObjectFactory();
  };

}

#endif
