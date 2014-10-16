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

const std::vector<mitk::PlaneIntersectionVisitor::Intersection>& mitk::PlaneIntersectionVisitor::GetIntersections() const
{
  return m_Intersections;
}

sofa::simulation::Visitor::Result mitk::PlaneIntersectionVisitor::processNodeTopDown(sofa::simulation::Node* node)
{
  for_each(this, node, node->visualModel, &PlaneIntersectionVisitor::processVisualModel);
  return RESULT_CONTINUE;
}

void mitk::PlaneIntersectionVisitor::processVisualModel(sofa::simulation::Node*, sofa::core::visual::VisualModel* visualModel)
{
  using sofa::component::visualmodel::VisualModelImpl;
  using sofa::core::topology::Triangle;
  using sofa::core::topology::Quad;
  using sofa::defaulttype::ResizableExtVector;

  typedef VisualModelImpl::VecCoord VecCoord;

  VisualModelImpl* visualModelImpl = dynamic_cast<VisualModelImpl*>(visualModel);

  if (visualModelImpl == NULL)
    return;

  const VecCoord& verts = visualModelImpl->getVertices();
  const ResizableExtVector<Triangle>& tris = visualModelImpl->getTriangles();
  const ResizableExtVector<Quad>& quads = visualModelImpl->getQuads();

  float n[3] = { static_cast<float>(m_Normal[0]), static_cast<float>(m_Normal[1]), static_cast<float>(m_Normal[2]) };
  float p[3] = { static_cast<float>(m_Point[0]), static_cast<float>(m_Point[1]), static_cast<float>(m_Point[2]) };
  float d[3];
  size_t index;
  const float* t0;
  const float* t1;
  const float* t2;
  float s;
  Edge edge;
  Intersection intersection;

  const size_t numTriangles = tris.size();

  for (size_t i = 0; i < numTriangles; ++i)
  {
    t0 = verts[tris[i][0]].data();
    t1 = verts[tris[i][1]].data();
    t2 = verts[tris[i][2]].data();

    d[0] = n[0] * (p[0] - t0[0]) + n[1] * (p[1] - t0[1]) + n[2] * (p[2] - t0[2]);
    d[1] = n[0] * (p[0] - t1[0]) + n[1] * (p[1] - t1[1]) + n[2] * (p[2] - t1[2]);
    d[2] = n[0] * (p[0] - t2[0]) + n[1] * (p[1] - t2[1]) + n[2] * (p[2] - t2[2]);

    if (d[0] * d[1] < 0.0f)
    {
      index = d[0] * d[2] < 0.0f
        ? 0
        : 1;
    }
    else if (d[0] * d[2] < 0.0f)
    {
      index = 2;
    }
    else
    {
      continue;
    }

    t0 = verts[tris[i][index]].data();
    t1 = verts[tris[i][(index + 1) % 3]].data();
    t2 = verts[tris[i][(index + 2) % 3]].data();

    s = (n[0] * (p[0] - t0[0]) + n[1] * (p[1] - t0[1]) + n[2] * (p[2] - t0[2])) / (n[0] * (t1[0] - t0[0]) + n[1] * (t1[1] - t0[1]) + n[2] * (t1[2] - t0[2]));

    edge.v0[0] = static_cast<ScalarType>(t0[0] + s * (t1[0] - t0[0]));
    edge.v0[1] = static_cast<ScalarType>(t0[1] + s * (t1[1] - t0[1]));
    edge.v0[2] = static_cast<ScalarType>(t0[2] + s * (t1[2] - t0[2]));

    s = (n[0] * (p[0] - t0[0]) + n[1] * (p[1] - t0[1]) + n[2] * (p[2] - t0[2])) / (n[0] * (t2[0] - t0[0]) + n[1] * (t2[1] - t0[1]) + n[2] * (t2[2] - t0[2]));

    edge.v1[0] = static_cast<ScalarType>(t0[0] + s * (t2[0] - t0[0]));
    edge.v1[1] = static_cast<ScalarType>(t0[1] + s * (t2[1] - t0[1]));
    edge.v1[2] = static_cast<ScalarType>(t0[2] + s * (t2[2] - t0[2]));

    intersection.edges.push_back(edge);
  }

  if (!intersection.edges.empty())
  {
    const sofa::core::loader::Material& material = visualModelImpl->material.getValue();

    if (material.useDiffuse)
    {
      intersection.color[0] = static_cast<ScalarType>(material.diffuse[0]);
      intersection.color[1] = static_cast<ScalarType>(material.diffuse[1]);
      intersection.color[2] = static_cast<ScalarType>(material.diffuse[2]);
    }
    else
    {
      intersection.color[0] = 1.0f;
      intersection.color[1] = 1.0f;
      intersection.color[2] = 1.0f;
    }

    m_Intersections.push_back(intersection);
  }
}
