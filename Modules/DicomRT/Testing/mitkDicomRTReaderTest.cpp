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
#include "mitkDicomRTReader.h"
#include <iostream>


int mitkDicomRTReaderTest(int  argc , char* argv[])
{
  // always start with this!
  MITK_TEST_BEGIN("DicomRTReader")

  mitk::DicomRTReader::Pointer _DicomRTReader = mitk::DicomRTReader::New();

  //ContourModelVector emptyItem;
  //ContourModelVector result = _DicomRTReader->ReadDicomFile(argv[1]);

  DcmFileFormat file;
  OFCondition outp = file.loadFile(argv[1], EXS_Unknown);
  DcmDataset *dataset = file.getDataset();

  std::cout << "Filename:" << argv[1] << "\n\n";

  ContourModelVector result = _DicomRTReader->ReadStructureSet(dataset);

  mitk::ContourModel::Pointer real = mitk::ContourModel::New();
  real = result.front();

  mitk::Point3D point;
  point[0] = 30.9;
  point[1] = 20.6;
  point[2] = 11.7;

  mitk::Point3D samePoint;
  samePoint[0] = -0.3823089599609;
  samePoint[1] = -0.3836975097656;
  samePoint[2] = 2.5;

  mitk::Point3D b;
  b[0] = -0.3823089599609;
  b[1] = -0.3836975097656;
  b[2] = 2.5;

  mitk::ContourModel::Pointer created = mitk::ContourModel::New();
  created->AddVertex(point);
  created->Close();

  mitk::ContourModel::Pointer same = mitk::ContourModel::New();
  same->AddVertex(samePoint);
  same->Close();

  mitk::ContourModel::Pointer a1 = mitk::ContourModel::New();
  mitk::ContourModel::Pointer a2 = mitk::ContourModel::New();

  a1->AddVertex(samePoint);
  a2->AddVertex(samePoint);

  a1->Close();
  a2->Close();

  MITK_TEST_CONDITION_REQUIRED( !real->IsEmpty() , "Tests if the first ContouModelObject from ReadDicomFile/Struct isnt empty" );
  MITK_TEST_CONDITION_REQUIRED( real != created , "Tests if created ContourModel is different to frist ContourModel" );
  MITK_TEST_CONDITION_REQUIRED( same != real , "Tests if the ContourModel looks like in the DcmDump Output" );
  MITK_TEST_CONDITION_REQUIRED( samePoint == b , "Tests the two Points with same coords" );
  MITK_TEST_CONDITION_REQUIRED( _DicomRTReader->Equals(a1,a2) , "Tests the two generated ContourModels" );

  std::cout << "\n";
  // always end with this!
  MITK_TEST_END()
}
