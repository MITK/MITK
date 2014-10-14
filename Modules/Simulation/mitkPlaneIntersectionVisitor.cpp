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

#include "mitkPlaneIntersectionVisitor.h"
#include <sofa/component/visualmodel/VisualModelImpl.h>

mitk::PlaneIntersectionVisitor::PlaneIntersectionVisitor(const Point3D& point, const Vector3D& normal, const sofa::core::ExecParams* params)
  : Visitor(params),
    m_Point(point),
    m_Normal(normal)
{
}

mitk::PlaneIntersectionVisitor::~PlaneIntersectionVisitor()
{
}

sofa::simulation::Visitor::Result mitk::PlaneIntersectionVisitor::processNodeTopDown(sofa::simulation::Node* node)
{
  for_each(this, node, node->visualModel, &PlaneIntersectionVisitor::processVisualModel);
  return RESULT_CONTINUE;
}

void mitk::PlaneIntersectionVisitor::processVisualModel(sofa::simulation::Node*, sofa::core::visual::VisualModel* visualModel)
{
  using sofa::component::visualmodel::VisualModelImpl;

  VisualModelImpl* visualModelImpl = dynamic_cast<VisualModelImpl*>(visualModel);
}
