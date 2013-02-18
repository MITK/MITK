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
#include <sofa/core/visual/VisualParams.h>
#include <vtkCellArray.h>
#include <vtkFloatArray.h>
#include <vtkImageReader2.h>
#include <vtkImageReader2Factory.h>
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
  m_Actors.clear();
  m_Textures.clear();
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

  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
  points->SetNumberOfPoints(numVertices);

  for (unsigned int i = 0; i < numVertices; ++i)
    points->SetPoint(i, vertices[i].elems);

  vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
  polyData->SetPoints(points);

  vtkSmartPointer<vtkCellArray> polys = vtkSmartPointer<vtkCellArray>::New();

  int numTriangles = g.tri0 + g.nbt;
  int numQuads = g.quad0 + g.nbq;

  for (int i = g.tri0; i < numTriangles; ++i)
  {
    polys->InsertNextCell(3);
    polys->InsertCellPoint(triangles[i].elems[0]);
    polys->InsertCellPoint(triangles[i].elems[1]);
    polys->InsertCellPoint(triangles[i].elems[2]);
  }

  for (int i = g.quad0; i < numQuads; ++i)
  {
    polys->InsertNextCell(4);
    polys->InsertCellPoint(quads[i].elems[0]);
    polys->InsertCellPoint(quads[i].elems[1]);
    polys->InsertCellPoint(quads[i].elems[2]);
    polys->InsertCellPoint(quads[i].elems[3]);
  }

  polyData->SetPolys(polys);

  const sofa::defaulttype::ResizableExtVector<Coord>& normals = this->getVnormals();
  unsigned int numNormals = normals.size();

  vtkSmartPointer<vtkFloatArray> vtkNormals = vtkSmartPointer<vtkFloatArray>::New();
  vtkNormals->SetNumberOfComponents(3);
  vtkNormals->SetNumberOfTuples(numNormals);

  for (unsigned int i = 0; i < numNormals; ++i)
    vtkNormals->SetTuple(i, normals[i].elems);

  polyData->GetPointData()->SetNormals(vtkNormals);

  sofa::core::loader::Material m = g.materialId >= 0
    ? materials.getValue()[g.materialId]
    : material.getValue();

  if (m.useTexture && m.activated)
  {
    const sofa::defaulttype::ResizableExtVector<TexCoord>& texCoords = this->getVtexcoords();
    unsigned int numTexCoords = texCoords.size();

    vtkSmartPointer<vtkFloatArray> vtkTexCoords = vtkSmartPointer<vtkFloatArray>::New();
    vtkTexCoords->SetNumberOfComponents(2);
    vtkTexCoords->SetNumberOfTuples(numTexCoords);

    for (unsigned int i = 0; i < numTexCoords; ++i)
      vtkTexCoords->SetTuple(i, texCoords[i].elems);

    polyData->GetPointData()->SetTCoords(vtkTexCoords);
  }

  vtkSmartPointer<vtkPolyDataMapper> polyDataMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  polyDataMapper->SetInput(polyData);

  vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
  actor->SetMapper(polyDataMapper);
  actor->SetScale(Simulation::ScaleFactor);

  if (m.useTexture && m.activated)
    actor->SetTexture(m_Textures[g.materialId]);

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
  property->SetSpecular(1.0);
  property->SetSpecularColor(specular.x(), specular.y(), specular.z());
  property->SetSpecularPower(shininess);

  if (vparams->displayFlags().getShowWireFrame())
    property->SetRepresentationToWireframe();

  m_Actors.push_back(actor);

  if (!m_LastShowNormals)
   return;

  points = vtkSmartPointer<vtkPoints>::New();
  points->SetNumberOfPoints(numVertices * 2);

  vtkSmartPointer<vtkCellArray> lines = vtkSmartPointer<vtkCellArray>::New();

  for (unsigned int i = 0; i < numVertices; ++i)
  {
    unsigned int j = 2 * i;
    unsigned int k = j + 1;

    points->SetPoint(j, vertices[i].elems);
    points->SetPoint(k, (vertices[i] + normals[i]).elems);

    lines->InsertNextCell(2);
    lines->InsertCellPoint(j);
    lines->InsertCellPoint(k);
  }

  polyData = vtkSmartPointer<vtkPolyData>::New();
  polyData->SetPoints(points);
  polyData->SetLines(lines);

  polyDataMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  polyDataMapper->SetInput(polyData);

  actor = vtkSmartPointer<vtkActor>::New();
  actor->SetMapper(polyDataMapper);
  actor->SetScale(Simulation::ScaleFactor);

  m_Actors.push_back(actor);
}

std::vector<vtkSmartPointer<vtkActor> > mitk::SimulationModel::GetActors() const
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

  m_Actors.clear();

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

bool mitk::SimulationModel::loadTexture(const std::string& filename)
{
  return false;
}

bool mitk::SimulationModel::loadTextures()
{
  m_Textures.clear();

  std::vector<unsigned int> activatedTextures;

  const sofa::helper::vector<sofa::core::loader::Material>& materials = this->materials.getValue();
  unsigned int numMaterials = materials.size();

  for (unsigned int i = 0; i < numMaterials; ++i)
  {
    const sofa::core::loader::Material& material = materials[i];

    if (material.useTexture && material.activated)
      activatedTextures.push_back(i);
  }

  unsigned int numActivatedTextures = activatedTextures.size();

  for (unsigned int i = 0; i < numActivatedTextures; ++i)
  {
    std::string textureFilename = materials[activatedTextures[i]].textureFilename;

    if (!sofa::helper::system::DataRepository.findFile(textureFilename))
      return false;

    vtkSmartPointer<vtkImageReader2> imageReader;
    imageReader.TakeReference(vtkImageReader2Factory::CreateImageReader2(textureFilename.c_str()));

    if (imageReader == NULL)
      return false;

    imageReader->SetFileName(textureFilename.c_str());

    vtkSmartPointer<vtkTexture> texture = vtkSmartPointer<vtkTexture>::New();
    texture->SetInput(imageReader->GetOutputDataObject(0));
    texture->InterpolateOn();

    m_Textures.insert(std::make_pair(activatedTextures[i], texture));
  }

  return true;
}
