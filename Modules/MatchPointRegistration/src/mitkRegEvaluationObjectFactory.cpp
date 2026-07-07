/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkRegEvaluationObjectFactory.h>

#include <mitkProperties.h>
#include <mitkBaseRenderer.h>
#include <mitkDataNode.h>

#include <mitkRegEvaluationMapper2D.h>

mitk::RegEvaluationObjectFactory::RegEvaluationObjectFactory()
: CoreObjectFactoryBase()
{
}

mitk::RegEvaluationObjectFactory::~RegEvaluationObjectFactory()
{
}

mitk::Mapper::Pointer
mitk::RegEvaluationObjectFactory::
CreateMapper(mitk::DataNode* node, MapperSlotId slotId)
{
    mitk::Mapper::Pointer newMapper = nullptr;

    if ( slotId == mitk::BaseRenderer::Standard2D )
    {
        std::string classname("RegEvaluationObject");
        if(node->GetData() && classname.compare(node->GetData()->GetNameOfClass())==0)
        {
          newMapper = mitk::RegEvaluationMapper2D::New();
          newMapper->SetDataNode(node);
        }
    }

    return newMapper;
};

void mitk::RegEvaluationObjectFactory::SetDefaultProperties(mitk::DataNode*)
{

}

struct RegisterRegEvaluationObjectFactoryHelper{
  RegisterRegEvaluationObjectFactoryHelper()
    : m_Factory( mitk::RegEvaluationObjectFactory::New() )
  {
    mitk::CoreObjectFactory::GetInstance()->RegisterExtraFactory( m_Factory );
  }

  ~RegisterRegEvaluationObjectFactoryHelper()
  {
    mitk::CoreObjectFactory::GetInstance()->UnRegisterExtraFactory( m_Factory );
  }

  mitk::RegEvaluationObjectFactory::Pointer m_Factory;
};

static RegisterRegEvaluationObjectFactoryHelper registerMITKRegistrationWrapperIOFactoryHelper;
