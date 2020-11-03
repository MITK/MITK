/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <fstream>
#include <mitkGeometry3D.h>
#include <mitkGeometryData.h>
#include <mitkNumericTypes.h>
#include <mitkPlaneFit.h>
#include <mitkPlaneGeometry.h>
#include <mitkPointSet.h>

int mitkPlaneFitTest(int, char *[])
{
  // float bounds[]={0.0f,10.0f,0.0f,10.0f,0.0f,5.0f};

  mitk::PlaneFit::Pointer PlaneFit = mitk::PlaneFit::New();
  mitk::PointSet::Pointer PointSet = mitk::PointSet::New();

  mitk::Point3D Point;

  // first without any point, then incrementally add points within thre points there will be a plane geometry
  std::cout << "Start PlaneFitTest " << std::endl;
  for (int position = 0; position < 6; position++)
  {
    // add a point directly
    mitk::FillVector3D(Point, (float)position, (float)position * 1.5, 2.5);
    PointSet->GetPointSet()->GetPoints()->InsertElement(position, Point);
  }

  // Set Input
  PlaneFit->SetInput(PointSet);

  const mitk::PointSet *testPointSet = PlaneFit->GetInput();
  std::cout << "  Size test of Input Method: ";
  if (testPointSet->GetSize() == PointSet->GetSize())
  {
    std::cout << "[PASSED]" << std::endl;
  }
  else
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }

  // Test Centroid
  std::cout << "  Testing centroid calculaation: ";
  PlaneFit->Update();
  const mitk::Point3D &centroid = PlaneFit->GetCentroid();
  mitk::Point3D expectedCentroid;
  expectedCentroid[0] = 2.5;
  expectedCentroid[1] = 3.75;
  expectedCentroid[2] = 2.5;

  if (centroid == expectedCentroid)
  {
    std::cout << "[PASSED]" << std::endl;
  }
  else
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }

  // Test PlaneGeometry
  std::cout << "  Test PlaneGeometry: ";
  auto *PlaneGeometry = dynamic_cast<mitk::PlaneGeometry *>(PlaneFit->GetOutput()->GetGeometry());
  if (PlaneGeometry)
  {
    std::cout << "[PASSED]" << std::endl;
  }
  else
  {
    std::cout << "[FAILED]" << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "[TEST DONE]" << std::endl;
  return EXIT_SUCCESS;
}
