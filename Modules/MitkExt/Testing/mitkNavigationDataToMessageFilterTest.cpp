/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-03-09 20:43:00 +0100 (Mo, 09 Mrz 2009) $
Version:   $Revision: 16522 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkNavigationDataToMessageFilter.h"
#include "mitkNavigationData.h"

#include "mitkTestingMacros.h"
#include "mitkVector.h"
#include <iostream>


// Receiver class remembers Messages received.
// Will tell about received Messages when asked.
class MessageReceiverClass
{
  public:

    MessageReceiverClass()
    {
      m_ReceivedData = mitk::NavigationData::New();
      m_MessagesReceived = 0;
    }

    void OnPositionChanged(mitk::NavigationData::PositionType v)
    {
      m_ReceivedData->SetPosition(v);
      ++m_MessagesReceived;
    }
    
    void OnOrientationChanged(mitk::NavigationData::OrientationType v)
    {
      m_ReceivedData->SetOrientation(v);
      ++m_MessagesReceived;
    }

    void OnErrorChanged(mitk::NavigationData::CovarianceMatrixType v)
    {
      m_ReceivedData->SetCovErrorMatrix(v);
      ++m_MessagesReceived;
    }
    
    void OnTimeStampChanged(mitk::NavigationData::TimeStampType v)
    {
      m_ReceivedData->SetTimeStamp(v);
      ++m_MessagesReceived;
    }

    void OnDataValidChanged(bool v)
    {
      m_ReceivedData->SetDataValid(v);
      ++m_MessagesReceived;
    }

    mitk::NavigationData::Pointer m_ReceivedData;
    int m_MessagesReceived;
};

/**Documentation
 *  test for the class "NavigationDataToMessageFilter".
 */
int mitkNavigationDataToMessageFilterTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("NavigationDataToMessageFilter");

  // let's create an object of our class  
  mitk::NavigationDataToMessageFilter::Pointer myFilter = mitk::NavigationDataToMessageFilter::New();
  
  // first test: did this work?
  // using MITK_TEST_CONDITION_REQUIRED makes the test stop after failure, since
  // it makes no sense to continue without an object.
  MITK_TEST_CONDITION_REQUIRED(myFilter.IsNotNull(),"Testing instantiation");

  /* create helper objects: navigation data with position as origin, zero quaternion, zero error and data valid */
  mitk::NavigationData::PositionType initialPos;
  mitk::FillVector3D(initialPos, 1.0, 2.0, 3.0);
  mitk::NavigationData::OrientationType initialOri(1.0, 2.0, 3.0, 4.0);
 
  mitk::NavigationData::Pointer nd1 = mitk::NavigationData::New();
  nd1->SetPosition(initialPos);
  nd1->SetOrientation(initialOri);
  nd1->SetPositionAccuracy(11.111);
  nd1->SetTimeStamp(64.46);
  nd1->SetDataValid(true);

  myFilter->SetInput(nd1);
  MITK_TEST_CONDITION(myFilter->GetInput() == nd1, "testing Set-/GetInput()");
 
  mitk::NavigationData* output = myFilter->GetOutput();
  MITK_TEST_CONDITION_REQUIRED(output != NULL, "Testing GetOutput()");


    /* register message receiver */
  MessageReceiverClass answers;
  myFilter->AddPositionChangedListener(mitk::MessageDelegate1<MessageReceiverClass, mitk::NavigationData::PositionType>(&answers, &MessageReceiverClass::OnPositionChanged));
  myFilter->AddOrientationChangedListener(mitk::MessageDelegate1<MessageReceiverClass, mitk::NavigationData::OrientationType>(&answers, &MessageReceiverClass::OnOrientationChanged));
  myFilter->AddErrorChangedListener(mitk::MessageDelegate1<MessageReceiverClass, mitk::NavigationData::CovarianceMatrixType>(&answers, &MessageReceiverClass::OnErrorChanged));
  myFilter->AddTimeStampChangedListener(mitk::MessageDelegate1<MessageReceiverClass, mitk::NavigationData::TimeStampType>(&answers, &MessageReceiverClass::OnTimeStampChanged));
  myFilter->AddDataValidChangedListener(mitk::MessageDelegate1<MessageReceiverClass, bool>(&answers, &MessageReceiverClass::OnDataValidChanged));


  output->Update(); // execute filter
  MITK_TEST_CONDITION( mitk::Equal(answers.m_ReceivedData->GetPosition(), nd1->GetPosition()), "Testing PositionChanged message");
  MITK_TEST_CONDITION( mitk::Equal(answers.m_ReceivedData->GetOrientation(), nd1->GetOrientation()), "Testing OrientationChanged message");
  MITK_TEST_CONDITION( answers.m_ReceivedData->GetCovErrorMatrix() == nd1->GetCovErrorMatrix(), "Testing ErrorChanged message");
  MITK_TEST_CONDITION( mitk::Equal(answers.m_ReceivedData->GetTimeStamp(), nd1->GetTimeStamp()), "Testing TimeStampChanged message");
  MITK_TEST_CONDITION( answers.m_ReceivedData->IsDataValid() == nd1->IsDataValid(), "Testing PositionChanged message");
  MITK_TEST_CONDITION( answers.m_MessagesReceived == 5, "Correct number of messages send?");
  
  /* change one input parameter */
  nd1->SetDataValid(false);
  output->Update(); // re-execute filter
  MITK_TEST_CONDITION( answers.m_ReceivedData->IsDataValid() == nd1->IsDataValid(), "Testing PositionChanged message");
  MITK_TEST_CONDITION( answers.m_MessagesReceived == 6, "only necessary messages send?");  // only datavalid message re-send

  /* changing two input parameters */  
  mitk::FillVector3D(initialPos, 11.0, 21.0, 31.0);
  nd1->SetPosition(initialPos); // change only one parameter
  nd1->SetTimeStamp(55.55); // change only one parameter
  output->Update(); // re-execute filter
  MITK_TEST_CONDITION( mitk::Equal(answers.m_ReceivedData->GetPosition(), nd1->GetPosition()), "Testing PositionChanged message");
  MITK_TEST_CONDITION( mitk::Equal(answers.m_ReceivedData->GetTimeStamp(), nd1->GetTimeStamp()), "Testing TimeStampChanged message");
  MITK_TEST_CONDITION( answers.m_MessagesReceived == 8, "only necessary messages send?");  // only 2 new messages send

  /* try to add a second input */
  MITK_TEST_OUTPUT_NO_ENDL( << "Exception on adding second input? --> ");
  mitk::NavigationData::Pointer nd2 = mitk::NavigationData::New();
  MITK_TEST_FOR_EXCEPTION(std::invalid_argument, myFilter->SetInput(1, nd2));

  // always end with this!
  MITK_TEST_END();
}
