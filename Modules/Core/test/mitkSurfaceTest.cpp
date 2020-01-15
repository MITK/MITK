/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// Testing
#include "mitkTestFixture.h"
#include <mitkTestingMacros.h>

// std includes
#include <string>

// MITK includes
#include "mitkCommon.h"
#include "mitkNumericTypes.h"
#include "mitkSurface.h"

// MITK includes
#include <mitkWeakPointer.h>

// VTK includes
#include "vtkPolyData.h"
#include "vtkSphereSource.h"

// stream includes
#include <fstream>

class mitkSurfaceTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkSurfaceTestSuite);

  MITK_TEST(InitializationSurfacePointer_Success);
  MITK_TEST(InitializationCloneSurfacePointer_Success);

  MITK_TEST(StateOfVtkPolyDataEqualNullPointer_Success);

  MITK_TEST(SetVtkPolyDataNotNullPointer_Failure);
  MITK_TEST(SetClonedVtkPolyDataNotNullPointer_Failure);

  MITK_TEST(GetBoundingBox_Success);

  MITK_TEST(SurfaceExpandTimestepsAreFive_Success);
  MITK_TEST(Surface4DDataCreation_Success);

  MITK_TEST(TimeGeometrySurface_Success);
  MITK_TEST(ChangingDataOfSpecificTimestepSurface_Success);
  MITK_TEST(SurfaceCopyWithGraft_Failure);
  MITK_TEST(CopyingNumberOfTimesteps_Success);

  MITK_TEST(DestructionOfSurface_Success);

  CPPUNIT_TEST_SUITE_END();

private:
  mitk::Surface::Pointer m_Surface;
  mitk::Surface::Pointer m_CloneSurface;
  vtkSmartPointer<vtkSphereSource> m_SphereSource;
  const mitk::TimeGeometry *m_InputTimeGeometry;

  int m_Time;
  int m_Timestep;

public:
  void setUp() override
  {
    m_Surface = mitk::Surface::New();
    m_CloneSurface = m_Surface->Clone();
    m_SphereSource = vtkSmartPointer<vtkSphereSource>::New();
    m_InputTimeGeometry = m_Surface->GetUpdatedTimeGeometry();

    m_SphereSource->SetCenter(0, 0, 0);
    m_SphereSource->SetRadius(5.0);
    m_SphereSource->SetThetaResolution(10);
    m_SphereSource->SetPhiResolution(10);
    m_SphereSource->Update();

    m_Time = 3;
    m_Timestep = 0;
  }

  void tearDown() override
  {
    m_Surface = nullptr;
    m_CloneSurface = nullptr;
  }

  void InitializationSurfacePointer_Success()
  {
    CPPUNIT_ASSERT_MESSAGE("Testing initialization", m_Surface.GetPointer());
  }

  void InitializationCloneSurfacePointer_Success()
  {
    CPPUNIT_ASSERT_MESSAGE("Testing clone surface initialization", m_CloneSurface.GetPointer());
  }

  void StateOfVtkPolyDataEqualNullPointer_Success()
  {
    CPPUNIT_ASSERT_MESSAGE("Testing initial state of vtkPolyData", m_Surface->GetVtkPolyData() == nullptr);
  }

  void SetVtkPolyDataNotNullPointer_Failure()
  {
    vtkSmartPointer<vtkPolyData> polys = m_SphereSource->GetOutput();
    m_Surface->SetVtkPolyData(polys);
    CPPUNIT_ASSERT_MESSAGE("Testing set vtkPolyData", m_Surface->GetVtkPolyData() != nullptr);
  }

  void SetClonedVtkPolyDataNotNullPointer_Failure()
  {
    vtkSmartPointer<vtkPolyData> polys = m_SphereSource->GetOutput();
    m_Surface->SetVtkPolyData(polys);
    m_CloneSurface = m_Surface->Clone();
    CPPUNIT_ASSERT_MESSAGE("Testing set vtkPolyData of cloned surface!", m_CloneSurface->GetVtkPolyData() != nullptr);
  }

  void GetBoundingBox_Success()
  {
    vtkSmartPointer<vtkPolyData> polys = m_SphereSource->GetOutput();
    m_Surface->SetVtkPolyData(polys);

    double bounds[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
    polys->ComputeBounds();
    polys->GetBounds(bounds);

    m_Surface->UpdateOutputInformation();
    m_Surface->SetRequestedRegionToLargestPossibleRegion();
    auto *bb = const_cast<mitk::BoundingBox *>(m_Surface->GetGeometry()->GetBoundingBox());
    mitk::BoundingBox::BoundsArrayType surfBounds = bb->GetBounds();

    bool passed = false;
    if (bounds[0] == surfBounds[0] && bounds[1] == surfBounds[1] && bounds[2] == surfBounds[2] &&
        bounds[3] == surfBounds[3] && bounds[4] == surfBounds[4] && bounds[5] == surfBounds[5])
    {
      passed = true;
    }

    CPPUNIT_ASSERT_MESSAGE("Testing GetBoundingBox()", passed);
  }

  void SurfaceExpandTimestepsAreFive_Success()
  {
    m_Surface->Expand(5);
    m_Surface->Update();
    m_Surface->SetRequestedRegionToLargestPossibleRegion();
    mitk::Surface::RegionType requestedRegion = m_Surface->GetRequestedRegion();
    CPPUNIT_ASSERT_MESSAGE("Testing mitk::Surface::Expand( timesteps ): ", requestedRegion.GetSize(3) == 5);
  }

  void Surface4DDataCreation_Success()
  {
    double boundsMat[5][6];

    for (int i = 0; i < 5; i++)
    {
      vtkNew<vtkSphereSource> sphereSource;
      sphereSource->SetCenter(0, 0, 0);
      sphereSource->SetRadius(1.0 * (i + 1.0));
      sphereSource->SetThetaResolution(10);
      sphereSource->SetPhiResolution(10);
      sphereSource->Update();
      sphereSource->GetOutput()->ComputeBounds();
      sphereSource->GetOutput()->GetBounds(boundsMat[i]);
      m_Surface->SetVtkPolyData(sphereSource->GetOutput(), i);
    }

    m_Surface->UpdateOutputInformation();
    m_Surface->SetRequestedRegionToLargestPossibleRegion();

    bool passed = true;
    for (int i = 0; i < 5; i++)
    {
      mitk::BoundingBox::BoundsArrayType surfBounds =
        (const_cast<mitk::BoundingBox *>(m_Surface->GetTimeGeometry()->GetGeometryForTimeStep(i)->GetBoundingBox()))
          ->GetBounds();

      if (boundsMat[i][0] != surfBounds[0] || boundsMat[i][1] != surfBounds[1] || boundsMat[i][2] != surfBounds[2] ||
          boundsMat[i][3] != surfBounds[3] || boundsMat[i][4] != surfBounds[4] || boundsMat[i][5] != surfBounds[5])
      {
        passed = false;
        break;
      }
    }
    CPPUNIT_ASSERT_MESSAGE("Testing mitk::Surface::Testing 4D surface data creation", passed);
  }

  void TimeGeometrySurface_Success()
  {
    m_Timestep = m_InputTimeGeometry->TimePointToTimeStep(m_Time);
    CPPUNIT_ASSERT_MESSAGE("Testing correctness of geometry for surface->GetUpdatedTimeGeometry()",
                           m_Time == m_Timestep);
  }

  void ChangingDataOfSpecificTimestepSurface_Success()
  {
    vtkNew<vtkSphereSource> sphereSource;
    sphereSource->SetCenter(0, 0, 0);
    sphereSource->SetRadius(100.0);
    sphereSource->SetThetaResolution(10);
    sphereSource->SetPhiResolution(10);
    sphereSource->Update();
    m_Surface->SetVtkPolyData(sphereSource->GetOutput(), 3);

    m_Timestep = m_InputTimeGeometry->TimePointToTimeStep(m_Time);
    CPPUNIT_ASSERT_MESSAGE(
      "Explicitly changing the data of timestep 3 and checking for timebounds correctness of surface's geometry again",
      m_Time == m_Timestep);
  }

  void SurfaceCopyWithGraft_Failure()
  {
    double boundsMat[5][6];

    for (int i = 0; i < 5; i++)
    {
      vtkNew<vtkSphereSource> sphereSource;
      sphereSource->SetCenter(0, 0, 0);
      sphereSource->SetRadius(1.0 * (i + 1.0));
      sphereSource->SetThetaResolution(10);
      sphereSource->SetPhiResolution(10);
      sphereSource->Update();
      sphereSource->GetOutput()->ComputeBounds();
      sphereSource->GetOutput()->GetBounds(boundsMat[i]);
      m_Surface->SetVtkPolyData(sphereSource->GetOutput(), i);
    }

    m_Surface->UpdateOutputInformation();
    m_Surface->SetRequestedRegionToLargestPossibleRegion();

    mitk::Surface::Pointer dummy = mitk::Surface::New();
    dummy->Graft(m_Surface);
    CPPUNIT_ASSERT_MESSAGE("Testing copying a Surface with Graft()", dummy->GetVtkPolyData() != nullptr);
  }

  void CopyingNumberOfTimesteps_Success()
  {
    double boundsMat[5][6];

    for (int i = 0; i < 5; i++)
    {
      vtkNew<vtkSphereSource> sphereSource;
      sphereSource->SetCenter(0, 0, 0);
      sphereSource->SetRadius(1.0 * (i + 1.0));
      sphereSource->SetThetaResolution(10);
      sphereSource->SetPhiResolution(10);
      sphereSource->Update();
      sphereSource->GetOutput()->ComputeBounds();
      sphereSource->GetOutput()->GetBounds(boundsMat[i]);
      m_Surface->SetVtkPolyData(sphereSource->GetOutput(), i);
    }

    m_Surface->UpdateOutputInformation();
    m_Surface->SetRequestedRegionToLargestPossibleRegion();

    unsigned int numberoftimesteps = m_Surface->GetTimeSteps();
    mitk::Surface::Pointer dummy = mitk::Surface::New();
    dummy->Graft(m_Surface);

    CPPUNIT_ASSERT_MESSAGE(" Old timesteps == copy of timesteps ", dummy->GetTimeSteps() == numberoftimesteps);
  }

  void DestructionOfSurface_Success()
  {
    m_Surface = nullptr;
    CPPUNIT_ASSERT_MESSAGE("Testing destruction of surface", m_Surface.IsNull());
  }
};
MITK_TEST_SUITE_REGISTRATION(mitkSurface)
