/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkTestingMacros.h>
#include <mitkTestFixture.h>

#include <mitkIGTLMessageFactory.h>

#include <igtlImageMessage.h>
#include <igtlTransformMessage.h>
#include <igtlPositionMessage.h>
#include <igtlStatusMessage.h>
#include <igtlCapabilityMessage.h>
#include <igtlStringMessage.h>
#include <mitkIGTLDummyMessage.h>

static const std::string TYPE_IMAGE = "IMAGE";
static const std::string TYPE_TRANSFORM = "TRANSFORM";
static const std::string TYPE_POSITION = "POSITION";
static const std::string TYPE_STATUS = "STATUS";
static const std::string TYPE_CAPABILITY = "CAPABILITY";
static const std::string TYPE_TEST = "TEST";

class mitkOpenIGTLinkImageFactoryTestSuite : public mitk::TestFixture {
CPPUNIT_TEST_SUITE(mitkOpenIGTLinkImageFactoryTestSuite);
MITK_TEST(Test_CreateImageMessage_NotNull);
MITK_TEST(Test_CreateTransformMessage_NotNull);
MITK_TEST(Test_CreatePositionMessage_NotNull);
MITK_TEST(Test_CreateStatusMessage_NotNull);
MITK_TEST(Test_CreateCapabilityMessage_NotNull);
MITK_TEST(Test_AddCustomMessageType_Succeeds);
CPPUNIT_TEST_SUITE_END();

private:

mitk::IGTLMessageFactory::Pointer m_MessageFactory;

public:

void setUp() override
{
m_MessageFactory = mitk::IGTLMessageFactory::New();
}

void tearDown() override
{
m_MessageFactory = nullptr;
}

void Test_CreateImageMessage_NotNull()
{
igtl::MessageBase::Pointer messageBase = m_MessageFactory->CreateInstance(TYPE_IMAGE);
igtl::ImageMessage::Pointer message = dynamic_cast<igtl::ImageMessage*>(messageBase.GetPointer());

CPPUNIT_ASSERT_MESSAGE("The created message was not of type igtl::ImageMessage", message.IsNotNull());
}

void Test_CreateTransformMessage_NotNull()
{
igtl::MessageBase::Pointer messageBase = m_MessageFactory->CreateInstance(TYPE_TRANSFORM);
igtl::TransformMessage::Pointer message = dynamic_cast<igtl::TransformMessage*>(messageBase.GetPointer());

CPPUNIT_ASSERT_MESSAGE("The created message was not of type igtl::TransformMessage", message.IsNotNull());
}

void Test_CreatePositionMessage_NotNull()
{
igtl::MessageBase::Pointer messageBase = m_MessageFactory->CreateInstance(TYPE_POSITION);
igtl::PositionMessage::Pointer message = dynamic_cast<igtl::PositionMessage*>(messageBase.GetPointer());

CPPUNIT_ASSERT_MESSAGE("The created message was not of type igtl::PositionMessage", message.IsNotNull());
}

void Test_CreateStatusMessage_NotNull()
{
igtl::MessageBase::Pointer messageBase = m_MessageFactory->CreateInstance(TYPE_STATUS);
igtl::StatusMessage::Pointer message = dynamic_cast<igtl::StatusMessage*>(messageBase.GetPointer());

CPPUNIT_ASSERT_MESSAGE("The created message was not of type igtl::StatusMessage", message.IsNotNull());
}

void Test_CreateCapabilityMessage_NotNull()
{
igtl::MessageBase::Pointer messageBase = m_MessageFactory->CreateInstance(TYPE_CAPABILITY);
igtl::CapabilityMessage::Pointer message = dynamic_cast<igtl::CapabilityMessage*>(messageBase.GetPointer());

CPPUNIT_ASSERT_MESSAGE("The created message was not of type igtl::CapabilityMessage", message.IsNotNull());
}

void Test_AddCustomMessageType_Succeeds()
{
m_MessageFactory->AddMessageNewMethod(TYPE_TEST, (mitk::IGTLMessageFactory::PointerToMessageBaseNew)&mitk::IGTLDummyMessage::New);

igtl::MessageBase::Pointer messageBase = m_MessageFactory->CreateInstance(TYPE_TEST);
mitk::IGTLDummyMessage::Pointer message = dynamic_cast<mitk::IGTLDummyMessage*>(messageBase.GetPointer());

CPPUNIT_ASSERT_MESSAGE("The created message was not of type mitk::IGTLDummyMessage", message.IsNotNull());
}
};

MITK_TEST_SUITE_REGISTRATION(mitkOpenIGTLinkImageFactory)
