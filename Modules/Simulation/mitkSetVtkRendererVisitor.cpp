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
    m_VtkRenderer(renderer),
    m_RenderingMode(VtkModel::Default)
{
}

mitk::SetVtkRendererVisitor::SetVtkRendererVisitor(vtkRenderer* renderer, const Point3D& planePoint, const Vector3D& planeNormal, ScalarType planeThickness, const sofa::core::ExecParams* params)
  : Visitor(params),
    m_RenderingMode(VtkModel::ClippingPlanes),
    m_PlanePoint(planePoint),
    m_PlaneNormal(planeNormal),
    m_PlaneThickness(planeThickness)
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

  if (vtkModel != NULL)
  {
    vtkModel->SetVtkRenderer(m_VtkRenderer);
    vtkModel->SetRenderingMode(m_RenderingMode);

    if (m_RenderingMode == VtkModel::ClippingPlanes)
      vtkModel->SetPlane(m_PlanePoint, m_PlaneNormal, m_PlaneThickness);
  }
}
