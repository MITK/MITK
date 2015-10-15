#include <mitkTestingMacros.h>
#include <mitkTestFixture.h>

class mitkOpenIGTLinkIGTLImageMessageFilterTestSuite : public mitk::TestFixture {
  CPPUNIT_TEST_SUITE(mitkOpenIGTLinkIGTLImageMessageFilterTestSuite);
  MITK_TEST(Confirm_TestsWorking);
  CPPUNIT_TEST_SUITE_END();

public:

  void setUp() override
  {
  }

  void tearDown() override
  {
  }

  void Confirm_TestsWorking()
  {
    CPPUNIT_ASSERT(true);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkOpenIGTLinkIGTLImageMessageFilter)