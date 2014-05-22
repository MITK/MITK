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

  char* h = "/home/riecker/DicomRT/DICOMRT_Bilder/DICOM-RT/W_K/RD1.2.826.0.1.3680043.8.176.2013826103830726.377.2260480657.dcm";
  char* i = "/home/riecker/DicomRT/DICOMRT_Bilder/RD.1.2.246.352.71.7.2088656855.452083.20110920153746.dcm";
  char* j = "/home/riecker/DicomRT/DICOMRT_Bilder/2/RTDOSE.2.16.840.1.113669.2.931128.509887832.20120106104806.483734.dcm";

  mitk::ContourModelSet::Pointer emptyItem;
  std::deque<mitk::ContourModelSet::Pointer> result;
  result = _DicomRTReader->ReadDicomFile(h);

//  DcmFileFormat file;
//  OFCondition outp = file.loadFile(argv[1], EXS_Unknown);
//  DcmDataset *dataset = file.getDataset();

//  std::cout << "Filename:" << argv[1] << "\n\n";

//  int x = _DicomRTReader->LoadRTDose(dataset);

//  ContourModelVector result = _DicomRTReader->ReadStructureSet(dataset);


//  mitk::ContourModel::Pointer real = mitk::ContourModel::New();
//  real = result.front();

//  mitk::Point3D point;
//  point[0] = 30.9;
//  point[1] = 20.6;
//  point[2] = 11.7;

//  mitk::Point3D samePoint;
//  samePoint[0] = -0.3823089599609;
//  samePoint[1] = -0.3836975097656;
//  samePoint[2] = 2.5;

//  mitk::Point3D b;
//  b[0] = -0.3823089599609;
//  b[1] = -0.3836975097656;
//  b[2] = 2.5;

//  mitk::ContourModel::Pointer created = mitk::ContourModel::New();
//  created->AddVertex(point);
//  created->Close();

//  mitk::ContourModel::Pointer same = mitk::ContourModel::New();
//  same->AddVertex(samePoint);
//  same->Close();

//  mitk::ContourModel::Pointer a1 = mitk::ContourModel::New();
//  mitk::ContourModel::Pointer a2 = mitk::ContourModel::New();
//  mitk::ContourModel::Pointer a3 = mitk::ContourModel::New();
//  mitk::ContourModel::Pointer a4 = mitk::ContourModel::New();

//  mitk::ContourModel::Pointer a5 = mitk::ContourModel::New();
//  mitk::ContourModel::Pointer a6 = mitk::ContourModel::New();
//  mitk::ContourModel::Pointer a7 = mitk::ContourModel::New();
//  mitk::ContourModel::Pointer empty0 = mitk::ContourModel::New();
//  mitk::ContourModel::Pointer empty1 = mitk::ContourModel::New();

//  a1->AddVertex(samePoint);
//  a2->AddVertex(samePoint);
//  a3->AddVertex(b);
//  a4->AddVertex(point);

//  a5->AddVertex(samePoint);
//  a5->AddVertex(b);
//  a6->AddVertex(b);
//  a6->AddVertex(samePoint);
//  a7->AddVertex(samePoint);
//  a7->AddVertex(point);

//  MITK_TEST_CONDITION_REQUIRED( !real->IsEmpty() , " 01. Tests if the first ContouModelObject from ReadDicomFile/Struct isnt empty -> False" );
//  MITK_TEST_CONDITION_REQUIRED( real != created , " 02. Tests if created ContourModel is different to frist ContourModel -> True" );
//  MITK_TEST_CONDITION_REQUIRED( same != real , " 03. Tests if the ContourModel looks like in the DcmDump Output -> True" );
//  MITK_TEST_CONDITION_REQUIRED( samePoint == b , " 04. Tests the two Points with same coords -> True" );

//  MITK_TEST_CONDITION_REQUIRED( _DicomRTReader->Equals(empty0,empty1) , " 05. Tests two empty ContourModels -> True" );
//  MITK_TEST_CONDITION_REQUIRED( !_DicomRTReader->Equals(a1,empty0) , " 06. Tests Model with 1 Point and empty Model -> False" );
//  MITK_TEST_CONDITION_REQUIRED( !_DicomRTReader->Equals(a1,a4) , " 07. Tests Models with 1 different point -> False" );
//  MITK_TEST_CONDITION_REQUIRED( _DicomRTReader->Equals(a1,a3) , " 08. Tests Models with 1 point and same coords -> True" );
//  MITK_TEST_CONDITION_REQUIRED( !_DicomRTReader->Equals(a5,a1) , " 09. Tests a Model with 2 points and a model with one point, first point is the same -> False" );
//  MITK_TEST_CONDITION_REQUIRED( !_DicomRTReader->Equals(a5,a4) , " 10. Tests a Model with 2 points and a model with one point, first point is different -> False" );
//  MITK_TEST_CONDITION_REQUIRED( _DicomRTReader->Equals(a5,a6) , " 11. Tests two Models with 2 points and same coords -> True" );
//  MITK_TEST_CONDITION_REQUIRED( !_DicomRTReader->Equals(a5,a7) , " 12. Tests two Models with 2 points first is the same second is different -> False" );

//  std::cout << "\n";
  // always end with this!
  MITK_TEST_END()
}
