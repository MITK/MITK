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

#include "mitkSimulation.h"
#include "mitkSimulationModel.h"
#include <sofa/core/visual/DisplayFlags.h>
#include <vtkActor.h>
#include <vtkCellArray.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>

mitk::SimulationModel::SimulationModel()
  : m_LastTime(-1.0),
    m_LastShowNormals(false),
    m_LastShowVisualModels(true),
    m_LastShowWireFrame(false)
{
}

mitk::SimulationModel::~SimulationModel()
{
  this->DeleteVtkObjects();
}

void mitk::SimulationModel::DeleteVtkObjects()
{
  for (std::vector<vtkObjectBase*>::const_iterator object = m_VtkObjects.begin(); object != m_VtkObjects.end(); ++object)
    (*object)->Delete();

  m_VtkObjects.clear();

  for (std::vector<vtkActor*>::const_iterator actor = m_Actors.begin(); actor != m_Actors.end(); ++actor)
    (*actor)->Delete();

  m_Actors.clear();
}

void mitk::SimulationModel::DrawGroup(int ig, const sofa::core::visual::VisualParams* vparams, bool transparent)
{
  const sofa::defaulttype::ResizableExtVector<Triangle>& triangles = this->getTriangles();
  const sofa::defaulttype::ResizableExtVector<Quad>& quads = this->getQuads();
  FaceGroup g;

  if (ig < 0)
  {
    g.materialId = -1;
    g.tri0 = 0;
    g.nbt = triangles.size();
    g.quad0 = 0;
    g.nbq = quads.size();
  }
  else
  {
    g = groups.getValue()[ig];
  }

  const sofa::defaulttype::ResizableExtVector<Coord>& vertices = this->getVertices();
  unsigned int numVertices = vertices.size();

  vtkPoints* points = vtkPoints::New();
  points->SetNumberOfPoints(numVertices);

  for (unsigned int i = 0; i < numVertices; ++i)
    points->SetPoint(i, vertices[i].elems);

  vtkPolyData* polyData = vtkPolyData::New();
  polyData->SetPoints(points);

  points->Delete();

  vtkCellArray* polys = vtkCellArray::New();

  int numTriangles = g.tri0 + g.nbt;
  int numQuads = g.quad0 + g.nbq;

  for (int i = g.tri0; i < numTriangles; ++i)
    polys->InsertNextCell(3, reinterpret_cast<const vtkIdType*>(triangles[i].elems));

  for (int i = g.quad0; i < numQuads; ++i)
    polys->InsertNextCell(4, reinterpret_cast<const vtkIdType*>(quads[i].elems));

  polyData->SetPolys(polys);

  polys->Delete();

  const sofa::defaulttype::ResizableExtVector<Coord>& normals = this->getVnormals();
  unsigned int numNormals = normals.size();

  vtkFloatArray* vtkNormals = vtkFloatArray::New();
  vtkNormals->SetNumberOfComponents(3);
  vtkNormals->SetName("Normals");

  for (unsigned int i = 0; i < numNormals; ++i)
    vtkNormals->InsertNextTuple(normals[i].elems);

  polyData->GetPointData()->SetNormals(vtkNormals);

  vtkNormals->Delete();

  vtkPolyDataMapper* polyDataMapper = vtkPolyDataMapper::New();
  polyDataMapper->SetInput(polyData);

  vtkActor* actor = vtkActor::New();
  actor->SetMapper(polyDataMapper);
  actor->SetScale(Simulation::ScaleFactor);

  sofa::core::loader::Material m = g.materialId >= 0
    ? materials.getValue()[g.materialId]
    : material.getValue();

  sofa::defaulttype::Vec4f ambient = !m.useAmbient
    ? sofa::defaulttype::Vec4f()
    : m.ambient;

  sofa::defaulttype::Vec4f diffuse = !m.useDiffuse
    ? sofa::defaulttype::Vec4f()
    : m.diffuse;

  sofa::defaulttype::Vec4f specular = !m.useSpecular
    ? sofa::defaulttype::Vec4f()
    : m.specular;

  float shininess = m.useShininess
    ? m.shininess
    : 45.0f;

  if (shininess == 0.0f)
  {
    specular.clear();
    shininess = 1.0f;
  }

  vtkProperty* property = actor->GetProperty();

  property->SetAmbientColor(ambient.x(), ambient.y(), ambient.z());
  property->SetDiffuseColor(diffuse.x(), diffuse.y(), diffuse.z());
  property->SetSpecularColor(specular.x(), specular.y(), specular.z());
  property->SetSpecularPower(shininess);

  if (vparams->displayFlags().getShowWireFrame())
    property->SetRepresentationToWireframe();

  m_VtkObjects.push_back(polyData);
  m_VtkObjects.push_back(polyDataMapper);
  m_Actors.push_back(actor);

  if (!m_LastShowNormals)
   return;

  points = vtkPoints::New();
  points->SetNumberOfPoints(numVertices * 2);

  vtkCellArray* lines = vtkCellArray::New();

  for (unsigned int i = 0; i < numVertices; ++i)
  {
    unsigned int j = 2 * i;
    unsigned int k = j + 1;

    points->SetPoint(j, vertices[i].elems);
    points->SetPoint(k, (vertices[i] + normals[i]).elems);

    vtkIdType line[] = { j, k };
    lines->InsertNextCell(2, line);
  }

  polyData = vtkPolyData::New();
  polyData->SetPoints(points);
  polyData->SetLines(lines);

  points->Delete();
  lines->Delete();

  polyDataMapper = vtkPolyDataMapper::New();
  polyDataMapper->SetInput(polyData);

  actor = vtkActor::New();
  actor->SetMapper(polyDataMapper);
  actor->SetScale(Simulation::ScaleFactor);

  m_VtkObjects.push_back(polyData);
  m_VtkObjects.push_back(polyDataMapper);
  m_Actors.push_back(actor);
}

std::vector<vtkActor*> mitk::SimulationModel::GetActors() const
{
  return m_Actors;
}

void mitk::SimulationModel::internalDraw(const sofa::core::visual::VisualParams* vparams, bool transparent)
{
  double time = this->getContext()->getTime();

  const sofa::core::visual::DisplayFlags& displayFlags = vparams->displayFlags();
  bool showNormals = displayFlags.getShowNormals();
  bool showVisualModels = displayFlags.getShowVisualModels();
  bool showWireFrame = displayFlags.getShowWireFrame();

  if (time == m_LastTime && showNormals == m_LastShowNormals && showVisualModels == m_LastShowVisualModels && showWireFrame == m_LastShowWireFrame)
    return;

  m_LastTime = time;
  m_LastShowNormals = showNormals;
  m_LastShowVisualModels = showVisualModels;
  m_LastShowWireFrame = showWireFrame;

  if (transparent)
    return;

  this->DeleteVtkObjects();

  if (!vparams->displayFlags().getShowVisualModels())
    return;

  sofa::helper::ReadAccessor<sofa::Data<sofa::helper::vector<FaceGroup> > > groups = this->groups;

  if (groups.empty())
  {
    this->DrawGroup(-1, vparams, transparent);
  }
  else
  {
    for (unsigned int i = 0; i < groups.size(); ++i)
      this->DrawGroup(i, vparams, transparent);
  }
}
