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

#include "mitkNavigationDataToIGTLMessageFilter.h"
#include "mitkNavigationDataSequentialPlayer.h"
#include "mitkNavigationDataReaderXML.h"
#include <mitkIGTConfig.h>
#include <mitkTestingMacros.h>
#include <igtlPositionMessage.h>
#include <igtlTransformMessage.h>
#include <igtlQuaternionTrackingDataMessage.h>
#include <igtlTrackingDataMessage.h>

#include <iostream>

/**
*  Simple test for the class "NavigationDataToIGTLMessageFilter".
*
*  argc and argv are the command line parameters which were passed to
*  the ADD_TEST command in the CMakeLists.txt file. For the automatic
*  tests, argv is either empty for the simple tests or contains the filename
*  of a test image for the image tests (see CMakeLists.txt).
*/

mitk::NavigationDataToIGTLMessageFilter::Pointer m_NavigationDataToIGTLMessageFilter;

static void Setup()
{
  m_NavigationDataToIGTLMessageFilter = mitk::NavigationDataToIGTLMessageFilter::New();
}

static void SetInputs()
{
  //Build up test data
  mitk::NavigationData::Pointer nd0 = mitk::NavigationData::New();
  mitk::NavigationData::Pointer nd1 = mitk::NavigationData::New();
  mitk::NavigationData::Pointer nd2 = mitk::NavigationData::New();
  mitk::NavigationData::Pointer nd3 = mitk::NavigationData::New();

  mitk::NavigationData::PositionType point0;
  point0[0] = 1.0;
  point0[1] = 2.0;
  point0[2] = 3.0;
  mitk::NavigationData::OrientationType orientation0;
  orientation0.put(0, 1.0);
  orientation0.put(1, 0.0);
  orientation0.put(2, 0.0);
  orientation0.put(3, 0.0);
  nd0->SetPosition(point0);
  nd0->SetOrientation(orientation0);
  nd0->SetDataValid(true);

  mitk::NavigationData::PositionType point1;
  point1[0] = 4.0;
  point1[1] = 5.0;
  point1[2] = 6.0;
  mitk::NavigationData::OrientationType orientation1;
  orientation1.put(0, 21.0);
  orientation1.put(1, 22.0);
  orientation1.put(2, 23.0);
  orientation1.put(3, 24.0);
  nd1->SetPosition(point1);
  nd1->SetOrientation(orientation1);
  nd1->SetDataValid(true);

  mitk::NavigationData::PositionType point2;
  point2[0] = 7.0;
  point2[1] = 8.0;
  point2[2] = 9.0;
  mitk::NavigationData::OrientationType orientation2;
  orientation2.put(0, 31.0);
  orientation2.put(1, 32.0);
  orientation2.put(2, 33.0);
  orientation2.put(3, 34.0);
  nd2->SetPosition(point2);
  nd2->SetOrientation(orientation2);
  nd2->SetDataValid(true);

  mitk::NavigationData::PositionType point3;
  point3[0] = 10.0;
  point3[1] = 11.0;
  point3[2] = 12.0;
  mitk::NavigationData::OrientationType orientation3;
  orientation3.put(0, 0.0);
  orientation3.put(1, 0.0);
  orientation3.put(2, 0.0);
  orientation3.put(3, 1.0);
  nd3->SetPosition(point3);
  nd3->SetOrientation(orientation3);
  nd3->SetDataValid(true);

  m_NavigationDataToIGTLMessageFilter->SetInput(0, nd0);
  m_NavigationDataToIGTLMessageFilter->SetInput(1, nd1);
  m_NavigationDataToIGTLMessageFilter->SetInput(2, nd2);
  m_NavigationDataToIGTLMessageFilter->SetInput(3, nd3);
}

static void TestModeQTransMsg()
{
  Setup();
  SetInputs();
  m_NavigationDataToIGTLMessageFilter->SetOperationMode(
        mitk::NavigationDataToIGTLMessageFilter::ModeSendQTransMsg);

  //Process
  mitk::IGTLMessage::Pointer msg0 = m_NavigationDataToIGTLMessageFilter->GetOutput();
  mitk::IGTLMessage::Pointer msg1 = m_NavigationDataToIGTLMessageFilter->GetOutput(1);
  mitk::IGTLMessage::Pointer msg2 = m_NavigationDataToIGTLMessageFilter->GetOutput(2);
  mitk::IGTLMessage::Pointer msg3 = m_NavigationDataToIGTLMessageFilter->GetOutput(3);

  msg0->Update();

  igtl::PositionMessage::Pointer igtlMsg0 =
      dynamic_cast<igtl::PositionMessage*>(msg0->GetMessage().GetPointer());
  igtl::PositionMessage::Pointer igtlMsg3 =
      dynamic_cast<igtl::PositionMessage*>(msg3->GetMessage().GetPointer());

  MITK_TEST_OUTPUT(<< "Testing the converted OpenIGTLink messages:");
  MITK_TEST_CONDITION(igtlMsg0.IsNotNull(), "Message0 is not null?");
  MITK_TEST_CONDITION(igtlMsg3.IsNotNull(), "Message3 is not null?");

  //Convert the data from the igtl message back to mitk types
  float pos0_[3];
  float orientation0_[4];
  igtlMsg0->GetPosition(pos0_);
  igtlMsg0->GetQuaternion(orientation0_);
  mitk::NavigationData::PositionType pos0;
  pos0[0] = pos0_[0];
  pos0[1] = pos0_[1];
  pos0[2] = pos0_[2];
  mitk::NavigationData::OrientationType orientation0;
  orientation0[0] = orientation0_[0];
  orientation0[1] = orientation0_[1];
  orientation0[2] = orientation0_[2];
  orientation0[3] = orientation0_[3];
  float pos3_[3];
  float orientation3_[4];
  igtlMsg3->GetPosition(pos3_);
  igtlMsg3->GetQuaternion(orientation3_);
  mitk::NavigationData::PositionType pos3;
  pos3[0] = pos3_[0];
  pos3[1] = pos3_[1];
  pos3[2] = pos3_[2];
  mitk::NavigationData::OrientationType orientation3;
  orientation3[0] = orientation3_[0];
  orientation3[1] = orientation3_[1];
  orientation3[2] = orientation3_[2];
  orientation3[3] = orientation3_[3];

  MITK_TEST_OUTPUT(<< "Testing the conversion of navigation data object to QTrans OpenIGTLink messages:");
  MITK_TEST_CONDITION(mitk::Equal(pos0, m_NavigationDataToIGTLMessageFilter->GetInput(0)->GetPosition()), "Position0 correct?");
  MITK_TEST_CONDITION(mitk::Equal(pos3, m_NavigationDataToIGTLMessageFilter->GetInput(3)->GetPosition()), "Position3 correct?");
  MITK_TEST_CONDITION(mitk::Equal(orientation0, m_NavigationDataToIGTLMessageFilter->GetInput(0)->GetOrientation()), "Orientation0 correct?");
  MITK_TEST_CONDITION(mitk::Equal(orientation3, m_NavigationDataToIGTLMessageFilter->GetInput(3)->GetOrientation()), "Orientation3 correct?");
}

static void TestModeTransMsg()
{
  Setup();
  SetInputs();
  m_NavigationDataToIGTLMessageFilter->SetOperationMode(
        mitk::NavigationDataToIGTLMessageFilter::ModeSendTransMsg);

  //Process
  mitk::IGTLMessage::Pointer msg0 = m_NavigationDataToIGTLMessageFilter->GetOutput();
  mitk::IGTLMessage::Pointer msg1 = m_NavigationDataToIGTLMessageFilter->GetOutput(1);
  mitk::IGTLMessage::Pointer msg2 = m_NavigationDataToIGTLMessageFilter->GetOutput(2);
  mitk::IGTLMessage::Pointer msg3 = m_NavigationDataToIGTLMessageFilter->GetOutput(3);

  msg0->Update();

  igtl::TransformMessage::Pointer igtlMsg0 =
      dynamic_cast<igtl::TransformMessage*>(msg0->GetMessage().GetPointer());
  igtl::TransformMessage::Pointer igtlMsg3 =
      dynamic_cast<igtl::TransformMessage*>(msg3->GetMessage().GetPointer());

  MITK_TEST_OUTPUT(<< "Testing the converted OpenIGTLink messages:");
  MITK_TEST_CONDITION(igtlMsg0.IsNotNull(), "Message0 is not null?");
  MITK_TEST_CONDITION(igtlMsg3.IsNotNull(), "Message3 is not null?");

  //Convert the data from the igtl message back to mitk types
  mitk::AffineTransform3D::Pointer affineTransformation0 =
      mitk::AffineTransform3D::New();
  igtl::Matrix4x4 transformation0_;
  mitk::Matrix3D  transformation0;
  mitk::Vector3D  offset0;
  igtlMsg0->GetMatrix(transformation0_);
  for ( unsigned int r = 0; r < 3; r++ )
  {
    for ( unsigned int c = 0; c < 3; c++ )
    {
      transformation0.GetVnlMatrix().set( r , c , transformation0_[r][c] );
    }
    offset0.SetElement(r, transformation0_[r][3]);
  }
  //convert the igtl matrix here and set it in the affine transformation
  affineTransformation0->SetMatrix(transformation0);
  affineTransformation0->SetOffset(offset0);
  //the easiest way to convert the affine transform to position and quaternion
  mitk::NavigationData::Pointer nd0 =
      mitk::NavigationData::New(affineTransformation0, true);


  mitk::AffineTransform3D::Pointer affineTransformation3 =
      mitk::AffineTransform3D::New();
  igtl::Matrix4x4 transformation3_;
  mitk::Matrix3D  transformation3;
  mitk::Vector3D  offset3;
  igtlMsg3->GetMatrix(transformation3_);
  for ( unsigned int r = 0; r < 3; r++ )
  {
    for ( unsigned int c = 0; c < 3; c++ )
    {
      transformation3.GetVnlMatrix().set( r , c , transformation3_[r][c] );
    }
    offset3.SetElement(r, transformation3_[r][3]);
  }
  //convert the igtl matrix here and set it in the affine transformation
  affineTransformation3->SetMatrix(transformation3);
  affineTransformation3->SetOffset(offset3);
  //the easiest way to convert the affine transform to position and quaternion
  mitk::NavigationData::Pointer nd3 =
      mitk::NavigationData::New(affineTransformation3, true);

  MITK_TEST_OUTPUT(<< "Testing the conversion of navigation data object to Trans OpenIGTLink messages:");
  MITK_TEST_CONDITION(mitk::Equal(nd0->GetPosition(), m_NavigationDataToIGTLMessageFilter->GetInput(0)->GetPosition()), "Position0 correct?");
  MITK_TEST_CONDITION(mitk::Equal(nd3->GetPosition(), m_NavigationDataToIGTLMessageFilter->GetInput(3)->GetPosition()), "Position3 correct?");
  MITK_TEST_CONDITION(mitk::Equal(nd0->GetOrientation(), m_NavigationDataToIGTLMessageFilter->GetInput(0)->GetOrientation()), "Orientation0 correct?");
  MITK_TEST_CONDITION(mitk::Equal(nd3->GetOrientation(), m_NavigationDataToIGTLMessageFilter->GetInput(3)->GetOrientation()), "Orientation3 correct?");
}

static void NavigationDataToIGTLMessageFilterContructor_DefaultCall_IsNotEmpty()
{
  Setup();
  MITK_TEST_CONDITION_REQUIRED(m_NavigationDataToIGTLMessageFilter.IsNotNull(),"Testing instantiation");
  //I think this test is meaningless, because it will never ever fail. I keep it for know just to be save.
}

int mitkNavigationDataToIGTLMessageFilterTest(int /* argc */, char* /*argv*/[])
{
  MITK_TEST_BEGIN("NavigationDataToIGTLMessageFilter");

  NavigationDataToIGTLMessageFilterContructor_DefaultCall_IsNotEmpty();
  TestModeQTransMsg();
  TestModeTransMsg();

  MITK_TEST_END();
}
