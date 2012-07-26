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
#include <mitkRenderingManager.h>

static void TestSaveContourModel()
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

  mitk::ContourModelWriter::Pointer writer = mitk::ContourModelWriter::New();

  writer->SetInput(contour);
  std::string filename = std::string( MITK_TEST_OUTPUT_DIR ) + "/contour.cnt";

  writer->SetFileName(filename);
  writer->Update();

  MITK_TEST_CONDITION(true,"");
}


int mitkContourModelWriterTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("mitkContourModelWriterTest")

    TestSaveContourModel();

  MITK_TEST_END()
}