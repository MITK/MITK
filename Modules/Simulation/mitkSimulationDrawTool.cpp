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
#include "mitkSimulationDrawTool.h"
#include <vtkActor.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkFloatArray.h>
#include <vtkLineSource.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkPolyDataNormals.h>
#include <vtkProperty.h>
#include <vtkSphereSource.h>
#include <vtkTubeFilter.h>

mitk::SimulationDrawTool::SimulationDrawTool()
  : m_PolygonMode(0),
    m_Wireframe(false),
    m_Update(true)
{
}

mitk::SimulationDrawTool::~SimulationDrawTool()
{
  this->DeleteVtkObjects();
}

void mitk::SimulationDrawTool::DeleteVtkObjects()
{
  for (std::vector<vtkObjectBase*>::const_iterator object = m_VtkObjects.begin(); object != m_VtkObjects.end(); ++object)
    (*object)->Delete();

  m_VtkObjects.clear();

  for (std::vector<vtkActor*>::const_iterator actor = m_Actors.begin(); actor != m_Actors.end(); ++actor)
    (*actor)->Delete();

  m_Actors.clear();
}

void mitk::SimulationDrawTool::DisableUpdate()
{
  m_Update = false;
}


std::vector<vtkActor*> mitk::SimulationDrawTool::GetActors() const
{
  return m_Actors;
}

void mitk::SimulationDrawTool::InitProperty(vtkProperty* property) const
{
  if (m_Wireframe)
    property->SetRepresentationToWireframe();
  else
    property->SetRepresentationToSurface();
}

void mitk::SimulationDrawTool::Reset()
{
  this->DeleteVtkObjects();
  m_Update = true;
}

void mitk::SimulationDrawTool::drawPoints(const std::vector<Vector3>& points, float pointSize, const Vec4f color)
{
  if (!m_Update || points.empty())
    return;

  unsigned int numPoints = points.size();

  vtkPoints* vtkPoints = vtkPoints::New();
  vtkPoints->SetNumberOfPoints(numPoints);

  vtkCellArray* cellArray = vtkCellArray::New();

  for (unsigned int i = 0; i < numPoints; ++i)
  {
    vtkPoints->SetPoint(i, points[i].elems);
    cellArray->InsertNextCell(1);
    cellArray->InsertCellPoint(i);
  }

  vtkPolyData* polyData = vtkPolyData::New();
  polyData->SetPoints(vtkPoints);
  polyData->SetVerts(cellArray);

  vtkPoints->Delete();
  cellArray->Delete();

  vtkPolyDataMapper* polyDataMapper = vtkPolyDataMapper::New();
  polyDataMapper->SetInput(polyData);

  vtkActor* actor = vtkActor::New();
  actor->SetMapper(polyDataMapper);
  actor->SetScale(Simulation::ScaleFactor);

  vtkProperty* property = actor->GetProperty();
  property->SetColor(color.x(), color.y(), color.z());
  property->SetPointSize(pointSize);

  m_VtkObjects.push_back(polyData);
  m_VtkObjects.push_back(polyDataMapper);
  m_Actors.push_back(actor);
}

void mitk::SimulationDrawTool::drawLines(const std::vector<Vector3>& points, float lineWidth, const Vec4f color)
{
  if (!m_Update || points.empty())
    return;

  unsigned int numPoints = points.size();
  std::vector<Vec2i> indices;

  for (unsigned int i = 0; i < numPoints; i += 2)
    indices.push_back(Vec2i(i, i + 1));

  this->drawLines(points, indices, lineWidth, color);
}

void mitk::SimulationDrawTool::drawLines(const std::vector<Vector3>& points, const std::vector<Vec2i>& indices, float lineWidth, const Vec4f color)
{
  if (!m_Update || points.empty())
    return;

  unsigned int numPoints = points.size();

  vtkPoints* vtkPoints = vtkPoints::New();
  vtkPoints->SetNumberOfPoints(numPoints);

  for (unsigned int i = 0; i < numPoints; ++i)
    vtkPoints->SetPoint(i, points[i].elems);

  vtkPolyData* polyData = vtkPolyData::New();
  polyData->SetPoints(vtkPoints);

  vtkPoints->Delete();

  vtkCellArray* lines = vtkCellArray::New();

  unsigned int numIndices = indices.size();

  for (unsigned int i = 0; i < numIndices; ++i)
  {
    lines->InsertNextCell(2);
    lines->InsertCellPoint(indices[i].elems[0]);
    lines->InsertCellPoint(indices[i].elems[1]);
  }

  polyData->SetLines(lines);

  lines->Delete();

  vtkPolyDataMapper* polyDataMapper = vtkPolyDataMapper::New();
  polyDataMapper->SetInput(polyData);

  vtkActor* actor = vtkActor::New();
  actor->SetMapper(polyDataMapper);
  actor->SetScale(Simulation::ScaleFactor);

  vtkProperty* property = actor->GetProperty();
  property->SetLineWidth(lineWidth);
  property->SetColor(color.x(), color.y(), color.z());

  m_VtkObjects.push_back(polyData);
  m_VtkObjects.push_back(polyDataMapper);
  m_Actors.push_back(actor);
}

void mitk::SimulationDrawTool::drawTriangles(const std::vector<Vector3>& points, const Vec4f color)
{
  if (!m_Update || points.empty())
    return;

  unsigned int numPoints = points.size();

  vtkPoints* vtkPoints = vtkPoints::New();
  vtkPoints->SetNumberOfPoints(numPoints);

  for (unsigned int i = 0; i < numPoints; ++i)
    vtkPoints->SetPoint(i, points[i].elems);

  vtkPolyData* polyData = vtkPolyData::New();
  polyData->SetPoints(vtkPoints);

  vtkPoints->Delete();

  vtkCellArray* triangles = vtkCellArray::New();

  for (unsigned int i = 0; i < points.size(); i += 3)
  {
    triangles->InsertNextCell(3);
    triangles->InsertCellPoint(i);
    triangles->InsertCellPoint(i + 1);
    triangles->InsertCellPoint(i + 2);
  }

  polyData->SetPolys(triangles);

  triangles->Delete();

  vtkPolyDataNormals* polyDataNormals = vtkPolyDataNormals::New();
  polyDataNormals->ComputeCellNormalsOff();
  polyDataNormals->SetInput(polyData);
  polyDataNormals->SplittingOff();

  vtkPolyDataMapper* polyDataMapper = vtkPolyDataMapper::New();
  polyDataMapper->SetInput(polyDataNormals->GetOutput());

  vtkActor* actor = vtkActor::New();
  actor->SetMapper(polyDataMapper);
  actor->SetScale(Simulation::ScaleFactor);

  vtkProperty* property = actor->GetProperty();

  this->InitProperty(property);

  property->SetColor(color.x(), color.y(), color.z());

  m_VtkObjects.push_back(polyData);
  m_VtkObjects.push_back(polyDataMapper);
  m_VtkObjects.push_back(polyDataNormals);
  m_Actors.push_back(actor);
}

void mitk::SimulationDrawTool::drawTriangles(const std::vector<Vector3>& points, const Vector3 normal, const Vec4f color)
{
  if (!m_Update || points.empty())
    return;

  unsigned int numPoints = points.size();

  vtkPoints* vtkPoints = vtkPoints::New();
  vtkPoints->SetNumberOfPoints(numPoints);

  for (unsigned int i = 0; i < numPoints; ++i)
    vtkPoints->SetPoint(i, points[i].elems);

  vtkPolyData* polyData = vtkPolyData::New();
  polyData->SetPoints(vtkPoints);

  vtkPoints->Delete();

  vtkCellArray* triangles = vtkCellArray::New();

  for (unsigned int i = 0; i < points.size(); i += 3)
  {
    triangles->InsertNextCell(3);
    triangles->InsertCellPoint(i);
    triangles->InsertCellPoint(i + 1);
    triangles->InsertCellPoint(i + 2);
  }

  polyData->SetPolys(triangles);

  triangles->Delete();

  vtkFloatArray* normals = vtkFloatArray::New();
  normals->SetNumberOfComponents(3);
  normals->SetName("Normals");

  for (int i = 0; i < numPoints; i += 3)
    normals->InsertNextTuple(normal.elems);

  polyData->GetCellData()->SetNormals(normals);

  normals->Delete();

  vtkPolyDataMapper* polyDataMapper = vtkPolyDataMapper::New();
  polyDataMapper->SetInput(polyData);

  vtkActor* actor = vtkActor::New();
  actor->SetMapper(polyDataMapper);
  actor->SetScale(Simulation::ScaleFactor);

  vtkProperty* property = actor->GetProperty();

  this->InitProperty(property);

  property->SetColor(color.x(), color.y(), color.z());

  m_VtkObjects.push_back(polyData);
  m_VtkObjects.push_back(polyDataMapper);
  m_Actors.push_back(actor);
}

void mitk::SimulationDrawTool::drawTriangles(const std::vector<Vector3>& points, const std::vector<Vec3i>& indices, const std::vector<Vector3>& normals, const Vec4f color)
{
  if (!m_Update || points.empty() || indices.empty() || normals.empty())
    return;

  unsigned int numPoints = points.size();

  vtkPoints* vtkPoints = vtkPoints::New();
  vtkPoints->SetNumberOfPoints(numPoints);

  for (unsigned int i = 0; i < numPoints; ++i)
    vtkPoints->SetPoint(i, points[i].elems);

  vtkPolyData* polyData = vtkPolyData::New();
  polyData->SetPoints(vtkPoints);

  vtkPoints->Delete();

  vtkCellArray* triangles = vtkCellArray::New();

  unsigned int numIndices = indices.size();

  for (unsigned int i = 0; i < numIndices; ++i)
  {
    triangles->InsertNextCell(3);
    triangles->InsertCellPoint(indices[i].elems[0]);
    triangles->InsertCellPoint(indices[i].elems[1]);
    triangles->InsertCellPoint(indices[i].elems[2]);
  }

  polyData->SetPolys(triangles);

  triangles->Delete();

  unsigned int numNormals = normals.size();

  vtkFloatArray* vtkNormals = vtkFloatArray::New();
  vtkNormals->SetNumberOfComponents(3);
  vtkNormals->SetName("Normals");

  for (int i = 0; i < numNormals; ++i)
    vtkNormals->InsertNextTuple(normals[i].elems);

  polyData->GetCellData()->SetNormals(vtkNormals);

  vtkNormals->Delete();

  vtkPolyDataMapper* polyDataMapper = vtkPolyDataMapper::New();
  polyDataMapper->SetInput(polyData);

  vtkActor* actor = vtkActor::New();
  actor->SetMapper(polyDataMapper);
  actor->SetScale(Simulation::ScaleFactor);

  vtkProperty* property = actor->GetProperty();

  this->InitProperty(property);

  property->SetColor(color.x(), color.y(), color.z());

  m_VtkObjects.push_back(polyData);
  m_VtkObjects.push_back(polyDataMapper);
  m_Actors.push_back(actor);
}

void mitk::SimulationDrawTool::drawTriangles(const std::vector<Vector3>&, const std::vector<Vector3>&, const std::vector<Vec4f>&)
{
}

void mitk::SimulationDrawTool::drawTriangleStrip(const std::vector<Vector3>&, const std::vector<Vector3>&, const Vec4f)
{
}

void mitk::SimulationDrawTool::drawTriangleFan(const std::vector<Vector3>&, const std::vector<Vector3>&, const Vec4f)
{
}

void mitk::SimulationDrawTool::drawFrame(const Vector3& position, const Quaternion& orientation, const Vec3f& size)
{
  if (!m_Update)
    return;

  const float radius = 0.05f;

  bool wireframeBackup = m_Wireframe;
  m_Wireframe = false;

  std::vector<Vec4f> colors;
  colors.push_back(Vec4f(0.0f, 0.0f, 1.0f, 1.0f));
  colors.push_back(Vec4f(0.0f, 1.0f, 0.0f, 1.0f));
  colors.push_back(Vec4f(1.0f, 0.0f, 0.0f, 1.0f));

  if (size.x() != 0.0f)
  {
    Vector3 point2 = position + orientation.rotate(Vec3f(size.x(), 0.0f, 0.0f));
    this->drawArrow(position, point2, radius, colors.back());
    colors.pop_back();
  }

  if (size.y() != 0.0f)
  {
    Vector3 point2 = position + orientation.rotate(Vec3f(0.0f, size.y(), 0.0f));
    this->drawArrow(position, point2, radius, colors.back());
    colors.pop_back();
  }

  if (size.z() != 0.0f)
  {
    Vector3 point2 = position + orientation.rotate(Vec3f(0.0f, 0.0f, size.z()));
    this->drawArrow(position, point2, radius, colors.back());
  }

  m_Wireframe = wireframeBackup;
}

void mitk::SimulationDrawTool::drawSpheres(const std::vector<Vector3>& points, const std::vector<float>& radii, const Vec4f color)
{
  if (!m_Update || points.empty())
    return;

  unsigned int numSpheres = points.size();

  for (unsigned int i = 0; i < numSpheres; ++i)
  {
    vtkSphereSource *sphereSource = vtkSphereSource::New();
    sphereSource->SetCenter(const_cast<double*>(points[i].elems));
    sphereSource->SetRadius(radii[i]);
    sphereSource->SetPhiResolution(16);
    sphereSource->SetThetaResolution(32);
    sphereSource->LatLongTessellationOn();

    vtkPolyDataMapper* polyDataMapper = vtkPolyDataMapper::New();
    polyDataMapper->SetInput(sphereSource->GetOutput());

    vtkActor* actor = vtkActor::New();
    actor->SetMapper(polyDataMapper);
    actor->SetScale(Simulation::ScaleFactor);

    vtkProperty* property = actor->GetProperty();

    this->InitProperty(property);

    property->SetColor(color.x(), color.y(), color.z());

    m_VtkObjects.push_back(sphereSource);
    m_VtkObjects.push_back(polyDataMapper);
    m_Actors.push_back(actor);
  }
}

void mitk::SimulationDrawTool::drawSpheres(const std::vector<Vector3>& points, float radius, const Vec4f color)
{
  if (!m_Update || points.empty())
    return;

  unsigned int numPoints = points.size();
  std::vector<float> radii(numPoints, radius);

  this->drawSpheres(points, radii, color);
}

void mitk::SimulationDrawTool::drawCone(const Vector3& point1, const Vector3& point2, float radius1, float radius2, const Vec4f color, int subdivisions)
{
  if (!m_Update)
    return;

  vtkPoints* points = vtkPoints::New();
  points->SetNumberOfPoints(2);
  points->SetPoint(0, point1.elems);
  points->SetPoint(1, point2.elems);

  vtkCellArray* line = vtkCellArray::New();
  line->InsertNextCell(2);
  line->InsertCellPoint(0);
  line->InsertCellPoint(1);

  vtkPolyData* polyData = vtkPolyData::New();
  polyData->SetPoints(points);
  polyData->SetLines(line);

  points->Delete();
  line->Delete();

  const char* radiiName = "Radii";

  vtkFloatArray* radii = vtkFloatArray::New();
  radii->SetName(radiiName);
  radii->SetNumberOfTuples(2);
  radii->SetTuple1(0, radius1);
  radii->SetTuple1(1, radius2);

  vtkPointData* pointData = polyData->GetPointData();
  pointData->AddArray(radii);
  pointData->SetActiveScalars(radiiName);

  radii->Delete();

  vtkTubeFilter* tubeFilter = vtkTubeFilter::New();
  tubeFilter->SetInput(polyData);
  tubeFilter->CappingOn();
  tubeFilter->SetNumberOfSides(subdivisions);
  tubeFilter->SetVaryRadiusToVaryRadiusByAbsoluteScalar();

  vtkPolyDataMapper* polyDataMapper = vtkPolyDataMapper::New();
  polyDataMapper->SetInput(tubeFilter->GetOutput());
  polyDataMapper->ScalarVisibilityOff();

  vtkActor* actor = vtkActor::New();
  actor->SetMapper(polyDataMapper);
  actor->SetScale(Simulation::ScaleFactor);

  vtkProperty* property = actor->GetProperty();

  this->InitProperty(property);

  property->SetColor(color.x(), color.y(), color.z());

  m_VtkObjects.push_back(polyData);
  m_VtkObjects.push_back(tubeFilter);
  m_VtkObjects.push_back(polyDataMapper);
  m_Actors.push_back(actor);
}

void mitk::SimulationDrawTool::drawCube(const float&, const Vec4f&, const int&)
{
}

void mitk::SimulationDrawTool::drawCylinder(const Vector3& point1, const Vector3& point2, float radius, const Vec4f color, int subdivisions)
{
  if (!m_Update)
    return;

  this->drawCone(point1, point2, radius, radius, color, subdivisions);
}

void mitk::SimulationDrawTool::drawArrow(const Vector3& point1, const Vector3& point2, float radius, const Vec4f color, int subdivisions)
{
  if (!m_Update)
    return;

  Vector3 point3 = point1 * 0.2f + point2 * 0.8f;

  this->drawCylinder(point1, point3, radius, color, subdivisions);
  this->drawCone(point3, point2, radius * 2.5f, 0.0f, color, subdivisions);
}

void mitk::SimulationDrawTool::drawPlus(const float& edgeRadius, const Vec4f& color, const int& subdivisions)
{
  if (!m_Update)
    return;

  this->drawCylinder(Vector3(-1.0, 0.0, 0.0), Vector3(1.0, 0.0, 0.0), edgeRadius, color, subdivisions);
  this->drawCylinder(Vector3(0.0, -1.0, 0.0), Vector3(0.0, 1.0, 0.0), edgeRadius, color, subdivisions);
  this->drawCylinder(Vector3(0.0, 0.0, -1.0), Vector3(0.0, 0.0, 1.0), edgeRadius, color, subdivisions);
}

void mitk::SimulationDrawTool::drawPoint(const Vector3&, const Vec4f&)
{
}

void mitk::SimulationDrawTool::drawPoint(const Vector3&, const Vector3&, const Vec4f&)
{
}

void mitk::SimulationDrawTool::drawTriangle(const Vector3&, const Vector3&, const Vector3&, const Vector3&, const Vec4f&)
{
}

void mitk::SimulationDrawTool::drawTriangle(const Vector3&, const Vector3&, const Vector3&, const Vector3&, const Vec4f&, const Vec4f&, const Vec4f&)
{
}

void mitk::SimulationDrawTool::drawSphere(const Vector3&, float)
{
}

void mitk::SimulationDrawTool::pushMatrix()
{
}

void mitk::SimulationDrawTool::popMatrix()
{
}

void mitk::SimulationDrawTool::multMatrix(float*)
{
}

void mitk::SimulationDrawTool::scale(float)
{
}

void mitk::SimulationDrawTool::setMaterial(const Vec4f&, std::string)
{
}

void mitk::SimulationDrawTool::resetMaterial(const Vec4f&, std::string)
{
}

void mitk::SimulationDrawTool::setPolygonMode(int mode, bool wireframe)
{
  if (!m_Update)
    return;

  m_PolygonMode = mode;
  m_Wireframe = wireframe;
}

void mitk::SimulationDrawTool::setLightingEnabled(bool)
{
}

void mitk::SimulationDrawTool::clear()
{
}
