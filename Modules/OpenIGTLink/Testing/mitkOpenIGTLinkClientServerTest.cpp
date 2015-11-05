//TEST
#include <mitkTestingMacros.h>
#include <mitkTestFixture.h>

//STD
#include <thread>
#include <chrono>

//MITK
#include "mitkIGTLServer.h"
#include "mitkIGTLClient.h"
#include "mitkIGTLMessageFactory.h"

//IGTL
#include "igtlStatusMessage.h"
#include "igtlClientSocket.h"
#include "igtlServerSocket.h"

class mitkOpenIGTLinkClientServerTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkOpenIGTLinkClientServerTestSuite);
  MITK_TEST(Test_JustIGTLImpl_OpenAndCloseAndThenReopenAndCloseServer_Successful);
  MITK_TEST(Test_ConnectingOneClientAndOneServer_Successful);
  MITK_TEST(Test_ConnectingMultipleClientsToOneServer_Successful);
  MITK_TEST(Test_DisconnectionServerFirst_Successful);
  MITK_TEST(Test_SendingMessageFromServerToOneClient_Successful);
  MITK_TEST(Test_SendingMessageFromServerToMultipleClients_Successful);
  CPPUNIT_TEST_SUITE_END();

private:
  int PORT = 35352;
  const std::string HOSTNAME = "localhost";
  const std::string SERVER_DEVICE_NAME = "Test Server";
  const std::string CLIENT_ONE_DEVICE_NAME = "Test Client 1";
  const std::string CLIENT_TWO_DEVICE_NAME = "Test Client 2";

  std::string m_Message;
  mitk::IGTLServer::Pointer m_Server;
  mitk::IGTLClient::Pointer m_Client_One;
  mitk::IGTLClient::Pointer m_Client_Two;
  mitk::IGTLMessageFactory::Pointer m_MessageFactory;

public:

  void setUp() override
  {
    m_Message = "This is a test status message";
    m_MessageFactory = mitk::IGTLMessageFactory::New();
    m_Server = mitk::IGTLServer::New(true);
    m_Client_One = mitk::IGTLClient::New(true);
    m_Client_Two = mitk::IGTLClient::New(true);

    m_Server->SetObjectName(SERVER_DEVICE_NAME);
    m_Server->SetHostname(HOSTNAME);
    m_Server->SetName(SERVER_DEVICE_NAME);

    m_Client_One->SetObjectName(CLIENT_ONE_DEVICE_NAME);
    m_Client_One->SetHostname(HOSTNAME);
    m_Client_One->SetName(CLIENT_ONE_DEVICE_NAME);

    m_Client_Two->SetObjectName(CLIENT_TWO_DEVICE_NAME);
    m_Client_Two->SetHostname(HOSTNAME);
    m_Client_Two->SetName(CLIENT_TWO_DEVICE_NAME);
  }

  void tearDown() override
  {
    m_Message.clear();
    m_Server = nullptr;
    m_Client_One = nullptr;
    m_Client_Two = nullptr;
    m_MessageFactory = nullptr;
  }

  void testMessagesEqual(igtl::MessageBase::Pointer sentMessage, igtl::MessageBase::Pointer receivedMessage)
  {
    std::string lhs(sentMessage->GetDeviceName());
    std::string rhs(receivedMessage->GetDeviceName());
    CPPUNIT_ASSERT_MESSAGE("The device names were not the same", lhs == rhs);
    igtl::StatusMessage::Pointer receivedStatusMessage = dynamic_cast<igtl::StatusMessage*>(receivedMessage.GetPointer());
    igtl::StatusMessage::Pointer sentStatusMessage = dynamic_cast<igtl::StatusMessage*>(sentMessage.GetPointer());
    CPPUNIT_ASSERT_MESSAGE("The received message was not of the appropriate type.", receivedStatusMessage != nullptr);
    CPPUNIT_ASSERT_MESSAGE("The sent message was not of the appropriate type.", sentStatusMessage != nullptr);

    lhs = receivedStatusMessage->GetStatusString();
    rhs = sentStatusMessage->GetStatusString();
    CPPUNIT_ASSERT_MESSAGE("The sent and received message did not contain the same status message.", lhs == rhs);
    CPPUNIT_ASSERT_MESSAGE("The sent message did not contain the correct status message.", lhs == m_Message);
    CPPUNIT_ASSERT_MESSAGE("The received message did not contain the correct status message.", m_Message == rhs);
  };

  void Test_JustIGTLImpl_OpenAndCloseAndThenReopenAndCloseServer_Successful()
  {
    //TODO: Delete this line. This is a workaround for BUG 19426 http://bugs.mitk.org/show_bug.cgi?id=19426
    m_Server->SetPortNumber(PORT);
    m_Client_One->SetPortNumber(PORT);
    m_Client_Two->SetPortNumber(PORT);

    igtl::ServerSocket::Pointer server = igtl::ServerSocket::New();
    igtl::ClientSocket::Pointer client = igtl::ClientSocket::New();

    server->CreateServer(PORT);
    client->ConnectToServer("localhost", PORT);

    client->CloseSocket();
    server->CloseSocket();

    server->CreateServer(++PORT);
    client->ConnectToServer("localhost", PORT);

    client->CloseSocket();
    server->CloseSocket();

    server = nullptr;
    client = nullptr;
  }

  void Test_ConnectingOneClientAndOneServer_Successful()
  {
    //TODO: Delete this line. This is a workaround for BUG 19426 http://bugs.mitk.org/show_bug.cgi?id=19426
    m_Server->SetPortNumber(PORT + 2);
    m_Client_One->SetPortNumber(PORT + 2);
    m_Client_Two->SetPortNumber(PORT + 2);

    CPPUNIT_ASSERT_MESSAGE("Could not open Connection with Server", m_Server->OpenConnection());
    CPPUNIT_ASSERT_MESSAGE("Could not connect to Server with first client", m_Client_One->OpenConnection());

    CPPUNIT_ASSERT(m_Client_One->CloseConnection());
    CPPUNIT_ASSERT(m_Server->CloseConnection());
  }

  void Test_ConnectingMultipleClientsToOneServer_Successful()
  {
    //TODO: Delete this line. This is a workaround for BUG 19426 http://bugs.mitk.org/show_bug.cgi?id=19426
    m_Server->SetPortNumber(PORT + 3);
    m_Client_One->SetPortNumber(PORT + 3);
    m_Client_Two->SetPortNumber(PORT + 3);

    CPPUNIT_ASSERT_MESSAGE("Could not open Connection with Server", m_Server->OpenConnection());
    m_Server->StartCommunication();

    CPPUNIT_ASSERT_MESSAGE("Could not connect to Server with first client", m_Client_One->OpenConnection());
    CPPUNIT_ASSERT_MESSAGE("Could not start communication with first client", m_Client_One->StartCommunication());

    CPPUNIT_ASSERT_MESSAGE("Could not connect to Server with second client", m_Client_Two->OpenConnection());
    CPPUNIT_ASSERT_MESSAGE("Could not start communication with second client", m_Client_Two->StartCommunication());

    CPPUNIT_ASSERT(m_Client_One->CloseConnection());
    CPPUNIT_ASSERT(m_Client_Two->CloseConnection());
    CPPUNIT_ASSERT(m_Server->CloseConnection());
  }

  void Test_DisconnectionServerFirst_Successful()
  {
    //TODO: Delete this line. This is a workaround for BUG 19426 http://bugs.mitk.org/show_bug.cgi?id=19426
    m_Server->SetPortNumber(PORT + 4);
    m_Client_One->SetPortNumber(PORT + 4);
    m_Client_Two->SetPortNumber(PORT + 4);

    CPPUNIT_ASSERT_MESSAGE("Could not open Connection with Server", m_Server->OpenConnection());
    m_Server->StartCommunication();
    CPPUNIT_ASSERT_MESSAGE("Could not connect to Server with first client", m_Client_One->OpenConnection());
    CPPUNIT_ASSERT_MESSAGE("Could not start communication with first client", m_Client_One->StartCommunication());
    CPPUNIT_ASSERT_MESSAGE("Could not connect to Server with second client", m_Client_Two->OpenConnection());
    CPPUNIT_ASSERT_MESSAGE("Could not start communication with second client", m_Client_Two->StartCommunication());

    CPPUNIT_ASSERT(m_Server->CloseConnection());
    CPPUNIT_ASSERT(m_Client_One->CloseConnection());
    CPPUNIT_ASSERT(m_Client_Two->CloseConnection());
  }

  void Test_SendingMessageFromServerToOneClient_Successful()
  {
    //TODO: Delete this line. This is a workaround for BUG 19426 http://bugs.mitk.org/show_bug.cgi?id=19426
    m_Server->SetPortNumber(PORT + 5);
    m_Client_One->SetPortNumber(PORT + 5);
    m_Client_Two->SetPortNumber(PORT + 5);

    CPPUNIT_ASSERT_MESSAGE("Server not connected to Client.", m_Server->OpenConnection());
    CPPUNIT_ASSERT_MESSAGE("Client 1 not connected to Server.", m_Client_One->OpenConnection());
    m_Server->StartCommunication();
    m_Client_One->StartCommunication();

    igtl::MessageBase::Pointer sentMessage = m_MessageFactory->CreateInstance("STATUS");
    dynamic_cast<igtl::StatusMessage*>(sentMessage.GetPointer())->SetStatusString(m_Message.c_str());
    m_Server->SendMessage(sentMessage);
    igtl::MessageBase::Pointer receivedMessage;
    int steps = 0;
    while ((receivedMessage = m_Client_One->GetReceiveQueue()->PullMessage()) == nullptr)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(5));
      if (++steps > 20)
        break;
    }
    CPPUNIT_ASSERT(receivedMessage != nullptr);

    CPPUNIT_ASSERT(m_Client_One->StopCommunication());
    CPPUNIT_ASSERT(m_Server->StopCommunication());

    CPPUNIT_ASSERT(m_Client_One->CloseConnection());
    CPPUNIT_ASSERT(m_Server->CloseConnection());

    testMessagesEqual(sentMessage, receivedMessage);
  }

  void Test_SendingMessageFromServerToMultipleClients_Successful()
  {
    //TODO: Delete this line. This is a workaround for BUG 19426 http://bugs.mitk.org/show_bug.cgi?id=19426
    m_Server->SetPortNumber(PORT + 6);
    m_Client_One->SetPortNumber(PORT + 6);
    m_Client_Two->SetPortNumber(PORT + 6);

    CPPUNIT_ASSERT_MESSAGE("Server not connected to Client.", m_Server->OpenConnection());
    m_Server->StartCommunication();
    CPPUNIT_ASSERT_MESSAGE("Client 1 not connected to Server.", m_Client_One->OpenConnection());
    m_Client_One->StartCommunication();
    CPPUNIT_ASSERT_MESSAGE("Client 2 not connected to Server.", m_Client_Two->OpenConnection());
    m_Client_Two->StartCommunication();

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    igtl::MessageBase::Pointer sentMessage = m_MessageFactory->CreateInstance("STATUS");
    dynamic_cast<igtl::StatusMessage*>(sentMessage.GetPointer())->SetStatusString(m_Message.c_str());
    m_Server->SendMessage(sentMessage);
    MITK_INFO << "SENT MESSAGE";

    igtl::MessageBase::Pointer receivedMessage1;
    igtl::MessageBase::Pointer receivedMessage2;
    int steps = 0;
    while (receivedMessage1 == nullptr || receivedMessage2 == nullptr)
    {
      std::this_thread::sleep_for(std::chrono::milliseconds(20));

      igtl::MessageBase::Pointer tmpMessage1 = m_Client_One->GetReceiveQueue()->PullMessage();
      if (tmpMessage1.IsNotNull())
        receivedMessage1 = tmpMessage1;

      igtl::MessageBase::Pointer tmpMessage2 = m_Client_Two->GetReceiveQueue()->PullMessage();
      if (tmpMessage2.IsNotNull())
        receivedMessage2 = tmpMessage2;

      if (++steps > 50)
        break;
    }

    CPPUNIT_ASSERT(m_Client_Two->StopCommunication());
    CPPUNIT_ASSERT(m_Client_One->StopCommunication());
    CPPUNIT_ASSERT(m_Server->StopCommunication());

    CPPUNIT_ASSERT(m_Client_Two->CloseConnection());
    CPPUNIT_ASSERT(m_Client_One->CloseConnection());
    CPPUNIT_ASSERT(m_Server->CloseConnection());

    CPPUNIT_ASSERT_MESSAGE("Message from first client was null..", receivedMessage1 != nullptr);
    CPPUNIT_ASSERT_MESSAGE("Message from first client was null..", receivedMessage1.IsNotNull());
    CPPUNIT_ASSERT_MESSAGE("Message from second client was null..", receivedMessage2 != nullptr);
    CPPUNIT_ASSERT_MESSAGE("Message from second client was null..", receivedMessage2.IsNotNull());

    testMessagesEqual(sentMessage, receivedMessage1);
    testMessagesEqual(sentMessage, receivedMessage2);
    testMessagesEqual(receivedMessage2, receivedMessage1);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkOpenIGTLinkClientServer)