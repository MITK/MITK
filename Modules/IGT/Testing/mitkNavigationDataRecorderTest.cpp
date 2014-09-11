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
#include <mitkNavigationDataSequentialPlayer.h>
#include <mitkNavigationDataSet.h>
#include <mitkStandardFileLocations.h>
#include <mitkTestingMacros.h>
#include <mitkTestFixture.h>

#include <mitkNavigationDataReaderXML.h>
#include <mitkNavigationDataSetWriterXML.h>

//for exceptions
#include "mitkIGTException.h"
#include "mitkIGTIOException.h"

class mitkNavigationDataRecorderTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkNavigationDataRecorderTestSuite);
  MITK_TEST(TestRecording);
  MITK_TEST(TestStopRecording);
  MITK_TEST(TestLimiting);

  CPPUNIT_TEST_SUITE_END();

private:
  mitk::NavigationDataSet::Pointer m_NavigationDataSet;
  mitk::NavigationDataSequentialPlayer::Pointer m_Player;
  mitk::NavigationDataRecorder::Pointer m_Recorder;

public:

  void setUp()
  {
    mitk::NavigationDataReaderXML::Pointer reader = mitk::NavigationDataReaderXML::New();
    std::string path = GetTestDataFilePath("IGT-Data/RecordedNavigationData.xml");
    m_NavigationDataSet = reader->Read(path);

    m_Player = mitk::NavigationDataSequentialPlayer::New();
    m_Player->SetNavigationDataSet(m_NavigationDataSet);

    m_Recorder = mitk::NavigationDataRecorder::New();
    m_Recorder->SetStandardizeTime(false);

    // connect player to recorder
    m_Recorder->ConnectTo(m_Player);
  }

  void tearDown()
  {
  }

  void TestRecording()
  {
    m_Recorder->StartRecording();
    while (!m_Player->IsAtEnd())
    {
      m_Recorder->Update();
      m_Player->GoToNextSnapshot();
    }

    mitk::NavigationDataSet::Pointer recordedData = m_Recorder->GetNavigationDataSet();

    MITK_TEST_CONDITION_REQUIRED(recordedData->Size() == m_NavigationDataSet->Size(), "Test if recorded Dataset is of equal size as original");
    MITK_TEST_CONDITION_REQUIRED(compareDataSet(recordedData), "Test recorded dataset for equality with reference");
  }

  void TestStopRecording()
  {
    // Aim is to read an xml into a pointset, play that set with a sequentialplayer, record it
    // again, write the result to xml , and compare the output

    m_Recorder->StartRecording();
    int i = 0;
    while (i < 5)
    {
      m_Recorder->Update();
      m_Player->GoToNextSnapshot();
      i++;
    }

    m_Recorder->StopRecording();
    MITK_TEST_CONDITION_REQUIRED(! m_Recorder->GetRecording(), "Test if StopRecording is working, part 1");
    while (i < 5)
    {
      m_Recorder->Update();
      m_Player->GoToNextSnapshot();
      i++;
    }

    MITK_TEST_CONDITION_REQUIRED(m_Recorder->GetNavigationDataSet()->Size() == 5, "Test if StopRecording is working, part 2");
  }

  void TestLimiting()
  {
    // Check if Limiting recording works
    m_Recorder->SetRecordCountLimit(30);
    m_Recorder->StartRecording();
    while (!m_Player->IsAtEnd())
    {
      m_Recorder->Update();
      m_Player->GoToNextSnapshot();
    }

    MITK_TEST_CONDITION_REQUIRED(m_Recorder->GetNavigationDataSet()->Size() == 30, "Test if SetRecordCountLimit works as intended.");
  }

private:

  /*
  * private hepler method that compares the recorded Dataset against the member variable.
  * This is a reasonable test only under the assumption that the Data should be equal from coyping - It does not consider
  * homonymus Quaternions and NO FLOAT ROUNDING ISSUES
  */
  bool compareDataSet(mitk::NavigationDataSet::Pointer recorded)
  {
    for (unsigned int tool = 0; tool < recorded->GetNumberOfTools(); tool++){
      for (unsigned int i = 0; i < recorded->Size(); i++)
      {
        mitk::NavigationData::Pointer ref = m_NavigationDataSet->GetNavigationDataForIndex(i,tool);
        mitk::NavigationData::Pointer rec = recorded->GetNavigationDataForIndex(i,tool);
        if (!(ref->GetOrientation().as_vector() == rec->GetOrientation().as_vector())) {return false;}
        if (!(ref->GetPosition().GetVnlVector() == rec->GetPosition().GetVnlVector())) {return false;}
      }
    }
    return true;
  }
};
MITK_TEST_SUITE_REGISTRATION(mitkNavigationDataRecorder)