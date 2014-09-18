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

#include "internal/org_mitk_simulation_Activator.h"
#include "mitkMeshMitkLoader.h"
#include <mitkDataStorage.h>
#include <mitkIDataStorageService.h>
#include <mitkNodePredicateDataType.h>
#include <mitkSurface.h>
#include <vtkCellArray.h>
#include <vtkLinearTransform.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkTransformPolyDataFilter.h>

static mitk::DataStorage::Pointer GetDataStorage()
{
  mitk::IDataStorageService* dataStorageService = mitk::org_mitk_simulation_Activator::GetService<mitk::IDataStorageService>();

  if (dataStorageService != NULL)
  {
    mitk::IDataStorageReference::Pointer dataStorageReference = dataStorageService->GetDefaultDataStorage();

    if (dataStorageReference.IsNotNull())
      return dataStorageReference->GetDataStorage();
  }

  return NULL;
}

template <class T>
typename T::Pointer GetData(const std::string& name)
{
  mitk::DataStorage::Pointer dataStorage = GetDataStorage();

  if (dataStorage.IsNull())
    return NULL;

  typename mitk::TNodePredicateDataType<T>::Pointer predicate = mitk::TNodePredicateDataType<T>::New();
  mitk::DataStorage::SetOfObjects::ConstPointer subset = dataStorage->GetSubset(predicate);

  for (mitk::DataStorage::SetOfObjects::ConstIterator it = subset->Begin(); it != subset->End(); ++it)
  {
    mitk::DataNode::Pointer dataNode = it.Value();

    if (dataNode->GetName() == name)
    {
      typename T::Pointer data = static_cast<T*>(dataNode->GetData());

      if (data.IsNotNull())
        return data;
    }
  }

  return NULL;
}

mitk::MeshMitkLoader::MeshMitkLoader()
{
  this->addAlias(&m_filename, "dataNode");
  this->addAlias(&m_filename, "surface");
}

mitk::MeshMitkLoader::~MeshMitkLoader()
{
}

bool mitk::MeshMitkLoader::canLoad()
{
  Surface::Pointer surface = GetData<Surface>(m_filename.getValue());

  if (surface.IsNull())
    return false;

  vtkPolyData* polyData = surface->GetVtkPolyData();

  if (polyData == NULL || polyData->GetNumberOfCells() == 0)
    return false;

  return true;
}

bool mitk::MeshMitkLoader::load()
{
  Surface::Pointer surface = GetData<Surface>(m_filename.getValue());
  vtkPolyData* polyData = surface->GetVtkPolyData();

  // vtkSmartPointer<vtkTransformPolyDataFilter> transformFilter = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  // transformFilter->SetTransform(surface->GetGeometry()->GetVtkTransform());
  // transformFilter->SetInputConnection(polyData->GetProducerPort());
  // transformFilter->Update();

  // polyData = vtkPolyData::SafeDownCast(transformFilter->GetOutputDataObject(0));

  sofa::helper::vector<sofa::defaulttype::Vector3>& positions = *this->positions.beginEdit();
  sofa::helper::vector<Edge>& edges = *this->edges.beginEdit();
  sofa::helper::vector<Triangle>& triangles = *this->triangles.beginEdit();
  sofa::helper::vector<Quad>& quads = *this->quads.beginEdit();
  sofa::helper::vector<sofa::helper::vector<unsigned int> > polygons = *this->polygons.beginEdit();

  vtkPoints* points = polyData->GetPoints();
  vtkIdType numPoints = points->GetNumberOfPoints();
  double point[3];

  for (vtkIdType i = 0; i < numPoints; ++i)
  {
    points->GetPoint(i, point);
    positions.push_back(sofa::defaulttype::Vec3d(point[0], point[1], point[2]));
  }

  vtkCellArray* polys = polyData->GetPolys();
  vtkIdType numPolys = polys->GetNumberOfCells();
  vtkSmartPointer<vtkIdList> poly = vtkSmartPointer<vtkIdList>::New();
  Edge edge;
  Triangle triangle;
  Quad quad;

  polys->InitTraversal();

  while (polys->GetNextCell(poly) != 0)
  {
    switch (poly->GetNumberOfIds())
    {
    case 1:
      break;

    case 2:
      edge[0] = poly->GetId(0);
      edge[1] = poly->GetId(1);

      edges.push_back(edge);
      break;

    case 3:
      triangle[0] = poly->GetId(0);
      triangle[1] = poly->GetId(1);
      triangle[2] = poly->GetId(2);

      triangles.push_back(triangle);
      break;

    case 4:
      quad[0] = poly->GetId(0);
      quad[1] = poly->GetId(1);
      quad[2] = poly->GetId(2);
      quad[3] = poly->GetId(3);

      quads.push_back(quad);
      break;

    default:
      sofa::helper::vector<unsigned int> polygon;
      vtkIdType numIds = poly->GetNumberOfIds();

      polygon.reserve(numIds);

      for (vtkIdType i = 0; i < numIds; ++i)
        polygon.push_back(poly->GetId(i));

      polygons.push_back(polygon);
      break;
    }
  }

  this->positions.endEdit();
  this->edges.endEdit();
  this->triangles.endEdit();
  this->quads.endEdit();
  this->polygons.endEdit();

  return true;
}
