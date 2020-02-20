/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkGizmoObjectFactory.h"

// Project includes
#include "mitkGizmo.h"
#include "mitkGizmoMapper2D.h"

// MITK includes
#include <mitkBaseRenderer.h>
#include <mitkDataNode.h>
#include <mitkProperties.h>

mitk::Mapper::Pointer mitk::GizmoObjectFactory::CreateMapper(mitk::DataNode *node, MapperSlotId id)
{
  mitk::Mapper::Pointer newMapper = nullptr;

  if ((dynamic_cast<mitk::Gizmo *>(node->GetData()) != nullptr))
  {
    if (id == mitk::BaseRenderer::Standard2D)
    {
      newMapper = mitk::GizmoMapper2D::New();
      newMapper->SetDataNode(node);
    }
  }

  return newMapper;
}

void mitk::GizmoObjectFactory::SetDefaultProperties(mitk::DataNode *node)
{
  if ((dynamic_cast<mitk::Gizmo *>(node->GetData()) != nullptr))
  {
    GizmoMapper2D::SetDefaultProperties(node);
  }
}

struct RegisterGizmoObjectFactory
{
  RegisterGizmoObjectFactory() : m_Factory(mitk::GizmoObjectFactory::New())
  {
    mitk::CoreObjectFactory::GetInstance()->RegisterExtraFactory(m_Factory);
  }
  ~RegisterGizmoObjectFactory() { mitk::CoreObjectFactory::GetInstance()->UnRegisterExtraFactory(m_Factory); }
  mitk::GizmoObjectFactory::Pointer m_Factory;
};

static RegisterGizmoObjectFactory registerGizmoObjectFactory;
