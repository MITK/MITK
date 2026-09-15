/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

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
  CPPUNIT_TEST_SUITE_END();

private:
  static mitk::Surface::Pointer MakeSphere(bool fixedSize32BitCells)
  {
    vtkNew<vtkSphereSource> source;
    source->SetThetaResolution(32);
    source->SetPhiResolution(32);
    source->Update();

    auto polyData = vtkSmartPointer<vtkPolyData>::New();
    polyData->DeepCopy(source->GetOutput());

    // The cell layout vtkSurfaceNets3D and other VTK 9.7 filters emit for
    // triangle meshes; ACVD's raw pointer access cannot handle it.
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

public:
  void Remesh_DefaultCellStorage_ProducesPolygons()
  {
    RemeshAndCheck(MakeSphere(false));
  }

  void Remesh_FixedSize32BitCellStorage_ProducesPolygons()
  {
    RemeshAndCheck(MakeSphere(true));
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkRemeshing)
