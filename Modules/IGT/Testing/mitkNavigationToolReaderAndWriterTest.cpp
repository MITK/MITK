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
#include "mitkDataTreeNode.h"
#include "mitkSurface.h"
#include "mitkStandaloneDataStorage.h"
#include "mitkDataStorage.h"
#include "mitkNavigationToolReader.h"

#include <sstream>


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
    //create a NavigationTool which we can write on the harddisc
    std::string toolFileName = mitk::StandardFileLocations::GetInstance()->FindFile("ClaronTool", "Modules/IGT/Testing/Data");
    MITK_TEST_CONDITION(toolFileName.empty() == false, "Check if tool calibration file exists");
    mitk::NavigationTool::Pointer myNavigationTool = mitk::NavigationTool::New();
    myNavigationTool->SetCalibrationFile(toolFileName);

    mitk::DataTreeNode::Pointer myNode = mitk::DataTreeNode::New();
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
    
    myNavigationTool->SetDataTreeNode(myNode);
    myNavigationTool->SetIdentifier("ClaronTool#1");
    myNavigationTool->SetSerialNumber("0815");
    myNavigationTool->SetTrackingDeviceType(mitk::ClaronMicron);
    myNavigationTool->SetType(mitk::NavigationTool::Fiducial);

    //now create a writer and write it to the harddisc
    mitk::NavigationToolWriter::Pointer myWriter = mitk::NavigationToolWriter::New();
    std::string filename = mitk::StandardFileLocations::GetInstance()->GetOptionDirectory()+Poco::Path::separator()+".."+Poco::Path::separator()+"TestTool.tool";
    
    MITK_TEST_OUTPUT(<<"---- Testing navigation tool writer ----");
    bool test = myWriter->DoWrite(filename,myNavigationTool);
    MITK_TEST_CONDITION_REQUIRED(test,"OK");
    }

    static void TestRead()
    {
    mitk::DataStorage::Pointer testStorage = mitk::StandaloneDataStorage::New();
    mitk::NavigationToolReader::Pointer myReader = mitk::NavigationToolReader::New(testStorage);
    mitk::NavigationTool::Pointer readTool = myReader->DoRead(mitk::StandardFileLocations::GetInstance()->GetOptionDirectory()+Poco::Path::separator()+".."+Poco::Path::separator()+"TestTool.tool");
    MITK_TEST_OUTPUT(<<"---- Testing navigation tool reader ----");
    MITK_TEST_CONDITION_REQUIRED(readTool->GetDataTreeNode() == testStorage->GetNamedNode(readTool->GetDataTreeNode()->GetName()),"Test if tool was added to storage...");
    MITK_TEST_CONDITION_REQUIRED(readTool->GetDataTreeNode()->GetData()==testSurface,"Test if surface was restored correctly ...");
    //MITK_TEST_CONDITION_REQUIRED();
    }

    static void CleanUp()
    {
    std::remove((mitk::StandardFileLocations::GetInstance()->GetOptionDirectory()+Poco::Path::separator()+".."+Poco::Path::separator()+"TestTool.tool").c_str());
    }

  };

mitk::Surface::Pointer mitkNavigationToolReaderAndWriterTestClass::testSurface = NULL;

/** This function is testing the TrackingVolume class. */
int mitkNavigationToolReaderAndWriterTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("NavigationToolWriter")

  mitkNavigationToolReaderAndWriterTestClass::TestInstantiation();
  mitkNavigationToolReaderAndWriterTestClass::TestWrite();
  //mitkNavigationToolReaderAndWriterTestClass::TestRead();
  mitkNavigationToolReaderAndWriterTestClass::CleanUp();


  MITK_TEST_END()
}


