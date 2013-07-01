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
    m_Lighting(true),
    m_Update(true)
{
}

mitk::SimulationDrawTool::~SimulationDrawTool()
{
  m_Actors.clear();
}

void mitk::SimulationDrawTool::DisableUpdate()
{
  m_Update = false;
}

std::vector<vtkSmartPointer<vtkActor> > mitk::SimulationDrawTool::GetActors() const
{
  return m_Actors;
}

void mitk::SimulationDrawTool::InitProperty(vtkProperty* property) const
{
  if (m_Wireframe)
    property->SetRepresentationToWireframe();
  else
    property->SetRepresentationToSurface();

  property->SetLighting(m_Lighting);
}

void mitk::SimulationDrawTool::Reset()
{
  m_Actors.clear();
  m_Update = true;
}

void mitk::SimulationDrawTool::drawPoints(const std::vector<Vector3>& points, float pointSize, const Vec4f color)
{
  if (!m_Update || points.empty())
    return;

  unsigned int numPoints = points.size();

  vtkSmartPointer<vtkPoints> vtkPoints = vtkSmartPointer< ::vtkPoints>::New();
  vtkPoints->SetNumberOfPoints(numPoints);

  vtkSmartPointer<vtkCellArray> cellArray = vtkSmartPointer<vtkCellArray>::New();

  for (unsigned int i = 0; i < numPoints; ++i)
  {
    vtkPoints->SetPoint(i, points[i].elems);
    cellArray->InsertNextCell(1);
    cellArray->InsertCellPoint(i);
  }

  vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
  polyData->SetPoints(vtkPoints);
  polyData->SetVerts(cellArray);

  vtkSmartPointer<vtkPolyDataMapper> polyDataMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  polyDataMapper->SetInput(polyData);

  vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
  actor->SetMapper(polyDataMapper);
  actor->SetScale(Simulation::ScaleFactor);

  vtkProperty* property = actor->GetProperty();
  property->SetColor(color.x(), color.y(), color.z());
  property->SetPointSize(pointSize);

  m_Actors.push_back(actor);
}

void mitk::SimulationDrawTool::drawPoints(const std::vector<Vector3>&, float, const std::vector<Vec4f>)
{
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

  vtkSmartPointer<vtkPoints> vtkPoints = vtkSmartPointer< ::vtkPoints>::New();
  vtkPoints->SetNumberOfPoints(numPoints);

  for (unsigned int i = 0; i < numPoints; ++i)
    vtkPoints->SetPoint(i, points[i].elems);

  vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
  polyData->SetPoints(vtkPoints);

  vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();

  unsigned int numIndices = indices.size();

  for (unsigned int i = 0; i < numIndices; ++i)
  {
    lines->InsertNextCell(2);
    lines->InsertCellPoint(indices[i].elems[0]);
    lines->InsertCellPoint(indices[i].elems[1]);
  }

  polyData->SetLines(lines);

  vtkSmartPointer<vtkPolyDataMapper> polyDataMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  polyDataMapper->SetInput(polyData);

  vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
  actor->SetMapper(polyDataMapper);
  actor->SetScale(Simulation::ScaleFactor);

  vtkProperty* property = actor->GetProperty();
  property->SetLineWidth(lineWidth);
  property->SetColor(color.x(), color.y(), color.z());

  m_Actors.push_back(actor);
}

void mitk::SimulationDrawTool::drawTriangles(const std::vector<Vector3>& points, const Vec4f color)
{
  if (!m_Update || points.empty())
    return;

  unsigned int numPoints = points.size();

  vtkSmartPointer<vtkPoints> vtkPoints = vtkSmartPointer< ::vtkPoints>::New();
  vtkPoints->SetNumberOfPoints(numPoints);

  for (unsigned int i = 0; i < numPoints; ++i)
    vtkPoints->SetPoint(i, points[i].elems);

  vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
  polyData->SetPoints(vtkPoints);

  vtkSmartPointer<vtkCellArray> triangles = vtkSmartPointer<vtkCellArray>::New();

  for (unsigned int i = 0; i < points.size(); i += 3)
  {
    triangles->InsertNextCell(3);
    triangles->InsertCellPoint(i);
    triangles->InsertCellPoint(i + 1);
    triangles->InsertCellPoint(i + 2);
  }

  polyData->SetPolys(triangles);

  vtkSmartPointer<vtkPolyDataNormals> polyDataNormals = vtkSmartPointer<vtkPolyDataNormals>::New();
  polyDataNormals->ComputeCellNormalsOff();
  polyDataNormals->SetInput(polyData);
  polyDataNormals->SplittingOff();

  vtkSmartPointer<vtkPolyDataMapper> polyDataMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  polyDataMapper->SetInput(polyDataNormals->GetOutput());

  vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
  actor->SetMapper(polyDataMapper);
  actor->SetScale(Simulation::ScaleFactor);

  vtkProperty* property = actor->GetProperty();
  this->InitProperty(property);
  property->SetColor(color.x(), color.y(), color.z());

  m_Actors.push_back(actor);
}

void mitk::SimulationDrawTool::drawTriangles(const std::vector<Vector3>& points, const Vector3 normal, const Vec4f color)
{
  if (!m_Update || points.empty())
    return;

  unsigned int numPoints = points.size();
  unsigned int numNormals = numPoints / 3;

  vtkSmartPointer<vtkPoints> vtkPoints = vtkSmartPointer< ::vtkPoints>::New();
  vtkPoints->SetNumberOfPoints(numPoints);

  for (unsigned int i = 0; i < numPoints; ++i)
    vtkPoints->SetPoint(i, points[i].elems);

  vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
  polyData->SetPoints(vtkPoints);

  vtkSmartPointer<vtkCellArray> triangles = vtkSmartPointer<vtkCellArray>::New();

  for (unsigned int i = 0; i < points.size(); i += 3)
  {
    triangles->InsertNextCell(3);
    triangles->InsertCellPoint(i);
    triangles->InsertCellPoint(i + 1);
    triangles->InsertCellPoint(i + 2);
  }

  polyData->SetPolys(triangles);

  vtkSmartPointer<vtkFloatArray> normals = vtkSmartPointer<vtkFloatArray>::New();
  normals->SetNumberOfComponents(3);
  normals->SetNumberOfTuples(numNormals);
  normals->SetName("Normals");

  for (unsigned int i = 0; i < numNormals; ++i)
    normals->SetTuple(i, normal.elems);

  polyData->GetCellData()->SetNormals(normals);

  vtkSmartPointer<vtkPolyDataMapper> polyDataMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  polyDataMapper->SetInput(polyData);

  vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
  actor->SetMapper(polyDataMapper);
  actor->SetScale(Simulation::ScaleFactor);

  vtkProperty* property = actor->GetProperty();
  this->InitProperty(property);
  property->SetColor(color.x(), color.y(), color.z());

  m_Actors.push_back(actor);
}

void mitk::SimulationDrawTool::drawTriangles(const std::vector<Vector3>& points, const std::vector<Vec3i>& indices, const std::vector<Vector3>& normals, const Vec4f color)
{
  if (!m_Update || points.empty() || indices.empty() || normals.empty())
    return;

  unsigned int numPoints = points.size();

  vtkSmartPointer<vtkPoints> vtkPoints = vtkSmartPointer< ::vtkPoints>::New();
  vtkPoints->SetNumberOfPoints(numPoints);

  for (unsigned int i = 0; i < numPoints; ++i)
    vtkPoints->SetPoint(i, points[i].elems);

  vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
  polyData->SetPoints(vtkPoints);

  vtkSmartPointer<vtkCellArray> triangles = vtkSmartPointer<vtkCellArray>::New();

  unsigned int numIndices = indices.size();

  for (unsigned int i = 0; i < numIndices; ++i)
  {
    triangles->InsertNextCell(3);
    triangles->InsertCellPoint(indices[i].elems[0]);
    triangles->InsertCellPoint(indices[i].elems[1]);
    triangles->InsertCellPoint(indices[i].elems[2]);
  }

  polyData->SetPolys(triangles);

  unsigned int numNormals = normals.size();

  vtkSmartPointer<vtkFloatArray> vtkNormals = vtkSmartPointer<vtkFloatArray>::New();
  vtkNormals->SetNumberOfComponents(3);
  vtkNormals->SetNumberOfTuples(numNormals);
  vtkNormals->SetName("Normals");

  for (unsigned int i = 0; i < numNormals; ++i)
    vtkNormals->SetTuple(i, normals[i].elems);

  polyData->GetCellData()->SetNormals(vtkNormals);

  vtkSmartPointer<vtkPolyDataMapper> polyDataMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  polyDataMapper->SetInput(polyData);

  vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
  actor->SetMapper(polyDataMapper);
  actor->SetScale(Simulation::ScaleFactor);

  vtkProperty* property = actor->GetProperty();
  this->InitProperty(property);
  property->SetColor(color.x(), color.y(), color.z());

  m_Actors.push_back(actor);
}

void mitk::SimulationDrawTool::drawTriangles(const std::vector<Vector3>& points, const std::vector<Vector3>& normals, const std::vector<Vec4f>& colors)
{
  if (!m_Update || points.empty())
    return;

  unsigned int numPoints = points.size();
  unsigned int numNormals = numPoints / 3;
  bool computeNormals = numNormals != normals.size();

  vtkSmartPointer<vtkPoints> vtkPoints = vtkSmartPointer< ::vtkPoints>::New();
  vtkPoints->SetNumberOfPoints(numPoints);

  for (unsigned int i = 0; i < numPoints; ++i)
    vtkPoints->SetPoint(i, points[i].elems);

  vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
  polyData->SetPoints(vtkPoints);

  vtkSmartPointer<vtkCellArray> triangles = vtkSmartPointer<vtkCellArray>::New();

  for (unsigned int i = 0; i < points.size(); i += 3)
  {
    triangles->InsertNextCell(3);
    triangles->InsertCellPoint(i);
    triangles->InsertCellPoint(i + 1);
    triangles->InsertCellPoint(i + 2);
  }

  polyData->SetPolys(triangles);

  vtkSmartPointer<vtkPolyDataMapper> polyDataMapper = vtkSmartPointer<vtkPolyDataMapper>::New();

  if (!computeNormals)
  {
    vtkSmartPointer<vtkFloatArray> vtkNormals = vtkSmartPointer<vtkFloatArray>::New();
    vtkNormals->SetNumberOfComponents(3);
    vtkNormals->SetNumberOfTuples(numNormals);
    vtkNormals->SetName("Normals");

    for (unsigned int i = 0; i < numNormals; ++i)
      vtkNormals->SetTuple(i, normals[i].elems);

    polyData->GetCellData()->SetNormals(vtkNormals);

    polyDataMapper->SetInput(polyData);
  }
  else
  {
    vtkSmartPointer<vtkPolyDataNormals> polyDataNormals = vtkSmartPointer<vtkPolyDataNormals>::New();
    polyDataNormals->ComputeCellNormalsOff();
    polyDataNormals->SetInput(polyData);
    polyDataNormals->SplittingOff();

    polyDataMapper->SetInput(polyDataNormals->GetOutput());
  }

  vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
  actor->SetMapper(polyDataMapper);
  actor->SetScale(Simulation::ScaleFactor);

  vtkProperty* property = actor->GetProperty();
  this->InitProperty(property);
  property->SetColor(colors[0].x(), colors[0].y(), colors[0].z());

  m_Actors.push_back(actor);
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

  float minSize = std::min(std::min(size.x(), size.y()), size.z());
  float maxSize = std::max(std::max(size.x(), size.y()), size.z());

  if (maxSize > minSize * 2.0f)
  {
    if (minSize > 0.0f)
      maxSize = minSize * 2.0f;
    else
      minSize = maxSize * 0.707f;
  }

  const float radii[] = { minSize * 0.1f, maxSize * 0.2f };

  bool wireframeBackup = m_Wireframe;
  m_Wireframe = false;

  if (size.x() != 0.0f)
  {
    Vector3 point2 = position + orientation.rotate(Vec3f(size.x(), 0.0f, 0.0f));
    Vector3 point3 = point2 + orientation.rotate(Vec3f(radii[1], 0.0f, 0.0f));
    Vec4f red(1.0f, 0.0f, 0.0f, 1.0f);

    this->drawCylinder(position, point2, radii[0], red);
    this->drawCone(point2, point3, radii[1], 0.0f, red);
  }

  if (size.y() != 0.0f)
  {
    Vector3 point2 = position + orientation.rotate(Vec3f(0.0f, size.y(), 0.0f));
    Vector3 point3 = point2 + orientation.rotate(Vec3f(0.0f, radii[1], 0.0f));
    Vec4f green(0.0f, 1.0f, 0.0f, 1.0f);

    this->drawCylinder(position, point2, radii[0], green);
    this->drawCone(point2, point3, radii[1], 0.0f, green);
  }

  if (size.z() != 0.0f)
  {
    Vector3 point2 = position + orientation.rotate(Vec3f(0.0f, 0.0f, size.z()));
    Vector3 point3 = point2 + orientation.rotate(Vec3f(0.0f, 0.0f, radii[1]));
    Vec4f blue(0.0f, 0.0f, 1.0f, 1.0f);

    this->drawCylinder(position, point2, radii[0], blue);
    this->drawCone(point2, point3, radii[1], 0.0f, blue);
  }

  m_Wireframe = wireframeBackup;
}

void mitk::SimulationDrawTool::drawFrame(const Vector3&, const Quaternion&, const Vec3f&, const Vec4f&)
{
}

void mitk::SimulationDrawTool::drawSpheres(const std::vector<Vector3>& points, const std::vector<float>& radii, const Vec4f color)
{
  if (!m_Update || points.empty())
    return;

  unsigned int numSpheres = points.size();

  for (unsigned int i = 0; i < numSpheres; ++i)
  {
    vtkSmartPointer<vtkSphereSource> sphereSource = vtkSmartPointer<vtkSphereSource>::New();
    sphereSource->SetCenter(const_cast<double*>(points[i].elems));
    sphereSource->SetRadius(radii[i]);
    sphereSource->SetPhiResolution(16);
    sphereSource->SetThetaResolution(32);
    sphereSource->LatLongTessellationOn();

    vtkSmartPointer<vtkPolyDataMapper> polyDataMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
    polyDataMapper->SetInputConnection(sphereSource->GetOutputPort());

    vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
    actor->SetMapper(polyDataMapper);
    actor->SetScale(Simulation::ScaleFactor);

    vtkProperty* property = actor->GetProperty();
    this->InitProperty(property);
    property->SetColor(color.x(), color.y(), color.z());

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

  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  points->SetNumberOfPoints(2);
  points->SetPoint(0, point1.elems);
  points->SetPoint(1, point2.elems);

  vtkSmartPointer<vtkCellArray> line = vtkSmartPointer<vtkCellArray>::New();
  line->InsertNextCell(2);
  line->InsertCellPoint(0);
  line->InsertCellPoint(1);

  vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
  polyData->SetPoints(points);
  polyData->SetLines(line);

  const char* radiiName = "Radii";

  vtkSmartPointer<vtkFloatArray> radii = vtkSmartPointer<vtkFloatArray>::New();
  radii->SetName(radiiName);
  radii->SetNumberOfTuples(2);
  radii->SetTuple1(0, radius1);
  radii->SetTuple1(1, radius2);

  vtkPointData* pointData = polyData->GetPointData();
  pointData->AddArray(radii);
  pointData->SetActiveScalars(radiiName);

  vtkSmartPointer<vtkTubeFilter> tubeFilter = vtkSmartPointer<vtkTubeFilter>::New();
  tubeFilter->SetInput(polyData);
  tubeFilter->CappingOn();
  tubeFilter->SetNumberOfSides(subdivisions);
  tubeFilter->SetVaryRadiusToVaryRadiusByAbsoluteScalar();

  vtkSmartPointer<vtkPolyDataMapper> polyDataMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  polyDataMapper->SetInputConnection(tubeFilter->GetOutputPort());
  polyDataMapper->ScalarVisibilityOff();

  vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
  actor->SetMapper(polyDataMapper);
  actor->SetScale(Simulation::ScaleFactor);

  vtkProperty* property = actor->GetProperty();
  this->InitProperty(property);
  property->SetColor(color.x(), color.y(), color.z());

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

void mitk::SimulationDrawTool::drawCapsule(const Vector3&, const Vector3&, float, const Vec4f, int)
{
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

void mitk::SimulationDrawTool::drawTriangle(const Vector3&, const Vector3&, const Vector3&, const Vector3&)
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

void mitk::SimulationDrawTool::drawBoundingBox(const Vector3&, const Vector3&)
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

void mitk::SimulationDrawTool::setLightingEnabled(bool isEnabled)
{
  m_Lighting = isEnabled;
}

void mitk::SimulationDrawTool::writeOverlayText(int, int, unsigned int, const Vec4f&, const char*)
{
}
