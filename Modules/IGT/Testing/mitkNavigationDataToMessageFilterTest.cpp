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

    MessageReceiverClass(unsigned int numberOfNavigationDatas)
    {
      for (unsigned int i = 0; i < numberOfNavigationDatas, ++i)
        m_ReceivedData[i] = mitk::NavigationData::New();
      
      m_MessagesReceived = 0;
    }

    void OnPositionChanged(mitk::NavigationData::PositionType v, unsigned int index)
    {
      m_ReceivedData[index]->SetPosition(v);
      ++m_MessagesReceived;
    }
    
    void OnOrientationChanged(mitk::NavigationData::OrientationType v, unsigned int index)
    {
      m_ReceivedData[index]->SetOrientation(v);
      ++m_MessagesReceived;
    }

    void OnErrorChanged(mitk::NavigationData::CovarianceMatrixType v, unsigned int index)
    {
      m_ReceivedData[index]->SetCovErrorMatrix(v);
      ++m_MessagesReceived;
    }
    
    void OnTimeStampChanged(mitk::NavigationData::TimeStampType v, unsigned int index)
    {
      m_ReceivedData[index]->SetTimeStamp(v);
      ++m_MessagesReceived;
    }

    void OnDataValidChanged(bool v, unsigned int index)
    {
      m_ReceivedData[index]->SetDataValid(v);
      ++m_MessagesReceived;
    }

    std::vector<mitk::NavigationData::Pointer> m_ReceivedData;
    int m_MessagesReceived;
};

/**Documentation
 *  test for the class "NavigationDataToMessageFilter".
 */
int mitkNavigationDataToMessageFilterTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("NavigationDataToMessageFilter");
  /* first tests with one input */
  { 
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
    MessageReceiverClass answers(1);
    myFilter->AddPositionChangedListener(mitk::MessageDelegate2<MessageReceiverClass, mitk::NavigationData::PositionType, unsigned int>(&answers, &MessageReceiverClass::OnPositionChanged));
    myFilter->AddOrientationChangedListener(mitk::MessageDelegate2<MessageReceiverClass, mitk::NavigationData::OrientationType, unsigned int>(&answers, &MessageReceiverClass::OnOrientationChanged));
    myFilter->AddErrorChangedListener(mitk::MessageDelegate2<MessageReceiverClass, mitk::NavigationData::CovarianceMatrixType, unsigned int>(&answers, &MessageReceiverClass::OnErrorChanged));
    myFilter->AddTimeStampChangedListener(mitk::MessageDelegate2<MessageReceiverClass, mitk::NavigationData::TimeStampType, unsigned int>(&answers, &MessageReceiverClass::OnTimeStampChanged));
    myFilter->AddDataValidChangedListener(mitk::MessageDelegate2<MessageReceiverClass, bool, unsigned int>(&answers, &MessageReceiverClass::OnDataValidChanged));


    output->Update(); // execute filter
    MITK_TEST_CONDITION( mitk::Equal(answers.m_ReceivedData[0]->GetPosition(), nd1->GetPosition()), "Testing PositionChanged message");
    MITK_TEST_CONDITION( mitk::Equal(answers.m_ReceivedData[0]->GetOrientation(), nd1->GetOrientation()), "Testing OrientationChanged message");
    MITK_TEST_CONDITION( answers.m_ReceivedData[0]->GetCovErrorMatrix() == nd1->GetCovErrorMatrix(), "Testing ErrorChanged message");
    MITK_TEST_CONDITION( mitk::Equal(answers.m_ReceivedData[0]->GetTimeStamp(), nd1->GetTimeStamp()), "Testing TimeStampChanged message");
    MITK_TEST_CONDITION( answers.m_ReceivedData[0]->IsDataValid() == nd1->IsDataValid(), "Testing PositionChanged message");
    MITK_TEST_CONDITION( answers.m_MessagesReceived == 5, "Correct number of messages send?");

    /* change one input parameter */
    nd1->SetDataValid(false);
    output->Update(); // re-execute filter
    MITK_TEST_CONDITION( answers.m_ReceivedData[0]->IsDataValid() == nd1->IsDataValid(), "Testing PositionChanged message");
    MITK_TEST_CONDITION( answers.m_MessagesReceived == 6, "only necessary messages send?");  // only datavalid message re-send

    /* changing two input parameters */  
    mitk::FillVector3D(initialPos, 11.0, 21.0, 31.0);
    nd1->SetPosition(initialPos); // change only one parameter
    nd1->SetTimeStamp(55.55); // change only one parameter
    output->Update(); // re-execute filter
    MITK_TEST_CONDITION( mitk::Equal(answers.m_ReceivedData[0]->GetPosition(), nd1->GetPosition()), "Testing PositionChanged message");
    MITK_TEST_CONDITION( mitk::Equal(answers.m_ReceivedData[0]->GetTimeStamp(), nd1->GetTimeStamp()), "Testing TimeStampChanged message");
    MITK_TEST_CONDITION( answers.m_MessagesReceived == 8, "only necessary messages send?");  // only 2 new messages send

    /* try to add a second input */
    //MITK_TEST_OUTPUT_NO_ENDL( << "Exception on adding second input? --> ");
    //mitk::NavigationData::Pointer nd2 = mitk::NavigationData::New();
    //MITK_TEST_FOR_EXCEPTION(std::invalid_argument, myFilter->SetInput(1, nd2));

  }
  /* now test with multiple inputs */
  {
    MITK_TEST_OUTPUT( << "Now, perform tests with multiple inputs");

    mitk::NavigationDataToMessageFilter::Pointer myFilter = mitk::NavigationDataToMessageFilter::New();

    /* create helper objects: navigation data with position as origin, zero quaternion, zero error and data valid */
    mitk::NavigationData::PositionType initialPos;
    mitk::FillVector3D(initialPos, 1.0, 1.0, 1.0);
    mitk::NavigationData::OrientationType initialOri(1.0, 1.0, 1.0, 1.0);

    mitk::NavigationData::Pointer nd0 = mitk::NavigationData::New();
    nd0->SetPosition(initialPos);
    nd0->SetOrientation(initialOri);
    nd0->SetPositionAccuracy(11.111);
    nd0->SetTimeStamp(64.46);
    nd0->SetDataValid(true);

    mitk::FillVector3D(initialPos, 2.0, 2.0, 2.0);
    mitk::NavigationData::OrientationType initialOri2(1.0, 1.0, 1.0, 1.0)
    mitk::NavigationData::Pointer nd1 = mitk::NavigationData::New();
    nd1->SetPosition(initialPos);
    nd1->SetOrientation(initialOri2);
    nd1->SetPositionAccuracy(22.222);
    nd1->SetTimeStamp(222.2);
    nd1->SetDataValid(false);
  
    myFilter->SetInput(0, nd0);
    myFilter->SetInput(1, nd1);
    MITK_TEST_CONDITION(myFilter->GetInput(0) == nd0, "testing Set-/GetInput(0)");
    MITK_TEST_CONDITION(myFilter->GetInput(1) == nd1, "testing Set-/GetInput(1)");

    mitk::NavigationData* output0 = myFilter->GetOutput(0);
    mitk::NavigationData* output1 = myFilter->GetOutput(1);
    MITK_TEST_CONDITION_REQUIRED(output != NULL, "Testing GetOutput(0)");
    MITK_TEST_CONDITION_REQUIRED(output != NULL, "Testing GetOutput(1)");

    /* register message receiver */
    MessageReceiverClass answers(2);
    myFilter->AddPositionChangedListener(mitk::MessageDelegate2<MessageReceiverClass, mitk::NavigationData::PositionType, unsigned int>(&answers, &MessageReceiverClass::OnPositionChanged));
    myFilter->AddOrientationChangedListener(mitk::MessageDelegate2<MessageReceiverClass, mitk::NavigationData::OrientationType, unsigned int>(&answers, &MessageReceiverClass::OnOrientationChanged));
    myFilter->AddErrorChangedListener(mitk::MessageDelegate2<MessageReceiverClass, mitk::NavigationData::CovarianceMatrixType, unsigned int>(&answers, &MessageReceiverClass::OnErrorChanged));
    myFilter->AddTimeStampChangedListener(mitk::MessageDelegate2<MessageReceiverClass, mitk::NavigationData::TimeStampType, unsigned int>(&answers, &MessageReceiverClass::OnTimeStampChanged));
    myFilter->AddDataValidChangedListener(mitk::MessageDelegate2<MessageReceiverClass, bool, unsigned int>(&answers, &MessageReceiverClass::OnDataValidChanged));

    output0->Update(); // execute filter. This should send messages for both inputs
    MITK_TEST_CONDITION( mitk::Equal(answers.m_ReceivedData[0]->GetPosition(), nd0->GetPosition()), "Testing PositionChanged message");
    MITK_TEST_CONDITION( mitk::Equal(answers.m_ReceivedData[0]->GetOrientation(), nd0->GetOrientation()), "Testing OrientationChanged message");
    MITK_TEST_CONDITION( answers.m_ReceivedData[0]->GetCovErrorMatrix() == nd0->GetCovErrorMatrix(), "Testing ErrorChanged message");
    MITK_TEST_CONDITION( mitk::Equal(answers.m_ReceivedData[0]->GetTimeStamp(), nd0->GetTimeStamp()), "Testing TimeStampChanged message");
    MITK_TEST_CONDITION( answers.m_ReceivedData[0]->IsDataValid() == nd0->IsDataValid(), "Testing PositionChanged message");
    MITK_TEST_CONDITION( mitk::Equal(answers.m_ReceivedData[1]->GetPosition(), nd1->GetPosition()), "Testing PositionChanged message");
    MITK_TEST_CONDITION( mitk::Equal(answers.m_ReceivedData[1]->GetOrientation(), nd1->GetOrientation()), "Testing OrientationChanged message");
    MITK_TEST_CONDITION( answers.m_ReceivedData[1]->GetCovErrorMatrix() == nd1->GetCovErrorMatrix(), "Testing ErrorChanged message");
    MITK_TEST_CONDITION( mitk::Equal(answers.m_ReceivedData[1]->GetTimeStamp(), nd1->GetTimeStamp()), "Testing TimeStampChanged message");
    MITK_TEST_CONDITION( answers.m_ReceivedData[1]->IsDataValid() == nd1->IsDataValid(), "Testing PositionChanged message");
    MITK_TEST_CONDITION( answers.m_MessagesReceived == 10, "Correct number of messages send?");

    /* change one input parameter */
    nd0->SetDataValid(false);
    output0->Update(); // re-execute filter
    MITK_TEST_CONDITION( answers.m_ReceivedData[0]->IsDataValid() == nd0->IsDataValid(), "Testing PositionChanged message for input 0");
    MITK_TEST_CONDITION( answers.m_MessagesReceived == 11, "only necessary messages send?");  // only datavalid message for input 0 re-send

    /* remove one listener and check that message is not send */
    myFilter->RemoveTimeStampChangedListener(mitk::MessageDelegate2<MessageReceiverClass, mitk::NavigationData::TimeStampType, unsigned int>(&answers, &MessageReceiverClass::OnTimeStampChanged));
    mitk::NavigationData::TimeStampType oldValue = nd1->GetTimeStamp();
    nd1->SetTimeStamp(999.9);
    myFilter->Update();
    MITK_TEST_CONDITION( ! mitk::Equal(answers.m_ReceivedData[1]->GetTimeStamp(), nd1->GetTimeStamp()), "Testing if TimeStamp message is _not_ send after RemoveListener (!= new value)");
    MITK_TEST_CONDITION( mitk::Equal(answers.m_ReceivedData[1]->GetTimeStamp(), oldValue), "Testing if TimeStamp message is _not_ send after RemoveListener (== old value)");
    MITK_TEST_CONDITION( answers.m_MessagesReceived == 11, "no new messages send?");  // no new message send?
    /* other messages are still send? */
    nd1->SetDataValid(true);
    myFilter->Update();
    MITK_TEST_CONDITION( answers.m_ReceivedData[1]->IsDataValid() == nd1->IsDataValid(), "Other messages still send? ->Testing PositionChanged message for input 1 again");
    MITK_TEST_CONDITION( answers.m_MessagesReceived == 12, "only necessary messages send?");  // only DataValid message for input 1 re-send
    /* check if other output still has its old value */
    MITK_TEST_CONDITION( answers.m_ReceivedData[0]->IsDataValid() == nd0->IsDataValid(), "Testing PositionChanged message for input 0");
  }
  // The end
  MITK_TEST_END();
}
