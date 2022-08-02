/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkRemeshing.h>
#include <mitkExceptionMacro.h>

#include <vtkIdList.h>
#include <vtkIntArray.h>
#include <vtkIsotropicDiscreteRemeshing.h>
#include <vtkMultiThreader.h>
#include <vtkPolyData.h>
#include <vtkPolyDataNormals.h>
#include <vtkSmartPointer.h>
#include <vtkSurface.h>

#include <array>
#include <vector>

namespace
{
  struct ClustersQuadrics final
  {
    explicit ClustersQuadrics(size_t size)
      : Elements(size),
        Size(size)
    {
      for (auto& array : Elements)
        array.fill(0.0);
    }

    ~ClustersQuadrics() = default;

    ClustersQuadrics(const ClustersQuadrics&) = delete;
    ClustersQuadrics& operator=(const ClustersQuadrics&) = delete;

    std::vector<std::array<double, 9>> Elements;
    size_t Size;
  };

  void ValidateSurface(const mitk::Surface* surface, mitk::TimeStepType t)
  {
    if (surface == nullptr)
      mitkThrow() << "Input surface is nullptr!";

    if (t >= surface->GetSizeOfPolyDataSeries())
      mitkThrow() << "Input surface doesn't have data at time step " << t << "!";

    auto* polyData = const_cast<mitk::Surface *>(surface)->GetVtkPolyData(t);

    if (polyData == nullptr)
      mitkThrow() << "PolyData of input surface at time step " << t << " is nullptr!";

    if (polyData->GetNumberOfPolys() == 0)
      mitkThrow() << "Input surface has no polygons at time step " << t << "!";
  }
}

mitk::Surface::Pointer mitk::Remesh(const Surface* surface,
                                    TimeStepType t,
                                    int numVertices,
                                    double gradation,
                                    int subsampling,
                                    double edgeSplitting,
                                    int optimizationLevel,
                                    bool forceManifold,
                                    bool boundaryFixing)
{
  ValidateSurface(surface, t);

  MITK_INFO << "Start remeshing...";

  auto surfacePolyData = vtkSmartPointer<vtkPolyData>::New();
  surfacePolyData->DeepCopy(const_cast<Surface *>(surface)->GetVtkPolyData(t));

  auto mesh = vtkSmartPointer<vtkSurface>::New();

  mesh->CreateFromPolyData(surfacePolyData);
  mesh->GetCellData()->Initialize();
  mesh->GetPointData()->Initialize();

  mesh->DisplayMeshProperties();

  if (numVertices == 0)
    numVertices = surfacePolyData->GetNumberOfPoints();

  if (edgeSplitting != 0.0)
    mesh->SplitLongEdges(edgeSplitting);

  auto remesher = vtkSmartPointer<vtkIsotropicDiscreteRemeshing>::New();

  remesher->GetMetric()->SetGradation(gradation);
  remesher->SetBoundaryFixing(boundaryFixing);
  remesher->SetConsoleOutput(1);
  remesher->SetForceManifold(forceManifold);
  remesher->SetInput(mesh);
  remesher->SetNumberOfClusters(numVertices);
  remesher->SetSubsamplingThreshold(subsampling);

  remesher->Remesh();

  // Optimization: Minimize distance between input surface and remeshed surface
  if (optimizationLevel != 0)
  {
    ClustersQuadrics clustersQuadrics(numVertices);

    auto faceList = vtkSmartPointer<vtkIdList>::New();
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
            vtkQuadricTools::AddTriangleQuadric(clustersQuadrics.Elements[cluster].data(), remesherInput, faceList->GetId(j), false);
        }
        else
        {
          vtkQuadricTools::AddTriangleQuadric(clustersQuadrics.Elements[cluster].data(), remesherInput, i, false);
        }
      }
      else
      {
        ++numMisclassifiedItems;
      }
    }

    if (numMisclassifiedItems != 0)
      MITK_INFO << numMisclassifiedItems << " items with wrong cluster association" << std::endl;

    vtkSmartPointer<vtkSurface> remesherOutput = remesher->GetOutput();
    double point[3];

    for (int i = 0; i < numVertices; ++i)
    {
      remesherOutput->GetPoint(i, point);
      vtkQuadricTools::ComputeRepresentativePoint(clustersQuadrics.Elements[i].data(), point, optimizationLevel);
      remesherOutput->SetPointCoordinates(i, point);
    }

    MITK_INFO << "After quadrics post-processing:" << std::endl;
    remesherOutput->DisplayMeshProperties();
  }

  auto normals = vtkSmartPointer<vtkPolyDataNormals>::New();

  normals->SetInputData(remesher->GetOutput());
  normals->AutoOrientNormalsOn();
  normals->ComputeCellNormalsOff();
  normals->ComputePointNormalsOn();
  normals->ConsistencyOff();
  normals->FlipNormalsOff();
  normals->NonManifoldTraversalOff();
  normals->SplittingOff();

  normals->Update();

  auto remeshedSurface = Surface::New();
  remeshedSurface->SetVtkPolyData(normals->GetOutput());

  MITK_INFO << "Finished remeshing";

  return remeshedSurface;
}

mitk::RemeshFilter::RemeshFilter()
  : m_TimeStep(0),
    m_NumVertices(0),
    m_Gradation(1.0),
    m_Subsampling(10),
    m_EdgeSplitting(0.0),
    m_OptimizationLevel(1),
    m_ForceManifold(false),
    m_BoundaryFixing(false)
{
  Surface::Pointer output = Surface::New();
  this->SetNthOutput(0, output);
}

mitk::RemeshFilter::~RemeshFilter()
{
}

void mitk::RemeshFilter::GenerateData()
{
  auto output = Remesh(this->GetInput(),
                       m_TimeStep,
                       m_NumVertices,
                       m_Gradation,
                       m_Subsampling,
                       m_EdgeSplitting,
                       m_OptimizationLevel,
                       m_ForceManifold,
                       m_BoundaryFixing);

  this->SetNthOutput(0, output);
}
