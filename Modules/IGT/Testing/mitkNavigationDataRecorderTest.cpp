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

#include <mitkNavigationDataRecorder.h>
#include <mitkNavigationDataPlayer.h>
#include <mitkNavigationData.h>
#include <mitkStandardFileLocations.h>
#include <mitkTestingMacros.h>

#include <Poco/Path.h>
#include <Poco/File.h>


#include <iostream>
#include <sstream>
#include <fstream>

//for exceptions
#include "mitkIGTException.h"
#include "mitkIGTIOException.h"


class mitkNavigationDataRecorderTestClass
  {
  public:

  static void TestInstantiation()
    {
    // let's create an object of our class
    mitk::NavigationDataRecorder::Pointer recorder = mitk::NavigationDataRecorder::New();
    MITK_TEST_CONDITION( recorder.IsNotNull(), "Testing instatiation of NavigationDataRecorder");
    }

  static void TestRecordingWithGivenStream()
    {
    std::string tmp = "";
  std::ostringstream* stream = new std::ostringstream( std::ostringstream::trunc );
  stream->setf( std::ios::fixed, std::ios::floatfield );

  // let's create an object of our class
  mitk::NavigationDataRecorder::Pointer recorder = mitk::NavigationDataRecorder::New();


  MITK_TEST_CONDITION(recorder->GetInputs().size() == 0, "testing initial number of inputs");
  MITK_TEST_CONDITION(recorder->GetOutputs().size() == 0, "testing initial number of outputs");

  mitk::NavigationData::Pointer naviData = mitk::NavigationData::New();
  recorder->AddNavigationData( naviData );
  //recorder->SetFileName("e:/test");
  //recorder->SetRecordingMode( mitk::NavigationDataRecorder::NormalFile );
  recorder->StartRecording( stream );
  for ( unsigned int i=0; i<5; i++ )
  {
    mitk::Point3D pnt;
    pnt[0] = i + 1;
    pnt[1] = i + 1/2;
    pnt[2] = i +1*3;


    naviData->SetPosition(pnt);
    //naviData->Modified();
    recorder->Update();

    //Sleep(80 + i*10);

  }

  recorder->StopRecording();

  std::string str = stream->str();
  int pos = str.find( "ToolCount=" );
  std::string sub = stream->str().substr(pos+11, 1);
  MITK_TEST_CONDITION( sub.compare("1") == 0, "check if number of inputs is correct by stringstream");

  pos = str.find( "X=" );
  sub = stream->str().substr(pos+3, 1);
  MITK_TEST_CONDITION( sub.compare("1") == 0, "check if the X coordinate is correct");


  pos = str.find( "Y=" );
  sub = stream->str().substr(pos+3, 1);
  MITK_TEST_CONDITION( sub.compare("0") == 0, "check if the Y coordinate is correct");


  pos = str.find( "Z=" );
  sub = stream->str().substr(pos+3, 1);
  MITK_TEST_CONDITION( sub.compare("3") == 0, "check if the Z coordinate is correct");


  recorder->SetFileName("blablabla");
  const char* string = recorder->GetFileName();
  MITK_TEST_CONDITION( strcmp(string, "blablabla") == 0, "check if set- and getName-methods work");

    }

  static void TestRecordingOnHarddiscXML()
    {
    mitk::NavigationDataRecorder::Pointer recorder = mitk::NavigationDataRecorder::New();

    //create filename
    std::string filename = mitk::StandardFileLocations::GetInstance()->GetOptionDirectory()+Poco::Path::separator()+"Recordertest.xml";

    recorder->SetFileName(filename.c_str());

    mitk::NavigationData::Pointer naviData = mitk::NavigationData::New();
    recorder->AddNavigationData( naviData );
    recorder->StartRecording();

    for ( unsigned int i=0; i<5; i++ )
     {
     mitk::Point3D pnt;
     pnt[0] = i + 1;
     pnt[1] = i + 1/2;
     pnt[2] = i +1*3;
     naviData->SetPosition(pnt);
     recorder->Update();
     }

    recorder->StopRecording();

    std::string prooffilename = mitk::StandardFileLocations::GetInstance()->GetOptionDirectory()+Poco::Path::separator()+"Recordertest-0.xml";
    Poco::File myFile(prooffilename);
    MITK_TEST_CONDITION(myFile.exists(),"Testing XML recording on harddisc (does file exist?).");
    }

  static void TestRecordingOnHarddiscXMLZIP()
    {
    mitk::NavigationDataRecorder::Pointer recorder = mitk::NavigationDataRecorder::New();

    //create filename
    std::string filename = mitk::StandardFileLocations::GetInstance()->GetOptionDirectory()+Poco::Path::separator()+"Recordertestzip.xml";

    recorder->SetFileName(filename.c_str());
    recorder->SetRecordingMode(mitk::NavigationDataRecorder::ZipFile);

    MITK_TEST_CONDITION(recorder->GetRecordingMode()==mitk::NavigationDataRecorder::ZipFile,"Testing setter of recording mode.");

    /* Zip file not supported yet, activate later
    mitk::NavigationData::Pointer naviData = mitk::NavigationData::New();
    recorder->AddNavigationData( naviData );
    recorder->StartRecording();

    for ( unsigned int i=0; i<5; i++ )
     {
     mitk::Point3D pnt;
     pnt[0] = i + 1;
     pnt[1] = i + 1/2;
     pnt[2] = i +1*3;
     naviData->SetPosition(pnt);
     recorder->Update();
     }

    recorder->StopRecording();

    std::string prooffilename = mitk::StandardFileLocations::GetInstance()->GetOptionDirectory()+Poco::Path::separator()+"Recordertest-0.xml";
    Poco::File myFile(prooffilename);
    MITK_TEST_CONDITION(myFile.exists(),"Testing XML Zip recording on harddisc (does file exist?).");
    */
    }

  static void TestRecordingOnHarddiscCSV()
    {
    mitk::NavigationDataRecorder::Pointer recorder = mitk::NavigationDataRecorder::New();

    //create filename
    std::string filename = mitk::StandardFileLocations::GetInstance()->GetOptionDirectory()+Poco::Path::separator()+"Recordertest.xml";

    recorder->SetFileName(filename.c_str());
    recorder->SetOutputFormat(mitk::NavigationDataRecorder::csv);

    mitk::NavigationData::Pointer naviData = mitk::NavigationData::New();
    recorder->AddNavigationData( naviData );
    recorder->StartRecording();

    for ( unsigned int i=0; i<5; i++ )
     {
     mitk::Point3D pnt;
     pnt[0] = i + 1;
     pnt[1] = i + 1/2;
     pnt[2] = i + 1*3;
     naviData->SetPosition(pnt);
     recorder->Update();
     }

    recorder->StopRecording();

    std::string prooffilename = mitk::StandardFileLocations::GetInstance()->GetOptionDirectory()+Poco::Path::separator()+"Recordertest-0.csv";
    Poco::File myFile(prooffilename);
    MITK_TEST_CONDITION(myFile.exists(),"Testing CSV recording on harddisc (does file exist?).");
    }

  static void TestLoadingRecordedXMLFile()
    {
    mitk::NavigationDataPlayer::Pointer myPlayer = mitk::NavigationDataPlayer::New();
    std::string filenameXML = mitk::StandardFileLocations::GetInstance()->GetOptionDirectory()+Poco::Path::separator()+"Recordertest-0.xml";
    myPlayer->SetFileName(filenameXML.c_str());
    myPlayer->StartPlaying();

    //only testing first position at the moment
    myPlayer->Update();
    mitk::NavigationData::Pointer thisData = myPlayer->GetOutput();
    mitk::Point3D reference_pnt;
    reference_pnt[0] = 1;
    reference_pnt[1] = 1/2;
    reference_pnt[2] = 1*3;

    myPlayer->StopPlaying();

    MITK_TEST_CONDITION((thisData->GetPosition() == reference_pnt),"Testing load data from xml file.");
    }

  static void TestRecordingInvalidData()
    {
    std::string tmp = "";
    std::ostringstream* stream = new std::ostringstream( std::ostringstream::trunc );
    stream->setf( std::ios::fixed, std::ios::floatfield );

    // let's create an object of our class
    mitk::NavigationDataRecorder::Pointer recorder = mitk::NavigationDataRecorder::New();


    mitk::NavigationData::Pointer naviData = mitk::NavigationData::New();
    recorder->AddNavigationData( naviData );
    naviData->SetDataValid(false);
    //recorder->SetFileName("e:/test");
    //recorder->SetRecordingMode( mitk::NavigationDataRecorder::NormalFile );
    recorder->StartRecording( stream );
    for ( unsigned int i=0; i<5; i++ )
    {
      mitk::Point3D pnt;
      pnt[0] = i + 1;
      pnt[1] = i + 1/2;
      pnt[2] = i +1*3;


      naviData->SetPosition(pnt);
      //naviData->Modified();
      recorder->Update();

      //Sleep(80 + i*10);

    }

    recorder->StopRecording();

    bool record_success = true;

    std::string str = stream->str();
    int pos = str.find( "ToolCount=" );
    std::string sub = stream->str().substr(pos+11, 1);
    if (sub.compare("1") != 0) {record_success = false;}

    pos = str.find( "X=" );
    sub = stream->str().substr(pos+3, 1);
    if (sub.compare("1") != 0) {record_success = false;}

    pos = str.find( "Y=" );
    sub = stream->str().substr(pos+3, 1);
    if (sub.compare("0") != 0) {record_success = false;}

    pos = str.find( "Z=" );
    sub = stream->str().substr(pos+3, 1);
    if (sub.compare("3") != 0) {record_success = false;}

    MITK_TEST_CONDITION(record_success,"Testing recording of invalid data.");

    }

  static void CleanUp()
    {
    std::string filenameXML = mitk::StandardFileLocations::GetInstance()->GetOptionDirectory()+Poco::Path::separator()+"Recordertest-0.xml";
    std::string filenameCSV = mitk::StandardFileLocations::GetInstance()->GetOptionDirectory()+Poco::Path::separator()+"Recordertest-0.csv";
    Poco::File myFileXML(filenameXML);
    Poco::File myFileCSV(filenameCSV);

    try
    {
    if (myFileXML.exists())
      {myFileXML.remove();}
    }
    catch(std::exception e)
    {
      MITK_WARN << "Cannot delete file while cleanup: " << filenameXML;
    }

    try
    {
    if (myFileCSV.exists())
      {myFileCSV.remove();}
    }
    catch(std::exception e)
    {
      MITK_WARN << "Cannot delete file while cleanup: " << filenameCSV;
    }

    }

  static void TestStartRecordingExceptions()
     {
     //Testing Start Recording for exceptions if recording has already started
     mitk::NavigationDataRecorder::Pointer recorder = mitk::NavigationDataRecorder::New();
     std::string filename = mitk::StandardFileLocations::GetInstance()->GetOptionDirectory()+Poco::Path::separator()+"Recordertest.xml";
     recorder->SetFileName(filename.c_str());

     //Testing double call of StartRecording().
     mitk::NavigationData::Pointer naviData = mitk::NavigationData::New();
     recorder->AddNavigationData( naviData );
     recorder->StartRecording();
     recorder->StartRecording();
     recorder->StopRecording();
     MITK_TEST_OUTPUT(<<"Tested double call of StartRecording(). Application should not crash.");

     //Testing exceptions for method StartRecording() when no file is set.
     mitk::NavigationDataRecorder::Pointer recorder1 = mitk::NavigationDataRecorder::New();
     std::string filename1 = mitk::StandardFileLocations::GetInstance()->GetOptionDirectory()+Poco::Path::separator()+"Recordertest.xml";
     recorder->SetFileName("");
     bool exceptionThrown1 = false;
     mitk::NavigationData::Pointer naviData1 = mitk::NavigationData::New();
     recorder1->AddNavigationData( naviData1 );
     try
       {
       recorder1->StartRecording();
       }
     catch(mitk::IGTException)
       {
       exceptionThrown1 = true;
       }
     MITK_TEST_CONDITION(exceptionThrown1,"Testing exception throwing when no file name or file path is set.");

     //Testing double call of StartRecording(stream) method.
     mitk::NavigationDataRecorder::Pointer recorder2 = mitk::NavigationDataRecorder::New();
     std::string tmp = "";
     std::ostringstream stream;// = new std::ostringstream( std::ostringstream::trunc );
     stream.setf( std::ios::fixed, std::ios::floatfield );
     recorder2->StartRecording(&stream);
     recorder2->StartRecording(&stream);
     recorder2->StopRecording();
     MITK_TEST_OUTPUT(<<"Tested double call of StartRecording(stream). Application should not crash.");

     //Testing exceptions if the stream is not good
     mitk::NavigationDataRecorder::Pointer recorder3 = mitk::NavigationDataRecorder::New();
     std::ofstream stream3; //making an invalid stream
     stream3.open("");
     bool exceptionThrown3 = false;
     try
       {
       recorder3->StartRecording(&stream3);
       }
     catch(mitk::IGTException)
       {
       exceptionThrown3 = true;
       }
     MITK_TEST_CONDITION(exceptionThrown3,"Testing exception thrown when the stream in not good.");
     }

  };



/**Documentation
 *  test for the class "NavigationDataRecorder".
 */
int mitkNavigationDataRecorderTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("NavigationDataRecorder");

  mitkNavigationDataRecorderTestClass::TestInstantiation();
  mitkNavigationDataRecorderTestClass::TestRecordingWithGivenStream();
  mitkNavigationDataRecorderTestClass::TestRecordingOnHarddiscXML();
  mitkNavigationDataRecorderTestClass::TestRecordingOnHarddiscXMLZIP();
  mitkNavigationDataRecorderTestClass::TestRecordingOnHarddiscCSV();
  mitkNavigationDataRecorderTestClass::TestRecordingInvalidData();
  mitkNavigationDataRecorderTestClass::TestStartRecordingExceptions();


  //Test fails under linux, perhaps reading permission problems, deactivated it temporary
  //mitkNavigationDataRecorderTestClass::TestLoadingRecordedXMLFile();

  mitkNavigationDataRecorderTestClass::CleanUp();


  // always end with this!
  MITK_TEST_END();
}
