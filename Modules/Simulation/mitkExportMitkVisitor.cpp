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

#include "mitkExportMitkVisitor.h"
#include <mitkNodePredicateDataType.h>
#include <mitkSimulation.h>
#include <mitkSurface.h>
#include <sofa/component/visualmodel/VisualModelImpl.h>
#include <vtkCellArray.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>

static void ApplyMaterial(mitk::DataNode::Pointer dataNode, const sofa::core::loader::Material& material)
{
  using sofa::defaulttype::Vec4f;

  if (dataNode.IsNull() || dynamic_cast<mitk::Surface*>(dataNode->GetData()) == NULL)
    return;

  if (material.useDiffuse)
    dataNode->SetFloatProperty("opacity", material.diffuse[3]);

  Vec4f ambient = material.useAmbient ? material.ambient : Vec4f();
  Vec4f diffuse = material.useDiffuse ? material.diffuse : Vec4f();
  Vec4f specular = material.useSpecular ? material.specular : Vec4f();
  float shininess = material.useShininess ? std::min(material.shininess, 128.0f) : 45.0f;

  if (shininess == 0.0f)
  {
    specular.clear();
    shininess = 1.0f;
  }

  dataNode->SetFloatProperty("material.ambientCoefficient", 1.0f);
  dataNode->SetProperty("material.ambientColor", mitk::ColorProperty::New(material.ambient.elems));

  dataNode->SetFloatProperty("material.diffuseCoefficient", 1.0f);
  dataNode->SetProperty("color", mitk::ColorProperty::New(material.diffuse.elems));

  dataNode->SetFloatProperty("material.specularCoefficient", 1.0f);
  dataNode->SetProperty("material.specularColor", mitk::ColorProperty::New(specular.elems));
  dataNode->SetFloatProperty("material.specularPower", shininess);
}

static mitk::DataNode::Pointer GetSimulationDataNode(mitk::DataStorage::Pointer dataStorage, sofa::core::objectmodel::BaseNode::SPtr rootNode)
{
  if (dataStorage.IsNull())
    return NULL;

  if (!rootNode)
    return NULL;

  mitk::TNodePredicateDataType<mitk::Simulation>::Pointer predicate = mitk::TNodePredicateDataType<mitk::Simulation>::New();
  mitk::DataStorage::SetOfObjects::ConstPointer subset = dataStorage->GetSubset(predicate);

  for (mitk::DataStorage::SetOfObjects::ConstIterator it = subset->Begin(); it != subset->End(); ++it)
  {
    mitk::DataNode::Pointer dataNode = it.Value();
    mitk::Simulation::Pointer simulation = static_cast<mitk::Simulation*>(dataNode->GetData());

    if (simulation->GetRootNode() == rootNode)
      return dataNode;
  }

  return NULL;
}


mitk::ExportMitkVisitor::ExportMitkVisitor(DataStorage::Pointer dataStorage, const sofa::core::ExecParams* params)
  : Visitor(params),
    m_DataStorage(dataStorage)
{
}

mitk::ExportMitkVisitor::ExportMitkVisitor(DataStorage::Pointer dataStorage, const std::string& visualModelName, const sofa::core::ExecParams* params)
  : Visitor(params),
    m_DataStorage(dataStorage),
    m_VisualModelName(visualModelName)
{
}

mitk::ExportMitkVisitor::~ExportMitkVisitor()
{
}

sofa::simulation::Visitor::Result mitk::ExportMitkVisitor::processNodeTopDown(sofa::simulation::Node* node)
{
  if (m_DataStorage.IsNotNull())
  {
    for_each(this, node, node->visualModel, &ExportMitkVisitor::processVisualModel);
    return RESULT_CONTINUE;
  }

  return RESULT_PRUNE;
}

void mitk::ExportMitkVisitor::processVisualModel(sofa::simulation::Node* node, sofa::core::visual::VisualModel* visualModel)
{
  using sofa::defaulttype::ResizableExtVector;
  typedef sofa::component::visualmodel::VisualModelImpl::VecCoord VecCoord;
  typedef sofa::component::visualmodel::VisualModelImpl::Triangle Triangle;
  typedef sofa::component::visualmodel::VisualModelImpl::Quad Quad;
  typedef sofa::component::visualmodel::VisualModelImpl::Deriv Deriv;
  typedef sofa::component::visualmodel::VisualModelImpl::VecTexCoord VecTexCoord;

  sofa::component::visualmodel::VisualModelImpl* visualModelImpl = dynamic_cast<sofa::component::visualmodel::VisualModelImpl*>(visualModel);

  if (visualModelImpl == NULL)
    return;

  if (!m_VisualModelName.empty() && m_VisualModelName != visualModelImpl->name.getValue())
    return;

  vtkSmartPointer<vtkPolyData> polyData = vtkSmartPointer<vtkPolyData>::New();
  vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();

  const VecCoord& vertices = visualModelImpl->m_vertices2.getValue().empty()
    ? visualModelImpl->m_positions.getValue()
    : visualModelImpl->m_vertices2.getValue();

  size_t numPoints = vertices.size();

  points->SetNumberOfPoints(numPoints);

  for (size_t i = 0; i < numPoints; ++i)
    points->SetPoint(i, vertices[i].elems);

  polyData->SetPoints(points);

  vtkSmartPointer<vtkCellArray> polys = vtkSmartPointer<vtkCellArray>::New();
  const ResizableExtVector<Triangle>& triangles = visualModelImpl->m_triangles.getValue();

  if (!triangles.empty())
  {
    ResizableExtVector<Triangle>::const_iterator trianglesEnd = triangles.end();

    for (ResizableExtVector<Triangle>::const_iterator it = triangles.begin(); it != trianglesEnd; ++it)
    {
      const Triangle& triangle = *it;

      polys->InsertNextCell(3);
      polys->InsertCellPoint(triangle[0]);
      polys->InsertCellPoint(triangle[1]);
      polys->InsertCellPoint(triangle[2]);
    }
  }

  const ResizableExtVector<Quad>& quads = visualModelImpl->m_quads.getValue();

  if (!quads.empty())
  {
    ResizableExtVector<Quad>::const_iterator quadsEnd = quads.end();

    for (ResizableExtVector<Quad>::const_iterator it = quads.begin(); it != quadsEnd; ++it)
    {
      const Quad& quad = *it;

      polys->InsertNextCell(4);
      polys->InsertCellPoint(quad[0]);
      polys->InsertCellPoint(quad[1]);
      polys->InsertCellPoint(quad[2]);
      polys->InsertCellPoint(quad[3]);
    }
  }

  polyData->SetPolys(polys);

  const ResizableExtVector<Deriv>& normals = visualModelImpl->m_vnormals.getValue();

  if (!normals.empty())
  {
    size_t numNormals = normals.size();

    vtkSmartPointer<vtkFloatArray> vtkNormals = vtkSmartPointer<vtkFloatArray>::New();
    vtkNormals->SetNumberOfComponents(3);
    vtkNormals->SetNumberOfTuples(numNormals);

    for (size_t i = 0; i < numNormals; ++i)
      vtkNormals->SetTuple(i, normals[i].elems);

    polyData->GetPointData()->SetNormals(vtkNormals);
  }

  const VecTexCoord& texCoords = visualModelImpl->m_vtexcoords.getValue();

  if (!texCoords.empty())
  {
    size_t numTexCoords = texCoords.size();

    vtkSmartPointer<vtkFloatArray> vtkTexCoords = vtkSmartPointer<vtkFloatArray>::New();
    vtkTexCoords->SetNumberOfComponents(2);
    vtkTexCoords->SetNumberOfTuples(numTexCoords);

    for (size_t i = 0; i < numTexCoords; ++i)
      vtkTexCoords->SetTuple(i, texCoords[i].elems);

    polyData->GetPointData()->SetTCoords(vtkTexCoords);
  }

  Surface::Pointer surface = Surface::New();
  surface->SetVtkPolyData(polyData);

  DataNode::Pointer dataNode = DataNode::New();
  dataNode->SetName(visualModelImpl->name.getValue());
  dataNode->SetData(surface);

  ApplyMaterial(dataNode, visualModelImpl->material.getValue());

  DataNode::Pointer parentDataNode = GetSimulationDataNode(m_DataStorage, node->getRoot());

  if (parentDataNode.IsNotNull())
    surface->SetGeometry(parentDataNode->GetData()->GetGeometry());

  m_DataStorage->Add(dataNode, parentDataNode);
}
