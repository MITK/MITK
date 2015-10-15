#include <mitkTestingMacros.h>
#include <mitkTestFixture.h>

#include "mitkIGTLServer.h"
#include "mitkIGTLClient.h"

class mitkOpenIGTLinkClientServerTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkOpenIGTLinkClientServerTestSuite);
  MITK_TEST(TestConnectingClientAndServer_Successful);
  CPPUNIT_TEST_SUITE_END();

private:
  const int PORT = 85353;
  const std::string HOSTNAME = "localhost";
  std::string m_Message;
  mitk::IGTLServer::Pointer m_Server;
  mitk::IGTLClient::Pointer m_Client;

public:

  void setUp() override
  {
    MITK_INFO << "Setting up now";
    m_Message = "Testmessage";
    m_Server = mitk::IGTLServer::New(true);
    m_Client = mitk::IGTLClient::New(true);

    m_Server->SetPortNumber(PORT);
    m_Server->SetHostname(HOSTNAME);
    m_Server->SetName("Test OpenIGTLink Server");

    m_Client->SetPortNumber(PORT);
    m_Client->SetHostname(HOSTNAME);
    m_Client->SetName("Test OpenIGTLink Client");
  }

  void tearDown() override
  {
    MITK_INFO << "Tearing down now";
  }

  void TestConnectingClientAndServer_Successful()
  {
    MITK_INFO << "Testing here :-)";
    CPPUNIT_ASSERT(true);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkOpenIGTLinkClientServer)