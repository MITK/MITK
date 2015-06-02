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

#include "mitkSetVtkRendererVisitor.h"
#include "mitkVtkModel.h"

mitk::SetVtkRendererVisitor::SetVtkRendererVisitor(vtkRenderer* renderer, const sofa::core::ExecParams* params)
  : Visitor(params),
    m_VtkRenderer(renderer)
{
}

mitk::SetVtkRendererVisitor::~SetVtkRendererVisitor()
{
}

sofa::simulation::Visitor::Result mitk::SetVtkRendererVisitor::processNodeTopDown(sofa::simulation::Node* node)
{
  for_each(this, node, node->visualModel, &SetVtkRendererVisitor::processVisualModel);
  return RESULT_CONTINUE;
}

void mitk::SetVtkRendererVisitor::processVisualModel(sofa::simulation::Node*, sofa::core::visual::VisualModel* visualModel)
{
  VtkModel* vtkModel = dynamic_cast<VtkModel*>(visualModel);

  if (vtkModel != nullptr)
    vtkModel->SetVtkRenderer(m_VtkRenderer);
}
