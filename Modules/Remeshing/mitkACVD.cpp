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
#include <vtkIsotropicDiscreteRemeshing.h>
#include <vtkMultiThreader.h>
#include <vtkPolyData.h>
#include <vtkPolyDataNormals.h>
#include <vtkSmartPointer.h>
#include <vtkSurface.h>

mitk::Surface::Pointer mitk::ACVD::Remesh(mitk::Surface::Pointer surface, int numVertices, double gradation, int subsampling, double edgeSplitting, bool forceManifold, bool boundaryFixing)
{
  MITK_INFO << "Start remeshing...";

  vtkSmartPointer<vtkPolyData> surfacePolyData = vtkSmartPointer<vtkPolyData>::New();
  surfacePolyData->DeepCopy(surface->GetVtkPolyData());

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
