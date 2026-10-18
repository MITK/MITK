/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkCoreServices.h>
#include <mitkGeometry3D.h>
#include <mitkIProgressService.h>
#include <mitkProgressTask.h>
#include <mitkProportionalTimeGeometry.h>
#include <mitkRemeshing.h>
#include <mitkSurface.h>
#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <vtkCellArray.h>
#include <vtkNew.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkSphereSource.h>

class mitkRemeshingTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkRemeshingTestSuite);
  MITK_TEST(Remesh_DefaultCellStorage_ProducesPolygons);
  MITK_TEST(Remesh_FixedSize32BitCellStorage_ProducesPolygons);
  MITK_TEST(RemeshFilter_TimeSteps_EachRemeshedToItsOwnDensity);
  MITK_TEST(RemeshFilter_NoPolygons_Throws);
  MITK_TEST(EstimateMemory_TimeSteps_CoversMostDemandingTimeStep);
  MITK_TEST(SetInput_TimeSteps_InputNotCopied);
  MITK_TEST(Remesh_CancelRequested_AbortsBeforeTouchingInput);
  CPPUNIT_TEST_SUITE_END();

private:
  static mitk::Surface::Pointer MakeSphere(bool fixedSize32BitCells, int resolution = 32)
  {
    vtkNew<vtkSphereSource> source;
    source->SetThetaResolution(resolution);
    source->SetPhiResolution(resolution);
    source->Update();

    auto polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->DeepCopy(source->GetOutput());

    // The cell layout vtkSurfaceNets3D and other VTK 9.7 filters emit for
    // triangle meshes. ACVD reads polygons through raw 64-bit pointers and
    // crashed on it before converting the storage in CreateFromPolyData().
    if (fixedSize32BitCells)
      CPPUNIT_ASSERT(polyData->GetPolys()->ConvertToFixedSize32BitStorage());

    auto surface = mitk::Surface::New();
    surface->SetVtkPolyData(polyData);
    return surface;
  }

  static void RemeshAndCheck(const mitk::Surface* surface)
  {
    auto result = mitk::Remesh(surface, 0, 200, 1.0, 10, 0.0, 1, false, false);

    CPPUNIT_ASSERT(result.IsNotNull());
    auto* polyData = result->GetVtkPolyData();
    CPPUNIT_ASSERT(polyData != nullptr);
    CPPUNIT_ASSERT(polyData->GetNumberOfPoints() > 0);
    CPPUNIT_ASSERT(polyData->GetNumberOfPolys() > 0);
  }

  // A small and a large sphere with an empty time step in between, the way
  // polygon models of segmentations look when a label is absent at a time step.
  static mitk::Surface::Pointer MakeTimeResolvedSphere()
  {
    auto timeGeometry = mitk::ProportionalTimeGeometry::New();
    timeGeometry->Initialize(mitk::Geometry3D::New(), 3);
    timeGeometry->SetFirstTimePoint(10.0);
    timeGeometry->SetStepDuration(5.0);

    auto surface = mitk::Surface::New();
    surface->SetTimeGeometry(timeGeometry);
    surface->Expand(3);
    surface->SetVtkPolyData(MakeSphere(false, 32)->GetVtkPolyData(), 0);
    surface->SetVtkPolyData(vtkSmartPointer<vtkPolyData>::New(), 1);
    surface->SetVtkPolyData(MakeSphere(false, 64)->GetVtkPolyData(), 2);
    return surface;
  }

public:
  void Remesh_DefaultCellStorage_ProducesPolygons()
  {
    RemeshAndCheck(MakeSphere(false));
  }

  void Remesh_FixedSize32BitCellStorage_ProducesPolygons()
  {
    RemeshAndCheck(MakeSphere(true));
  }

  void RemeshFilter_TimeSteps_EachRemeshedToItsOwnDensity()
  {
    const auto input = MakeTimeResolvedSphere();
    mitk::ProgressTask task("Remeshing");

    auto remesher = mitk::RemeshFilter::New();
    remesher->SetInput(input);
    remesher->SetDensity(0.5);
    remesher->SetProgressTask(&task);
    remesher->Update();

    const auto* output = remesher->GetOutput();
    CPPUNIT_ASSERT_EQUAL(mitk::TimeStepType(3), output->GetTimeGeometry()->CountTimeSteps());
    CPPUNIT_ASSERT_EQUAL(15.0, output->GetTimeGeometry()->GetTimeBounds(1)[0]);

    // The spheres are closed, so remeshing hits the vertex count exactly.
    for (const unsigned int timeStep : { 0u, 2u })
    {
      const auto numInputVertices = input->GetVtkPolyData(timeStep)->GetNumberOfPoints();
      CPPUNIT_ASSERT_EQUAL(numInputVertices / 2, output->GetVtkPolyData(timeStep)->GetNumberOfPoints());
    }

    CPPUNIT_ASSERT_EQUAL(vtkIdType(0), output->GetVtkPolyData(1)->GetNumberOfPoints());
    CPPUNIT_ASSERT_EQUAL(2u, task.GetStepsToDo());
  }

  void RemeshFilter_NoPolygons_Throws()
  {
    auto input = mitk::Surface::New();
    input->SetVtkPolyData(vtkSmartPointer<vtkPolyData>::New());

    auto remesher = mitk::RemeshFilter::New();
    remesher->SetInput(input);

    CPPUNIT_ASSERT_THROW(remesher->Update(), mitk::Exception);
    CPPUNIT_ASSERT_THROW(remesher->EstimateMemory(), mitk::Exception);
  }

  void EstimateMemory_TimeSteps_CoversMostDemandingTimeStep()
  {
    const auto input = MakeTimeResolvedSphere();

    auto remesher = mitk::RemeshFilter::New();
    remesher->SetInput(input);
    remesher->SetDensity(0.5);

    const auto numVertices = static_cast<int>(input->GetVtkPolyData(2)->GetNumberOfPoints() / 2);
    CPPUNIT_ASSERT(remesher->EstimateMemory() > mitk::EstimateRemeshingMemory(input, 2, numVertices));

    // The time steps are remeshed one after another, so their estimates do not add up.
    const auto numVerticesOfFirst = static_cast<int>(input->GetVtkPolyData(0)->GetNumberOfPoints() / 2);
    CPPUNIT_ASSERT(remesher->EstimateMemory() < mitk::EstimateRemeshingMemory(input, 0, numVerticesOfFirst) +
                                                  mitk::EstimateRemeshingMemory(input, 2, numVertices));
  }

  void SetInput_TimeSteps_InputNotCopied()
  {
    // The Remeshing view sets the input for every memory estimate. The output
    // stays the empty surface the filter starts with; asking it for poly data
    // would run the filter.
    auto remesher = mitk::RemeshFilter::New();
    remesher->SetInput(MakeTimeResolvedSphere());

    CPPUNIT_ASSERT_EQUAL(1u, remesher->GetOutput()->GetSizeOfPolyDataSeries());
  }

  void Remesh_CancelRequested_AbortsBeforeTouchingInput()
  {
    auto* progressService = mitk::CoreServices::GetProgressService();
    CPPUNIT_ASSERT(nullptr != progressService);

    mitk::ProgressTask task("Remeshing", mitk::ProgressTask::Indeterminate, true);
    progressService->RequestCancel(task.GetId());
    mitk::CoreServices::Unget(progressService);

    // Without polygons, an input that remeshing looked at would be refused
    // with an mitk::Exception.
    auto input = mitk::Surface::New();
    input->SetVtkPolyData(vtkSmartPointer<vtkPolyData>::New());

    CPPUNIT_ASSERT_THROW(mitk::Remesh(input, 0, 100, 0.5, 10, 0.0, 1, false, false, &task), itk::ProcessAborted);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkRemeshing)
