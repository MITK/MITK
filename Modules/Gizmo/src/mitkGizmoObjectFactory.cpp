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

#include "mitkGizmoObjectFactory.h"

// Project includes
#include "mitkGizmo.h"
#include "mitkGizmoMapper2D.h"

// MITK includes
#include <mitkBaseRenderer.h>
#include <mitkDataNode.h>
#include <mitkProperties.h>

mitk::Mapper::Pointer mitk::GizmoObjectFactory::CreateMapper(mitk::DataNode* node, MapperSlotId id)
{
  mitk::Mapper::Pointer newMapper = NULL;

  if ( (dynamic_cast<mitk::Gizmo*>(node->GetData()) != NULL) )
  {
    if ( id == mitk::BaseRenderer::Standard2D )
    {
      newMapper = mitk::GizmoMapper2D::New();
      newMapper->SetDataNode(node);
    }
  }

  return newMapper;
}

void mitk::GizmoObjectFactory::SetDefaultProperties(mitk::DataNode* node)
{
  if ( (dynamic_cast<mitk::Gizmo*>(node->GetData()) != NULL) )
  {
    GizmoMapper2D::SetDefaultProperties(node);
  }
}

struct RegisterGizmoObjectFactory
{
  RegisterGizmoObjectFactory()
    : m_Factory( mitk::GizmoObjectFactory::New() )
  {
    mitk::CoreObjectFactory::GetInstance()->RegisterExtraFactory( m_Factory );
  }
  ~RegisterGizmoObjectFactory()
  {
    mitk::CoreObjectFactory::GetInstance()->UnRegisterExtraFactory( m_Factory );
  }
  mitk::GizmoObjectFactory::Pointer m_Factory;
};

static RegisterGizmoObjectFactory registerGizmoObjectFactory;
