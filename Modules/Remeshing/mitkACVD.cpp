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

#include "mitkACVD.h"
#include <vtkIdList.h>
#include <vtkIntArray.h>
#include <vtkIsotropicDiscreteRemeshing.h>
#include <vtkMultiThreader.h>
#include <vtkPolyData.h>
#include <vtkPolyDataNormals.h>
#include <vtkSmartPointer.h>
#include <vtkSurface.h>

struct ClustersQuadrics
{
  explicit ClustersQuadrics(int size)
    : Elements(new double*[size]),
      Size(size)
  {
    for (int i = 0; i < size; ++i)
    {
      Elements[i] = new double[9];

      for (int j = 0; j < 9; ++j)
        Elements[i][j] = 0.0;
    }
  }

  ~ClustersQuadrics()
  {
    for (int i = 0; i < Size; ++i)
      delete[] Elements[i];

    delete Elements;
  }

  double** Elements;
  int Size;

private:
  ClustersQuadrics(const ClustersQuadrics&);
  ClustersQuadrics& operator=(const ClustersQuadrics&);
};

static bool ValidateSurface(mitk::Surface::Pointer surface, unsigned int t)
{
  if (surface.IsNull())
  {
    MITK_ERROR << "Input surface is NULL!";
    return false;
  }

  if (t >= surface->GetSizeOfPolyDataSeries())
  {
    MITK_ERROR << "Input surface doesn't have data at time step " << t << "!";
    return false;
  }

  vtkPolyData* polyData = surface->GetVtkPolyData(t);

  if (polyData == NULL)
  {
    MITK_ERROR << "PolyData of input surface at time step " << t << " is NULL!";
    return false;
  }

  if (polyData->GetNumberOfPolys() == 0)
  {
    MITK_ERROR << "Input surface has no polygons at time step " << t << "!";
    return false;
  }

  return true;
}

mitk::Surface::Pointer mitk::ACVD::Remesh(mitk::Surface::Pointer surface, unsigned int t, int numVertices, double gradation, int subsampling, double edgeSplitting, int optimizationLevel, bool forceManifold, bool boundaryFixing)
{
  if (!ValidateSurface(surface, t))
    return NULL;

  MITK_INFO << "Start remeshing...";

  vtkSmartPointer<vtkPolyData> surfacePolyData = vtkSmartPointer<vtkPolyData>::New();
  surfacePolyData->DeepCopy(surface->GetVtkPolyData(t));

  vtkSmartPointer<vtkSurface> mesh = vtkSmartPointer<vtkSurface>::New();

  mesh->CreateFromPolyData(surfacePolyData);
  mesh->GetCellData()->Initialize();
  mesh->GetPointData()->Initialize();

  mesh->DisplayMeshProperties();

  if (edgeSplitting != 0.0)
    mesh->SplitLongEdges(edgeSplitting);

  vtkSmartPointer<vtkIsotropicDiscreteRemeshing> remesher = vtkSmartPointer<vtkIsotropicDiscreteRemeshing>::New();

  remesher->GetMetric()->SetGradation(gradation);
  remesher->SetBoundaryFixing(boundaryFixing);
  remesher->SetConsoleOutput(1);
  remesher->SetForceManifold(forceManifold);
  remesher->SetInput(mesh);
  remesher->SetNumberOfClusters(numVertices);
  remesher->SetNumberOfThreads(vtkMultiThreader::GetGlobalDefaultNumberOfThreads());
  remesher->SetSubsamplingThreshold(subsampling);

  remesher->Remesh();

  // Optimization: Minimize distance between input surface and remeshed surface
  if (optimizationLevel != 0)
  {
    ClustersQuadrics clustersQuadrics(numVertices);

    vtkSmartPointer<vtkIdList> faceList = vtkSmartPointer<vtkIdList>::New();
    vtkSmartPointer<vtkIntArray> clustering = remesher->GetClustering();
    vtkSmartPointer<vtkSurface> remesherInput = remesher->GetInput();
    int clusteringType = remesher->GetClusteringType();
    int numItems = remesher->GetNumberOfItems();
    int numMisclassifiedItems = 0;

    for (int i = 0; i < numItems; ++i)
    {
      int cluster = clustering->GetValue(i);

      if (cluster >= 0 && cluster < numVertices)
      {
        if (clusteringType != 0)
        {
          remesherInput->GetVertexNeighbourFaces(i, faceList);
          int numIds = static_cast<int>(faceList->GetNumberOfIds());

          for (int j = 0; j < numIds; ++j)
            vtkQuadricTools::AddTriangleQuadric(clustersQuadrics.Elements[cluster], remesherInput, faceList->GetId(j), false);
        }
        else
        {
          vtkQuadricTools::AddTriangleQuadric(clustersQuadrics.Elements[cluster], remesherInput, i, false);
        }
      }
      else
      {
        ++numMisclassifiedItems;
      }
    }

    if (numMisclassifiedItems != 0)
      std::cout << numMisclassifiedItems << " items with wrong cluster association" << std::endl;

    vtkSmartPointer<vtkSurface> remesherOutput = remesher->GetOutput();
    double point[3];

    for (int i = 0; i < numVertices; ++i)
    {
      remesherOutput->GetPoint(i, point);
      vtkQuadricTools::ComputeRepresentativePoint(clustersQuadrics.Elements[i], point, optimizationLevel);
      remesherOutput->SetPointCoordinates(i, point);
    }

    std::cout << "After quadrics post-processing:" << std::endl;
    remesherOutput->DisplayMeshProperties();
  }

  vtkSmartPointer<vtkPolyDataNormals> normals = vtkSmartPointer<vtkPolyDataNormals>::New();

  normals->SetInput(remesher->GetOutput());
  normals->AutoOrientNormalsOn();
  normals->ComputeCellNormalsOff();
  normals->ComputePointNormalsOn();
  normals->ConsistencyOff();
  normals->FlipNormalsOff();
  normals->NonManifoldTraversalOff();
  normals->SplittingOff();

  normals->Update();

  Surface::Pointer remeshedSurface = Surface::New();
  remeshedSurface->SetVtkPolyData(normals->GetOutput());

  MITK_INFO << "Finished remeshing";

  return remeshedSurface;
}
