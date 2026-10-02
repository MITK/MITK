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
#include <vtkPolyData.h>
#include <vtkPolyDataNormals.h>
#include <vtkQuadricTools.h>
#include <vtkSMPThreadLocalObject.h>
#include <vtkSMPTools.h>
#include <vtkSmartPointer.h>
#include <vtkSurface.h>

#include <array>
#include <vector>

namespace
{
  // ACVD runs its connectivity-constrained clustering phase until not a single
  // item changes its cluster anymore. Its last loops move a handful of items
  // each, at the cost of a full loop, and can take most of the clustering time
  // without changing the triangle quality or the distance to the input. The
  // phase ends once a loop moves no more than this fraction of the items.
  constexpr double MinimumMovedItemsRatio = 1e-4;

  /** \brief ACVD's isotropic remeshing, ending its clustering early once it
   * has practically converged.
   */
  class Remesher : public vtkIsotropicDiscreteRemeshing
  {
  public:
    static Remesher* New()
    {
      auto* remesher = new Remesher;
      remesher->InitializeObjectBase();
      return remesher;
    }

  protected:
    Remesher()
      : m_LoopBudget(this->MaxNumberOfLoops)
    {
    }

    ~Remesher() override = default;

    void MinimizeEnergy() override
    {
      // A manifold output makes ACVD minimize again after each round of
      // topology fixes. ProcessOneLoop() ends a run by exhausting its loop
      // budget, so every run starts with a full one.
      this->MaxNumberOfLoops = this->NumberOfLoops + m_LoopBudget;

      vtkIsotropicDiscreteRemeshing::MinimizeEnergy();
    }

    int ProcessOneLoop() override
    {
      const int movedItems = vtkIsotropicDiscreteRemeshing::ProcessOneLoop();

      const bool converged = this->ConnexityConstraint &&
        movedItems <= MinimumMovedItemsRatio * this->GetNumberOfItems();

      // Exhausting the loop budget ends the clustering through its own exit,
      // which still reconnects split clusters. Throwing from here instead would
      // leave the clustering half done and leak what ACVD allocated.
      if (converged)
        this->MaxNumberOfLoops = this->NumberOfLoops;

      return movedItems;
    }

  private:
    int m_LoopBudget;
  };

  void ValidateSurface(const mitk::Surface* surface, mitk::TimeStepType t)
  {
    if (surface == nullptr)
      mitkThrow() << "Input surface is nullptr!";

    if (t >= surface->GetSizeOfPolyDataSeries())
      mitkThrow() << "Input surface doesn't have data at time step " << t << "!";

    auto* polyData = surface->GetVtkPolyData(t);

    if (polyData == nullptr)
      mitkThrow() << "PolyData of input surface at time step " << t << " is nullptr!";

    if (polyData->GetNumberOfPolys() == 0)
      mitkThrow() << "Input surface has no polygons at time step " << t << "!";
  }

  /** \brief Moves every vertex of the remeshed surface to the minimum of the
   * quadric of the input triangles around its cluster.
   */
  void OptimizeVertexPositions(Remesher* remesher, int optimizationLevel)
  {
    vtkIntArray* clustering = remesher->GetClustering();
    vtkSurface* input = remesher->GetInput();
    vtkSurface* output = remesher->GetOutput();
    const vtkIdType numItems = remesher->GetNumberOfItems();
    const int numClusters = remesher->GetNumberOfClusters();

    // Items grouped by cluster, so that every quadric is summed up by a single
    // thread: items of cluster c are clusterItems[clusterBegin[c]] up to
    // clusterItems[clusterBegin[c + 1]].
    std::vector<vtkIdType> clusterBegin(numClusters + 1, 0);
    vtkIdType numUnclusteredItems = 0;

    for (vtkIdType i = 0; i < numItems; ++i)
    {
      const int cluster = clustering->GetValue(i);

      if (cluster >= 0 && cluster < numClusters)
        ++clusterBegin[cluster + 1];
      else
        ++numUnclusteredItems;
    }

    if (numUnclusteredItems != 0)
      MITK_WARN << numUnclusteredItems << " items of the input surface do not belong to any cluster";

    for (int cluster = 0; cluster < numClusters; ++cluster)
      clusterBegin[cluster + 1] += clusterBegin[cluster];

    std::vector<vtkIdType> clusterItems(clusterBegin.back());
    std::vector<vtkIdType> nextSlot(clusterBegin.begin(), clusterBegin.end() - 1);

    for (vtkIdType i = 0; i < numItems; ++i)
    {
      const int cluster = clustering->GetValue(i);

      if (cluster >= 0 && cluster < numClusters)
        clusterItems[nextSlot[cluster]++] = i;
    }

    std::vector<std::array<double, 3>> points(numClusters);
    vtkSMPThreadLocalObject<vtkIdList> faceLists;

    vtkSMPTools::For(0, numClusters, [&](vtkIdType begin, vtkIdType end) {
      vtkIdList* faceList = faceLists.Local();

      for (vtkIdType cluster = begin; cluster < end; ++cluster)
      {
        std::array<double, 9> quadric{};

        for (vtkIdType i = clusterBegin[cluster]; i < clusterBegin[cluster + 1]; ++i)
        {
          input->GetVertexNeighbourFaces(clusterItems[i], faceList);
          const vtkIdType numFaces = faceList->GetNumberOfIds();

          for (vtkIdType j = 0; j < numFaces; ++j)
            vtkQuadricTools::AddTriangleQuadric(quadric.data(), input, faceList->GetId(j), false);
        }

        auto& point = points[cluster];
        output->GetPoint(cluster, point.data());
        vtkQuadricTools::ComputeRepresentativePoint(quadric.data(), point.data(), optimizationLevel);
      }
    });

    for (int cluster = 0; cluster < numClusters; ++cluster)
      output->SetPointCoordinates(cluster, points[cluster].data());
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

  auto surfacePolyData = vtkSmartPointer<vtkPolyData>::New();
  surfacePolyData->DeepCopy(surface->GetVtkPolyData(t));

  auto mesh = vtkSmartPointer<vtkSurface>::New();

  mesh->CreateFromPolyData(surfacePolyData);
  mesh->GetCellData()->Initialize();
  mesh->GetPointData()->Initialize();

  if (numVertices == 0)
    numVertices = surfacePolyData->GetNumberOfPoints();

  if (edgeSplitting != 0.0)
    mesh->SplitLongEdges(edgeSplitting);

  auto remesher = vtkSmartPointer<Remesher>::New();

  remesher->GetMetric()->SetGradation(gradation);
  remesher->SetBoundaryFixing(boundaryFixing);
  remesher->SetForceManifold(forceManifold);
  remesher->SetInput(mesh);
  remesher->SetNumberOfClusters(numVertices);
  remesher->SetSubsamplingThreshold(subsampling);

  remesher->Remesh();

  // Optimization: Minimize distance between input surface and remeshed surface
  if (optimizationLevel != 0)
    OptimizeVertexPositions(remesher, optimizationLevel);

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
