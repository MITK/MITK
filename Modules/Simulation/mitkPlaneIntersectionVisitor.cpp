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

  if (visualModelImpl == nullptr)
    return;

  const sofa::core::loader::Material& material = visualModelImpl->material.getValue();

  if (!material.useDiffuse)
    return;

  const VecCoord& verts = visualModelImpl->getVertices();
  const ResizableExtVector<Triangle>& tris = visualModelImpl->getTriangles();
  const ResizableExtVector<Quad>& quads = visualModelImpl->getQuads();

  float n[3] = { static_cast<float>(m_Normal[0]), static_cast<float>(m_Normal[1]), static_cast<float>(m_Normal[2]) };
  float p[3] = { static_cast<float>(m_Point[0]), static_cast<float>(m_Point[1]), static_cast<float>(m_Point[2]) };
  float d[4];
  int j;
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
      j = d[0] * d[2] < 0.0f
        ? 0
        : 1;
    }
    else if (d[0] * d[2] < 0.0f)
    {
      j = 2;
    }
    else
    {
      continue;
    }

    t0 = verts[tris[i][j]].data();
    t1 = verts[tris[i][(j + 1) % 3]].data();
    t2 = verts[tris[i][(j + 2) % 3]].data();

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

  const float* q0;
  const float* q1;
  const float* q2;
  const float* q3;

  const size_t numQuads = quads.size();

  for (size_t i = 0; i < numQuads; ++i)
  {
    q0 = verts[quads[i][0]].data();
    q1 = verts[quads[i][1]].data();
    q2 = verts[quads[i][2]].data();
    q3 = verts[quads[i][3]].data();

    d[0] = n[0] * (p[0] - q0[0]) + n[1] * (p[1] - q0[1]) + n[2] * (p[2] - q0[2]);
    d[1] = n[0] * (p[0] - q1[0]) + n[1] * (p[1] - q1[1]) + n[2] * (p[2] - q1[2]);
    d[2] = n[0] * (p[0] - q2[0]) + n[1] * (p[1] - q2[1]) + n[2] * (p[2] - q2[2]);
    d[3] = n[0] * (p[0] - q3[0]) + n[1] * (p[1] - q3[1]) + n[2] * (p[2] - q3[2]);

    if (d[0] * d[2] < 0.0f)
    {
      if (d[0] * d[3] < 0.0f)
      {
        if (d[0] * d[1] < 0.0f)
        {
          q2 = q0;
        }
        else
        {
          const float* q = q0;
          q0 = q1;
          q1 = q2;
          q2 = q;
        }
      }
      else if (d[0] * d[1] >= 0.0f)
      {
        q0 = q1;
        q1 = q2;
      }
    }
    else if (d[1] * d[3] < 0.0f)
    {
      if (d[1] * d[0] < 0.0f)
      {
        q3 = q2;
        q2 = q1;
      }
      else
      {
        q1 = q3;
      }
    }
    else
    {
      continue;
    }

    s = (n[0] * (p[0] - q0[0]) + n[1] * (p[1] - q0[1]) + n[2] * (p[2] - q0[2])) / (n[0] * (q1[0] - q0[0]) + n[1] * (q1[1] - q0[1]) + n[2] * (q1[2] - q0[2]));

    edge.v0[0] = static_cast<ScalarType>(q0[0] + s * (q1[0] - q0[0]));
    edge.v0[1] = static_cast<ScalarType>(q0[1] + s * (q1[1] - q0[1]));
    edge.v0[2] = static_cast<ScalarType>(q0[2] + s * (q1[2] - q0[2]));

    s = (n[0] * (p[0] - q2[0]) + n[1] * (p[1] - q2[1]) + n[2] * (p[2] - q2[2])) / (n[0] * (q3[0] - q2[0]) + n[1] * (q3[1] - q2[1]) + n[2] * (q3[2] - q2[2]));

    edge.v1[0] = static_cast<ScalarType>(q2[0] + s * (q3[0] - q2[0]));
    edge.v1[1] = static_cast<ScalarType>(q2[1] + s * (q3[1] - q2[1]));
    edge.v1[2] = static_cast<ScalarType>(q2[2] + s * (q3[2] - q2[2]));

    intersection.edges.push_back(edge);
  }

  if (!intersection.edges.empty())
  {
    intersection.color[0] = material.diffuse[0];
    intersection.color[1] = material.diffuse[1];
    intersection.color[2] = material.diffuse[2];
    intersection.color[3] = material.diffuse[3];

    m_Intersections.push_back(intersection);
  }
}
