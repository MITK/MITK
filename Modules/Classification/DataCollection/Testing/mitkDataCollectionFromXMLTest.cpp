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
#pragma warning (disable : 4996)

#include "mitkTestingMacros.h"
#include "mitkDataCollection.h"
#include "mitkCollectionReader.h"
#include "mitkCollectionWriter.h"
#include "mitkIOUtil.h"

#include <vtkSmartPointer.h>

int mitkDataCollectionFromXMLTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("DataCollectionFromXML");

  if(argc <= 1)
  {
    MITK_ERROR << "Provide XML File to parse.";
    return -1;
  }


  std::vector<std::string> list;
  std::vector<std::string> refSuffix;
  std::vector<std::string> seriesNames;
  list.push_back("_T2_reg.nrrd");
  list.push_back("_T1_reg.nrrd");
  //list.push_back("_gtv_reg.nrrd");
  list.push_back("_reg.dwi");

  refSuffix.push_back("_T2.nrrd");
  refSuffix.push_back("_T1.nrrd");
  refSuffix.push_back("_DWI.dwi");
  refSuffix.push_back("_GTV.nrrd");
  //list.push_back("_ne_reg.nrrd");
  //list.push_back("_ce_reg.nrrd");

  seriesNames.push_back("T2");
  seriesNames.push_back("T1");
  //seriesNames.push_back("NE");
  //seriesNames.push_back("CE");
  seriesNames.push_back("DWI");
  seriesNames.push_back("GTV");

  // GTV has to go last -> TARGET
  mitk::CollectionWriter::SingleFolderToXml(argv[1],argv[2], refSuffix,seriesNames,true,0,0);

return 0;
mitk::CollectionWriter::FolderToXml("/home/cweber/SSD-Data/Features/Frychel_Jadwiga", "2010-10-07", "/tmp/", list,seriesNames);

  mitk::CollectionReader colReader;
  mitk::DataCollection::Pointer collection = colReader.LoadCollection(argv[1]);



  MITK_TEST_CONDITION_REQUIRED(
        collection->Size() == 2
        , "01) Size of Container is " << collection->Size() << " should be 2" );


  mitk::DataCollection* patientCol = dynamic_cast<mitk::DataCollection*> (collection->GetData("Patient2").GetPointer());

  MITK_TEST_CONDITION_REQUIRED(
        patientCol != NULL
      , "02) Check if Collection: Patient2 is found" );


  MITK_TEST_CONDITION_REQUIRED(
        patientCol->GetData("TimeStep0").IsNotNull()
        , "03) Check if Patient2 contains TimeStep0" );


  mitk::DataCollection* timeStep = dynamic_cast<mitk::DataCollection*>  (patientCol->GetData("TimeStep0").GetPointer());

  mitk::Image* image = dynamic_cast<mitk::Image*> (timeStep->GetData("T2").GetPointer());


  MITK_TEST_CONDITION_REQUIRED(
        image != NULL
      , "04) Check if T2 image can be extracted");


  // Try saving image to disk to ensure it is a valid object
  mitk::IOUtil::SaveImage(image, "/tmp/dump.nrrd");


  std::vector<std::string> colIds;
  colIds.push_back("patient2ID");
  colReader.AddSubColIds(colIds);

  collection = colReader.LoadCollection(argv[1]);

  MITK_TEST_CONDITION_REQUIRED(
        collection->Size() == 1
        , "05) Partial Read: Size of Container is " << collection->Size() << " should be 1" );


  colReader.ClearSubColIds();

  colIds.push_back("oma");
  colReader.AddSubColIds(colIds);

  collection = colReader.LoadCollection(argv[1]);

  MITK_TEST_CONDITION_REQUIRED(
        collection->Size() == 2
        , "06) Partial Read: Size of Container is " << collection->Size() << " should be 2 again" );

  std::vector<std::string> filter;
  filter.push_back("GTV");
  mitk::CollectionWriter::ExportCollectionToFolder(collection, "/tmp/test.xml", filter);

  MITK_TEST_END();
}
