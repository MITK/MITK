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
#include <mitkTestingMacros.h>
#include "mitkTestingConfig.h"

#include <mitkContourModelWriter.h>
#include <iostream>
#include <fstream>
#include <locale>

#include <mitkIOUtil.h>



static void TestContourModel(mitk::ContourModel* contour, std::string fileName)
{
  std::string filename = std::string( MITK_TEST_OUTPUT_DIR ) + fileName;

  mitk::IOUtil::Save(contour, filename);

  std::vector<itk::SmartPointer<mitk::BaseData> > readerOutput = mitk::IOUtil::Load(filename);

  mitk::ContourModel::Pointer contour2 = dynamic_cast<mitk::ContourModel*>(readerOutput.at(0).GetPointer());

  MITK_TEST_CONDITION_REQUIRED(contour2.IsNotNull(),"contour is not null");

  MITK_TEST_CONDITION_REQUIRED(contour->GetNumberOfVertices() == contour2->GetNumberOfVertices(),"contours have the same number of vertices");

  mitk::ContourModel::VertexIterator it = contour2->IteratorBegin();
  mitk::ContourModel::VertexIterator end = contour2->IteratorEnd();

  mitk::ContourModel::VertexIterator it2 = contour2->IteratorBegin();

  bool areEqual = true;

  while(it != end)
  {
    areEqual &= ((*it)->Coordinates == (*it2)->Coordinates);
    it++; it2++;
  }

  MITK_TEST_CONDITION(areEqual, "contours are equal");
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



static void TestContourModelIO_EmptyContourModel()
{
  mitk::ContourModel::Pointer contour = mitk::ContourModel::New();

  TestContourModel(contour.GetPointer(), "/contourEmpty.cnt");
}



int mitkContourModelIOTest(int /*argc*/, char* /*argv*/[])
{
  MITK_TEST_BEGIN("mitkContourModelIOTest")

  TestContourModelIO_OneTimeStep();
  TestContourModelIO_EmptyContourModel();

  MITK_TEST_END()
}
