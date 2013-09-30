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
#include "mitkTestingMacros.h"
#include "mitkContourUtils.h"
#include "mitkContourModel.h"

  void TestContourModel2itkMesh()
  {
    mitk::ContourModel::Pointer contour = mitk::ContourModel::New();
    mitk::ContourUtils::Pointer utils = mitk::ContourUtils::New();

    mitk::Point3D p,p1,p2;

    p[0] = 0;
    p[1] = 0;
    p[2] = 0;
    p1[0] = 1;
    p1[1] = 0;
    p1[2] = 0;
    p2[0] = 1;
    p2[1] = 0;
    p2[2] = 1;

    contour->AddVertex(p);
    contour->AddVertex(p1);
    contour->AddVertex(p2);

    MITK_INFO << "test";
    MITK_TEST_CONDITION(contour->GetNumberOfVertices() > 0, "Add a Vertex, size increased");

    utils->contourModel2itkMesh(contour);
    //MITK_INFO << utils->m_mesh;
    MITK_INFO << "should be done";

  }

  int mitkContourUtilsTest(int argc, char* argv[])
  {
    MITK_TEST_BEGIN("mitkContourUtilsTest")

    TestContourModel2itkMesh();

    MITK_TEST_END()
  }

