/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2008-02-25 17:27:17 +0100 (Mo, 25 Feb 2008) $
Version:   $Revision: 7837 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

//Poco headers
#include "Poco/Path.h"

//mitk headers
#include "mitkNavigationToolWriter.h"
#include "mitkCommon.h"
#include "mitkTestingMacros.h"
#include "mitkStandardFileLocations.h"
#include "mitkNavigationTool.h"
#include "mitkSTLFileReader.h"
#include "mitkBaseData.h"
#include "mitkDataNode.h"
#include "mitkSurface.h"
#include "mitkStandaloneDataStorage.h"
#include "mitkDataStorage.h"
#include "mitkNavigationToolReader.h"

#include <sstream>
#include <fstream>


class mitkNavigationToolReaderAndWriterTestClass
  {
  private:
    
    static mitk::Surface::Pointer testSurface;

  public:

    static void TestInstantiation()
    {
    // let's create an object of our class
    mitk::NavigationToolWriter::Pointer myWriter = mitk::NavigationToolWriter::New();
    MITK_TEST_CONDITION_REQUIRED(myWriter.IsNotNull(),"Testing instantiation")
    }

    static void TestWrite()
    {
    //testcase with first test tool: a claron tool

    //create a NavigationTool which we can write on the harddisc
    std::string toolFileName = mitk::StandardFileLocations::GetInstance()->FindFile("ClaronTool", "Modules/IGT/Testing/Data");
    MITK_TEST_CONDITION(toolFileName.empty() == false, "Check if tool calibration of claron tool file exists");
    mitk::NavigationTool::Pointer myNavigationTool = mitk::NavigationTool::New();
    myNavigationTool->SetCalibrationFile(toolFileName);

    mitk::DataNode::Pointer myNode = mitk::DataNode::New();
    myNode->SetName("ClaronTool");
      
    //load an stl File
    mitk::STLFileReader::Pointer stlReader = mitk::STLFileReader::New();
    try
      {
      stlReader->SetFileName( mitk::StandardFileLocations::GetInstance()->FindFile("ClaronTool.stl", "Testing/Data/").c_str() );
      stlReader->Update();
      }
    catch (...)
      {
      MITK_TEST_FAILED_MSG(<<"Cannot read stl file.");
      }

    if ( stlReader->GetOutput() == NULL )
      {
      MITK_TEST_FAILED_MSG(<<"Cannot read stl file.");
      }
    else
      {
      testSurface = stlReader->GetOutput();
      myNode->SetData(testSurface);    
      }
    
    myNavigationTool->SetDataNode(myNode);
    myNavigationTool->SetIdentifier("ClaronTool#1");
    myNavigationTool->SetSerialNumber("0815");
    myNavigationTool->SetTrackingDeviceType(mitk::ClaronMicron);
    myNavigationTool->SetType(mitk::NavigationTool::Fiducial);

    //now create a writer and write it to the harddisc
    mitk::NavigationToolWriter::Pointer myWriter = mitk::NavigationToolWriter::New();
    std::string filename = "TestTool.tool";

    MITK_TEST_OUTPUT(<<"---- Testing navigation tool writer with first test tool (claron tool) ----");
    bool test = myWriter->DoWrite(filename,myNavigationTool);
    MITK_TEST_CONDITION_REQUIRED(test,"OK");
    }

    static void TestRead()
    {
    mitk::NavigationToolReader::Pointer myReader = mitk::NavigationToolReader::New();
    mitk::NavigationTool::Pointer readTool = myReader->DoRead("TestTool.tool");
    MITK_TEST_OUTPUT(<<"---- Testing navigation tool reader with first test tool (claron tool) ----");

    //Test if the surfaces do have the same number of vertexes (it would be better to test for real equality of the surfaces!)
    MITK_TEST_CONDITION_REQUIRED(dynamic_cast<mitk::Surface*>(readTool->GetDataNode()->GetData())->GetSizeOfPolyDataSeries()==testSurface->GetSizeOfPolyDataSeries(),"Test if surface was restored correctly ...");

    MITK_TEST_CONDITION_REQUIRED(readTool->GetType()==mitk::NavigationTool::Fiducial,"Testing Tool Type");

    MITK_TEST_CONDITION_REQUIRED(readTool->GetTrackingDeviceType()==mitk::ClaronMicron,"Testing Tracking Device Type");

    MITK_TEST_CONDITION_REQUIRED(readTool->GetSerialNumber()=="0815","Testing Serial Number");

    std::ifstream TestFile(readTool->GetCalibrationFile().c_str());
    MITK_TEST_CONDITION_REQUIRED(TestFile,"Testing If Calibration File Exists");
 
    }

    static void TestWrite2()
    {
    //testcase with second test tool: an aurora tool
    //create a NavigationTool which we can write on the harddisc
    mitk::NavigationTool::Pointer myNavigationTool = mitk::NavigationTool::New();
   
    mitk::DataNode::Pointer myNode = mitk::DataNode::New();
    myNode->SetName("AuroraTool");
      
    //load an stl File
    mitk::STLFileReader::Pointer stlReader = mitk::STLFileReader::New();
    try
      {
      stlReader->SetFileName( mitk::StandardFileLocations::GetInstance()->FindFile("EMTool.stl", "Testing/Data/").c_str() );
      stlReader->Update();
      }
    catch (...)
      {
      MITK_TEST_FAILED_MSG(<<"Cannot read stl file.");
      }

    if ( stlReader->GetOutput() == NULL )
      {
      MITK_TEST_FAILED_MSG(<<"Cannot read stl file.");
      }
    else
      {
      testSurface = stlReader->GetOutput();
      myNode->SetData(testSurface);    
      }
    
    myNavigationTool->SetDataNode(myNode);
    myNavigationTool->SetIdentifier("AuroraTool#1");
    myNavigationTool->SetSerialNumber("0816");
    myNavigationTool->SetTrackingDeviceType(mitk::NDIAurora);
    myNavigationTool->SetType(mitk::NavigationTool::Instrument);

    //now create a writer and write it to the harddisc
    mitk::NavigationToolWriter::Pointer myWriter = mitk::NavigationToolWriter::New();
    std::string filename = "TestTool2.tool";

    MITK_TEST_OUTPUT(<<"---- Testing navigation tool writer with second tool (aurora tool) ----");
    bool test = myWriter->DoWrite(filename,myNavigationTool);
    MITK_TEST_CONDITION_REQUIRED(test,"OK");
    }

    static void TestRead2()
    {
    mitk::NavigationToolReader::Pointer myReader = mitk::NavigationToolReader::New();
    mitk::NavigationTool::Pointer readTool = myReader->DoRead("TestTool2.tool");
    MITK_TEST_OUTPUT(<<"---- Testing navigation tool reader  with second tool (aurora tool) ----");

    //Test if the surfaces do have the same number of vertexes (it would be better to test for real equality of the surfaces!)
    MITK_TEST_CONDITION_REQUIRED(dynamic_cast<mitk::Surface*>(readTool->GetDataNode()->GetData())->GetSizeOfPolyDataSeries()==testSurface->GetSizeOfPolyDataSeries(),"Test if surface was restored correctly ...");

    //Test if the tool type is the same
    MITK_TEST_CONDITION_REQUIRED(readTool->GetType()==mitk::NavigationTool::Instrument,"Testing Tool Type");

    MITK_TEST_CONDITION_REQUIRED(readTool->GetTrackingDeviceType()==mitk::NDIAurora,"Testing Tracking Device Type");

    MITK_TEST_CONDITION_REQUIRED(readTool->GetSerialNumber()=="0816","Testing Serial Number");

    MITK_TEST_CONDITION_REQUIRED(readTool->GetCalibrationFile()=="none","Testing Calibration File");
    
    }

    static void CleanUp()
    {
      std::remove("TestTool.tool");
      std::remove("TestTool2.tool");
    }

    static void TestReadInvalidData()
    {
    mitk::NavigationToolReader::Pointer myReader = mitk::NavigationToolReader::New();
    mitk::NavigationTool::Pointer readTool = myReader->DoRead("invalidTool");

    MITK_TEST_CONDITION_REQUIRED(readTool.IsNull(), "Testing return value if filename is invalid");
    MITK_TEST_CONDITION_REQUIRED(myReader->GetErrorMessage() == "Cannot open 'invalidTool' for reading", "Testing error message in this case");
    }

    static void TestWriteInvalidData()
    {
    mitk::NavigationTool::Pointer myNavigationTool = mitk::NavigationTool::New();
    myNavigationTool->SetIdentifier("ClaronTool#1");
    myNavigationTool->SetSerialNumber("0815");
    myNavigationTool->SetTrackingDeviceType(mitk::ClaronMicron);
    myNavigationTool->SetType(mitk::NavigationTool::Fiducial);

    //now create a writer and write it to the harddisc
    mitk::NavigationToolWriter::Pointer myWriter = mitk::NavigationToolWriter::New();
    std::string filename = "NH:/sfdsfsdsf.&%%%";
    
    MITK_TEST_OUTPUT(<<"---- Testing write invalid file ----");
    bool test = myWriter->DoWrite(filename,myNavigationTool);
    MITK_TEST_CONDITION_REQUIRED(!test,"testing write");
    MITK_TEST_CONDITION_REQUIRED(myWriter->GetErrorMessage() == "Could not open a zip file for writing: 'NH:/sfdsfsdsf.&%%%'","testing error message"); 
    }

  };

mitk::Surface::Pointer mitkNavigationToolReaderAndWriterTestClass::testSurface = NULL;

/** This function is testing the TrackingVolume class. */
int mitkNavigationToolReaderAndWriterTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("NavigationToolWriter")

  mitkNavigationToolReaderAndWriterTestClass::TestInstantiation();
  mitkNavigationToolReaderAndWriterTestClass::TestWrite();
  mitkNavigationToolReaderAndWriterTestClass::TestRead();
  mitkNavigationToolReaderAndWriterTestClass::TestWrite2();
  mitkNavigationToolReaderAndWriterTestClass::TestRead2();
  mitkNavigationToolReaderAndWriterTestClass::TestReadInvalidData();
  mitkNavigationToolReaderAndWriterTestClass::TestWriteInvalidData();
  mitkNavigationToolReaderAndWriterTestClass::CleanUp();


  MITK_TEST_END()
}


