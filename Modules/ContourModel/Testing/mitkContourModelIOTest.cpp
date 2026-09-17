/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/
#include "mitkTestingConfig.h"
#include <mitkTestingMacros.h>

#include <fstream>
#include <iostream>
#include <locale>
#include <mitkIOUtil.h>
#include <mitkContourModel.h>

static void TestContourModel(mitk::ContourModel *contour, std::string fileName)
{
  std::string filename = std::string(MITK_TEST_OUTPUT_DIR) + fileName;

  mitk::IOUtil::Save(contour, filename);

  std::vector<itk::SmartPointer<mitk::BaseData>> readerOutput = mitk::IOUtil::Load(filename);

  mitk::ContourModel::Pointer contour2 = dynamic_cast<mitk::ContourModel *>(readerOutput.at(0).GetPointer());

  MITK_TEST_CONDITION_REQUIRED(contour2.IsNotNull(), "contour is not null");

  MITK_TEST_CONDITION_REQUIRED(contour->GetTimeSteps() == contour2->GetTimeSteps(),
                               "contours have the same number of time steps");

  bool sameVertexCounts = true;
  bool sameVertices = true;
  bool sameControlPointFlags = true;
  bool sameClosedFlags = true;

  for (unsigned int t = 0; t < contour->GetTimeSteps(); ++t)
  {
    sameVertexCounts &= contour->GetNumberOfVertices(t) == contour2->GetNumberOfVertices(t);
    sameClosedFlags &= contour->IsClosed(t) == contour2->IsClosed(t);

    auto it = contour->IteratorBegin(t);
    auto end = contour->IteratorEnd(t);

    auto it2 = contour2->IteratorBegin(t);
    auto end2 = contour2->IteratorEnd(t);

    while (it != end && it2 != end2)
    {
      sameVertices &= ((*it)->Coordinates == (*it2)->Coordinates);
      sameControlPointFlags &= ((*it)->IsControlPoint == (*it2)->IsControlPoint);
      it++;
      it2++;
    }
  }

  MITK_TEST_CONDITION(sameVertexCounts, "time steps have the same number of vertices");
  MITK_TEST_CONDITION(sameVertices, "vertices are equal");
  MITK_TEST_CONDITION(sameControlPointFlags, "control point flags are equal");
  MITK_TEST_CONDITION(sameClosedFlags, "closed flags are equal");
}

static void TestContourModelIO_OneTimeStep()
{
  mitk::ContourModel::Pointer contour = mitk::ContourModel::New();

  mitk::Point3D p;
  p[0] = p[1] = p[2] = 0;

  contour->AddVertex(p);

  mitk::Point3D p2;
  p2[0] = p2[1] = p2[2] = 1;

  contour->AddVertex(p2);

  mitk::Point3D p3;
  p3[0] = -2;
  p3[1] = 10;
  p3[2] = 0;

  contour->AddVertex(p3);

  mitk::Point3D p4;
  p4[0] = -3;
  p4[1] = 6;
  p4[2] = -5;

  contour->AddVertex(p4);

  TestContourModel(contour.GetPointer(), "/contour.cnt");
}

static void TestContourModelIO_MultipleTimeSteps()
{
  const unsigned int numberOfTimeSteps = 4;
  const unsigned int numberOfVertices = 5;

  mitk::ContourModel::Pointer contour = mitk::ContourModel::New();
  contour->Expand(numberOfTimeSteps);

  for (unsigned int t = 0; t < numberOfTimeSteps; ++t)
  {
    for (unsigned int i = 0; i < numberOfVertices; ++i)
    {
      mitk::Point3D p;
      p[0] = 10.0 * t + i;
      p[1] = -1.0 * i;
      p[2] = 2.0 * t;

      // Every third vertex is a control point, so a flag that is lost or read
      // under a name nobody writes does not pass unnoticed.
      contour->AddVertex(p, 0 == i % 3, t);
    }

    // Only every other time step is closed, so a flag that is taken from the
    // first step for all of them does not pass either.
    if (0 == t % 2)
      contour->Close(t);
  }

  TestContourModel(contour.GetPointer(), "/contour4D.cnt");
}

int mitkContourModelIOTest(int /*argc*/, char * /*argv*/ [])
{
  MITK_TEST_BEGIN("mitkContourModelIOTest")

  TestContourModelIO_OneTimeStep();
  TestContourModelIO_MultipleTimeSteps();

  MITK_TEST_END()
}
