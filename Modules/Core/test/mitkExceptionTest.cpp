/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// Testing
#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"

// std includes
#include <string>

// MITK includes
#include "mitkException.h"
#include "mitkExceptionMacro.h"
#include "mitkTestingMacros.h"
#include <mitkCommon.h>

// ITK includes
#include <itkObject.h>
#include <itkObjectFactory.h>

// VTK includes
#include <vtkDebugLeaks.h>

class SpecializedTestException : public mitk::Exception
{
public:
  mitkExceptionClassMacro(SpecializedTestException, mitk::Exception);
};

class mitkExceptionTestSuite : public itk::Object, public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkExceptionTestSuite);

  MITK_TEST(TestExceptionConstructor_Success);
  MITK_TEST(TestSpecializedExceptionConstructor_Success);

  MITK_TEST(TestExceptionMessageStreamAddingString_Success);
  MITK_TEST(TestExceptionMessageStreamAddingSingleChars_Success);
  MITK_TEST(TestExceptionMessageStreamAddingObject_Success);
  MITK_TEST(TestSpecializedExceptionMessageStreamAddingString);
  MITK_TEST(TestExceptionMessageStreamThrowing_Success);

  MITK_TEST(TestMitkThrowMacroThrowing_Success);
  MITK_TEST(TestMitkThrowMacroMessage_Success);
  MITK_TEST(TestMitkThrowMacroException_Success);
  MITK_TEST(TestMitkThrowMacroSpezcializedException);

  MITK_TEST(TestGetNumberOfRethrows_Success);

  MITK_TEST(TestGetRethrowDataWithNegativNumber_Success);
  MITK_TEST(TestGetRethrowDataWithNumberZero_Success);
  MITK_TEST(TestGetRethrowDataWithNumberOne_Success);

  MITK_TEST(TestAddRethrowData_Success);

  MITK_TEST(TestFirstRethrowDataAreStoredProperly_Success);
  MITK_TEST(TestSecondRethrowDataAreStoredProperly_Success);

  MITK_TEST(TestRethrowMacro_Success);

  CPPUNIT_TEST_SUITE_END();

private:
  bool m_ExceptionThrown;

  std::string m_MessageText;
  std::string m_Message;
  std::string m_File;

  int m_Line;

  mitk::Exception m_E = mitk::Exception("test.cpp", 155, "", "");
  mitk::Exception m_MyException = mitk::Exception("testfile.cpp", 111, "testmessage");

public:
  mitkClassMacroItkParent(mitkExceptionTestSuite, itk::Object);
  itkFactorylessNewMacro(Self) itkCloneMacro(Self)

    void throwExceptionManually()
  // this method is ONLY to test the constructor and no code example
  // normally exceptions should only be thrown by using the exception macro!
  {
    throw mitk::Exception("test.cpp", 155, "", "");
  }

  void throwSpecializedExceptionManually()
  // this method is ONLY to test the constructor and no code example
  // normally exceptions should only be thrown by using the exception macro!
  {
    throw SpecializedTestException("test.cpp", 155, "", "");
  }

  void throwExceptionManually(std::string message1, std::string message2)
  // this method is ONLY to test methods of mitk::Exception and no code example
  // normally exceptions should only be thrown by using the exception macro!
  {
    throw mitk::Exception("testfile.cpp", 155, message1.c_str(), "") << message2;
  }

  void throwExceptionWithThrowMacro() { mitkThrow() << "TEST EXCEPION THROWING WITH mitkThrow()"; }
  void throwExceptionWithThrowMacro(std::string message) { mitkThrow() << message.c_str(); }
  void throwSpecializedExceptionWithThrowMacro(std::string message) { mitkThrowException(mitk::Exception) << message; }
  void throwSpecializedExceptionWithThrowMacro2(std::string message)
  {
    mitkThrowException(SpecializedTestException) << message;
  }

  void reThrowExceptionWithReThrowMacro(std::string messageThrow, std::string messageReThrow)
  {
    try
    {
      throwExceptionWithThrowMacro(messageThrow);
    }
    catch (mitk::Exception &e)
    {
      mitkReThrow(e) << messageReThrow;
    }
  }

  void setUp() override
  {
    m_ExceptionThrown = false;
    m_MessageText = "";
    m_Message = "invalid";
    m_File = "invalid";
    m_Line = -1;
  }

  void tearDown() override
  {
    m_ExceptionThrown = false;
    m_MessageText = "";
    m_Message = "";
    m_File = "";
    m_Line = 0;
  }

  void TestExceptionConstructor_Success()
  {
    try
    {
      this->throwExceptionManually();
    }
    catch (const mitk::Exception &)
    {
      m_ExceptionThrown = true;
    }
    CPPUNIT_ASSERT_MESSAGE("Testing constructor of mitkException", m_ExceptionThrown);
  }

  void TestSpecializedExceptionConstructor_Success()
  {
    try
    {
      this->throwSpecializedExceptionManually();
    }
    catch (const SpecializedTestException &)
    {
      m_ExceptionThrown = true;
    }
    CPPUNIT_ASSERT_MESSAGE("Testing constructor specialized exception (deriving from mitkException)",
                           m_ExceptionThrown);
  }

  //##### this methods are ONLY to test the streaming operators of the exceptions and
  //##### NO code example. Please do not instantiate exceptions by yourself in normal code!
  //##### Normally exceptions should only be thrown by using the exception macro!
  void TestExceptionMessageStreamAddingString_Success()
  {
    m_MyException << " and additional stream";
    CPPUNIT_ASSERT_MESSAGE("Testing mitkException message stream (adding std::string)",
                           m_MyException.GetDescription() == std::string("testmessage and additional stream"));
  }

  void TestExceptionMessageStreamAddingSingleChars_Success()
  {
    m_MyException.SetDescription("testmessage2");
    m_MyException << ' ' << 'a' << 'n' << 'd' << ' ' << 'c' << 'h' << 'a' << 'r' << 's';
    CPPUNIT_ASSERT_MESSAGE("Testing mitkException message stream (adding single chars)",
                           m_MyException.GetDescription() == std::string("testmessage2 and chars"));
  }

  void TestExceptionMessageStreamAddingObject_Success()
  {
    m_MyException.SetDescription("testmessage3");
    m_MyException << m_MyException; // adding the object itself makes no sense but should work
    CPPUNIT_ASSERT_MESSAGE("Testing mitkException message stream (adding object)",
                           m_MyException.GetDescription() != std::string(""));
  }

  void TestSpecializedExceptionMessageStreamAddingString()
  {
    SpecializedTestException mySpecializedException =
      SpecializedTestException("testfile.cpp", 111, "testmessage", "test");
    mySpecializedException << " and additional stream";
    CPPUNIT_ASSERT_MESSAGE("Testing specialized exception message stream (adding std::string)",
                           mySpecializedException.GetDescription() == std::string("testmessage and additional stream"));
  }

  void TestExceptionMessageStreamThrowing_Success()
  {
    std::string thrownMessage = "";
    try
    {
      this->throwExceptionManually("message1", " and message2");
    }
    catch (const mitk::Exception &e)
    {
      thrownMessage = e.GetDescription();
      m_ExceptionThrown = true;
    }
    CPPUNIT_ASSERT_MESSAGE("Testing throwing and streaming of mitk::Exception together.",
                           m_ExceptionThrown && (thrownMessage == std::string("message1 and message2")));
  }

  void TestMitkThrowMacroThrowing_Success()
  {
    // case 1: test throwing
    try
    {
      this->throwExceptionWithThrowMacro();
    }
    catch (const mitk::Exception &)
    {
      m_ExceptionThrown = true;
    }
    CPPUNIT_ASSERT_MESSAGE("Testing mitkThrow()", m_ExceptionThrown);
  }

  void TestMitkThrowMacroMessage_Success()
  {
    // case 2: test message text
    try
    {
      this->throwExceptionWithThrowMacro("test123");
    }
    catch (const mitk::Exception &e)
    {
      m_ExceptionThrown = true;
      m_MessageText = e.GetDescription();
    }
    CPPUNIT_ASSERT_MESSAGE("Testing message test of mitkThrow()", (m_ExceptionThrown && (m_MessageText == "test123")));
  }

  void TestMitkThrowMacroException_Success()
  {
    // case 3: specialized exception / command mitkThrow(mitk::Exception)
    try
    {
      this->throwSpecializedExceptionWithThrowMacro("test123");
    }
    catch (const mitk::Exception &e)
    {
      m_ExceptionThrown = true;
      m_MessageText = e.GetDescription();
    }
    CPPUNIT_ASSERT_MESSAGE("Testing special exception with mitkThrow(mitk::Exception)",
                           m_ExceptionThrown && m_MessageText == "test123");
  }

  void TestMitkThrowMacroSpezcializedException()
  {
    // case 4: specialized exception / command mitkThrow(mitk::SpecializedException)
    try
    {
      this->throwSpecializedExceptionWithThrowMacro2("test123");
    }
    catch (const SpecializedTestException &e)
    {
      m_ExceptionThrown = true;
      m_MessageText = e.GetDescription();
    }
    CPPUNIT_ASSERT_MESSAGE("Testing special exception with mitkThrow(mitk::SpecializedException)",
                           m_ExceptionThrown && m_MessageText == "test123");
  }

  //#####  this methods are ONLY to test methods of mitk::Exception and no code example
  //#####  normally exceptions should only be instantiated and thrown by using the exception macros!
  void TestGetNumberOfRethrows_Success()
  {
    // first: testing rethrow information methods, when no information is stored
    // case 1.1: method GetNumberOfRethrows()
    CPPUNIT_ASSERT_MESSAGE("Testing GetNumberOfRethrows() with empty rethrow information",
                           m_E.GetNumberOfRethrows() == 0);
  }

  void TestGetRethrowDataWithNegativNumber_Success()
  {
    // case 1.2: GetRethrowData() with negative number
    m_E.GetRethrowData(-1, m_File, m_Line, m_Message);
    CPPUNIT_ASSERT_MESSAGE("Testing GetRethrowData() with invalid rethrow number (negative).",
                           ((m_File == "") && (m_Line == 0) && (m_Message == "")));
  }

  void TestGetRethrowDataWithNumberZero_Success()
  {
    // case 1.3: GetRethrowData() with number 0
    m_E.GetRethrowData(0, m_File, m_Line, m_Message);
    CPPUNIT_ASSERT_MESSAGE("Testing GetRethrowData() with non-existing rethrow number (0).",
                           ((m_File == "") && (m_Line == 0) && (m_Message == "")));
  }

  void TestGetRethrowDataWithNumberOne_Success()
  {
    // case 1.4: GetRethrowData() with number 1
    m_E.GetRethrowData(1, m_File, m_Line, m_Message);
    CPPUNIT_ASSERT_MESSAGE("Testing GetRethrowData() with non-existing rethrow number (1).",
                           ((m_File == "") && (m_Line == 0) && (m_Message == "")));
  }

  void TestAddRethrowData_Success()
  {
    // second: add rethrow data
    m_E.AddRethrowData("test2.cpp", 10, "Rethrow one");
    CPPUNIT_ASSERT_MESSAGE("Testing adding of rethrow data.", m_E.GetNumberOfRethrows() == 1);
    m_E.AddRethrowData("test3.cpp", 15, "Rethrow two");
    CPPUNIT_ASSERT_MESSAGE("Testing adding of more rethrow data.", m_E.GetNumberOfRethrows() == 2);
  }

  void TestFirstRethrowDataAreStoredProperly_Success()
  {
    // third: test if this rethrow data was stored properly
    m_E.AddRethrowData("test2.cpp", 10, "Rethrow one");
    m_E.GetRethrowData(0, m_File, m_Line, m_Message);
    CPPUNIT_ASSERT_MESSAGE("Testing stored information of first rethrow.",
                           ((m_File == "test2.cpp") && (m_Line == 10) && (m_Message == "Rethrow one")));
  }

  void TestSecondRethrowDataAreStoredProperly_Success()
  {
    m_E.AddRethrowData("test2.cpp", 10, "Rethrow one");
    m_E.AddRethrowData("test3.cpp", 15, "Rethrow two");
    m_E.GetRethrowData(1, m_File, m_Line, m_Message);
    CPPUNIT_ASSERT_MESSAGE("Testing stored information of second rethrow.",
                           ((m_File == "test3.cpp") && (m_Line == 15) && (m_Message == "Rethrow two")));
  }

  void TestRethrowMacro_Success()
  {
    // case 1: test throwing
    try
    {
      this->reThrowExceptionWithReThrowMacro("Test original message.", "Test rethrow message.");
    }
    catch (const mitk::Exception &e)
    {
      m_Message = e.GetDescription();
      m_ExceptionThrown = true;
    }
    CPPUNIT_ASSERT_MESSAGE("Testing mitkReThrow()", m_ExceptionThrown);
    CPPUNIT_ASSERT_MESSAGE("Testing message/descriprion after rethrow.",
                           m_Message == "Test original message.Test rethrow message.");
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkException)
