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
  MITK_TEST(PlayAndRecord);

  CPPUNIT_TEST_SUITE_END();

private:
  mitk::NavigationDataSet::Pointer m_NavigationDataSet;
  mitk::NavigationDataSequentialPlayer::Pointer m_Player;

public:

  void setUp()
  {
    mitk::NavigationDataReaderXML::Pointer reader = mitk::NavigationDataReaderXML::New();
    std::string path = GetTestDataFilePath("IGT-Data/RecordedNavigationData.xml");
    m_NavigationDataSet = reader->Read(path);

    m_Player = mitk::NavigationDataSequentialPlayer::New();
    m_Player->SetNavigationDataSet(m_NavigationDataSet);
  }

  void tearDown()
  {
  }

  void PlayAndRecord()
  {
    // Aim is to read an xml into a pointset, play that set with a sequentialplayer, record it
    // again, write the result to xml , and compare the output

    mitk::NavigationDataRecorder::Pointer recorder = mitk::NavigationDataRecorder::New();
    recorder->SetStandardizeTime(false);

    // connect player to recorder
    recorder->ConnectTo(m_Player);

    recorder->StartRecording();
    while (!m_Player->IsAtEnd())
    {
      recorder->Update();
      m_Player->GoToNextSnapshot();
    }

    recorder->StopRecording();
    MITK_TEST_CONDITION_REQUIRED(! recorder->GetRecording(), "Test if StopRecording is working");
    recorder->ResetRecording();
    MITK_TEST_CONDITION_REQUIRED(recorder->GetNavigationDataSet()->Size() == 0, "Test correct reset of recorder");

    //Reset Player
    //player = mitk::NavigationDataSequentialPlayer::New();
    m_Player->SetNavigationDataSet(m_NavigationDataSet);

    // Check if Limiting recording works
    recorder->SetRecordCountLimit(100);
    recorder->StartRecording();
    while (!m_Player->IsAtEnd())
    {
      recorder->Update();
      m_Player->GoToNextSnapshot();
    }

    MITK_TEST_CONDITION_REQUIRED(recorder->GetNavigationDataSet()->Size() == 100, "Test if SetRecordCountLimit works as intended.");
  }
};
MITK_TEST_SUITE_REGISTRATION(mitkNavigationDataRecorder)