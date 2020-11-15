/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkRemeshing.h>

#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <OpenMesh/Tools/Decimater/DecimaterT.hh>
#include <OpenMesh/Tools/Decimater/ModQuadricT.hh>

#include <vtkCellArray.h>
#include <vtkIdList.h>
#include <vtkPoints.h>
#include <vtkPolyData.h>
#include <vtkPolyDataNormals.h>
#include <vtkSmartPointer.h>
#include <vtkTriangleFilter.h>

#include <algorithm>
#include <functional>

using Mesh = OpenMesh::TriMesh_ArrayKernelT<OpenMesh::DefaultTraitsDouble>;

namespace
{
  bool IsValidPolyData(vtkPolyData* polyData)
  {
    return nullptr != polyData && 0 < polyData->GetNumberOfPoints() &&
      (0 < polyData->GetNumberOfPolys() || 0 < polyData->GetNumberOfStrips());
  }

  vtkSmartPointer<vtkPolyData> TriangulatePolyData(vtkPolyData* polyData)
  {
    auto triangleFilter = vtkSmartPointer<vtkTriangleFilter>::New();
    triangleFilter->SetInputData(polyData);
    triangleFilter->PassVertsOff();
    triangleFilter->PassLinesOff();

    triangleFilter->Update();

    return triangleFilter->GetOutput();
  }

  vtkSmartPointer<vtkPolyData> CalculateNormals(vtkPolyData* polyData, bool flipNormals)
  {
    auto polyDataNormals = vtkSmartPointer<vtkPolyDataNormals>::New();
    polyDataNormals->SetInputData(polyData);

    if (flipNormals)
      polyDataNormals->FlipNormalsOn();

    polyDataNormals->Update();

    return polyDataNormals->GetOutput();
  }

  Mesh ConvertPolyDataToMesh(vtkPolyData* polyData)
  {
    Mesh mesh;

    auto* points = polyData->GetPoints();
    const auto numPoints = points->GetNumberOfPoints();

    for (std::remove_const_t<decltype(numPoints)> i = 0; i < numPoints; ++i)
      mesh.add_vertex(Mesh::Point(points->GetPoint(i)));

    auto* polys = polyData->GetPolys();
    const auto numPolys = polys->GetNumberOfCells();

    auto ids = vtkSmartPointer<vtkIdList>::New();
    std::array<Mesh::VertexHandle, 3> vertexHandles;

    for (std::remove_const_t<decltype(numPolys)> i = 0; i < numPolys; ++i)
    {
      polys->GetCellAtId(i, ids);

      vertexHandles[0] = Mesh::VertexHandle(static_cast<int>(ids->GetId(0)));
      vertexHandles[1] = Mesh::VertexHandle(static_cast<int>(ids->GetId(1)));
      vertexHandles[2] = Mesh::VertexHandle(static_cast<int>(ids->GetId(2)));

      mesh.add_face(vertexHandles.data(), 3);
    }

    return mesh;
  }

  vtkSmartPointer<vtkPolyData> ConvertMeshToPolyData(const Mesh& mesh)
  {
    auto polyData = vtkSmartPointer<vtkPolyData>::New();

    const auto numVertices = mesh.n_vertices();
    auto points = vtkSmartPointer<vtkPoints>::New();
    points->SetNumberOfPoints(numVertices);

    for (std::remove_const_t<decltype(numVertices)> i = 0; i < numVertices; ++i)
      points->SetPoint(i, mesh.point(Mesh::VertexHandle(static_cast<int>(i))).data());

    polyData->SetPoints(points);

    const auto numFaces = mesh.n_faces();
    auto polys = vtkSmartPointer<vtkCellArray>::New();

    auto ids = vtkSmartPointer<vtkIdList>::New();
    ids->SetNumberOfIds(3);
    Mesh::CFVIter iter;

    for (std::remove_const_t<decltype(numFaces)> i = 0; i < numFaces; ++i)
    {
      iter = mesh.cfv_iter(Mesh::FaceHandle(static_cast<int>(i)));

      ids->SetId(0, (iter++)->idx());
      ids->SetId(1, (iter++)->idx());
      ids->SetId(2, iter->idx());

      polys->InsertNextCell(ids);
    }

    polyData->SetPolys(polys);

    return polyData;
  }

  mitk::Surface::Pointer ProcessEachTimeStep(const mitk::Surface* input, bool calculateNormals, bool flipNormals, const std::function<void(Mesh&)>& ProcessMesh)
  {
    if (nullptr == input || !input->IsInitialized())
      return nullptr;

    auto output = mitk::Surface::New();
    const auto numTimeSteps = input->GetTimeSteps();

    for (std::remove_const_t<decltype(numTimeSteps)> t = 0; t < numTimeSteps; ++t)
    {
      vtkSmartPointer<vtkPolyData> polyData = input->GetVtkPolyData(t);

      if (IsValidPolyData(polyData))
      {
        polyData = TriangulatePolyData(polyData);

        if (IsValidPolyData(polyData))
        {
          auto mesh = ConvertPolyDataToMesh(polyData);
          ProcessMesh(mesh);
          polyData = ConvertMeshToPolyData(mesh);

          if (calculateNormals)
            polyData = CalculateNormals(polyData, flipNormals);

          output->SetVtkPolyData(polyData, t);
          continue;
        }
      }

      output->SetVtkPolyData(nullptr, t);
    }

    return output;
  }
}

mitk::Surface::Pointer mitk::Remeshing::Decimate(const Surface* input, double percent, bool calculateNormals, bool flipNormals)
{
  return ProcessEachTimeStep(input, calculateNormals, flipNormals, [percent](Mesh& mesh) {
    using Decimater = OpenMesh::Decimater::DecimaterT<Mesh>;
    using HModQuadric = OpenMesh::Decimater::ModQuadricT<Mesh>::Handle;

    Decimater decimater(mesh);

    HModQuadric hModQuadric;
    decimater.add(hModQuadric);
    decimater.module(hModQuadric).unset_max_err();

    decimater.initialize();
    decimater.decimate_to(mesh.n_vertices() * std::max(0.0, std::min(percent, 1.0)));

    mesh.garbage_collection();
  });
}
