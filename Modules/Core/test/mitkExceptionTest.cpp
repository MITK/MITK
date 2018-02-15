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

//Testing
#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"

// std includes
#include <string>

// MITK includes 
#include "mitkExceptionMacro.h"
#include "mitkTestingMacros.h"
#include <mitkCommon.h>
#include "mitkException.h"

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


class mitkExceptionTestSuite :  public itk::Object,  public mitk::TestFixture
{
	CPPUNIT_TEST_SUITE(mitkExceptionTestSuite);

	MITK_TEST(TestExceptionConstructor_Success);
	MITK_TEST(TestExceptionMessageStream_Success);
	MITK_TEST(TestExceptionMessageStreamThrowing_Success);
	MITK_TEST(TestMitkThrowMacro_Success);
	MITK_TEST(TestRethrowInformation_Success);
	MITK_TEST(TestRethrowMacro_Success);

	CPPUNIT_TEST_SUITE_END();
	
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
	
	void setUp()
	{

	}

	void tearDown()
	{

	}

    void TestExceptionConstructor_Success()
	{
		bool exceptionThrown = false;
		mitkExceptionTestSuite::Pointer myExceptionTestObject = mitkExceptionTestSuite::New();
		try
		{
		 myExceptionTestObject->throwExceptionManually();
		}
		catch (mitk::Exception)
		{
			exceptionThrown = true;
		}
		CPPUNIT_ASSERT_MESSAGE("Testing constructor of mitkException", exceptionThrown);
	
		exceptionThrown = false;
		try
		{
			myExceptionTestObject->throwSpecializedExceptionManually();
		}
		catch (SpecializedTestException)
		{
			exceptionThrown = true;
		}
		CPPUNIT_ASSERT_MESSAGE("Testing constructor specialized exception (deriving from mitkException)", exceptionThrown);
	}
	


	void TestExceptionMessageStream_Success()
	{
		//##### this method is ONLY to test the streaming operators of the exceptions and
		//##### NO code example. Please do not instantiate exceptions by yourself in normal code!
		//##### Normally exceptions should only be thrown by using the exception macro!
		mitk::Exception myException = mitk::Exception("testfile.cpp", 111, "testmessage");
		myException << " and additional stream";
		CPPUNIT_ASSERT_MESSAGE("Testing mitkException message stream (adding std::string)", myException.GetDescription() == std::string("testmessage and additional stream"));
	
		myException.SetDescription("testmessage2");
		myException << ' ' << 'a' << 'n' << 'd' << ' ' << 'c' << 'h' << 'a' << 'r' << 's';
		CPPUNIT_ASSERT_MESSAGE("Testing mitkException message stream (adding single chars)", myException.GetDescription() == std::string("testmessage2 and chars"));
	
		myException.SetDescription("testmessage3");
		myException << myException; // adding the object itself makes no sense but should work
		CPPUNIT_ASSERT_MESSAGE("Testing mitkException message stream (adding object)", myException.GetDescription() != std::string(""));
	
		SpecializedTestException mySpecializedException =
			SpecializedTestException("testfile.cpp", 111, "testmessage", "test");
		mySpecializedException << " and additional stream";
		CPPUNIT_ASSERT_MESSAGE("Testing specialized exception message stream (adding std::string)", 
			mySpecializedException.GetDescription() == std::string("testmessage and additional stream"));
	}
	
	void TestExceptionMessageStreamThrowing_Success()
	{
		bool exceptionThrown = false;
		mitkExceptionTestSuite::Pointer myExceptionTestObject = mitkExceptionTestSuite::New();
		std::string thrownMessage = "";
		try
		{
			 myExceptionTestObject->throwExceptionManually("message1", " and message2");
		}
		catch (mitk::Exception &e)
		{
			 thrownMessage = e.GetDescription();
			 exceptionThrown = true;
		}
		CPPUNIT_ASSERT_MESSAGE("Testing throwing and streaming of mitk::Exception together.", exceptionThrown && (thrownMessage == std::string("message1 and message2")));
	}
	
	void TestMitkThrowMacro_Success()
	{
		bool exceptionThrown = false;
		mitkExceptionTestSuite::Pointer myExceptionTestObject = mitkExceptionTestSuite::New();
	
		// case 1: test throwing
	
		try
		{
			 myExceptionTestObject->throwExceptionWithThrowMacro();
		}
		catch (mitk::Exception)
		{
			exceptionThrown = true;
		}
		CPPUNIT_ASSERT_MESSAGE("Testing mitkThrow()", exceptionThrown);
	
		// case 2: test message text
	
		exceptionThrown = false;
		std::string messageText = "";
	
		try
		{
			myExceptionTestObject->throwExceptionWithThrowMacro("test123");
		}
		catch (mitk::Exception &e)
		{
			exceptionThrown = true;
			messageText = e.GetDescription();
		}
		CPPUNIT_ASSERT_MESSAGE("Testing message test of mitkThrow()", (exceptionThrown && (messageText == "test123")));
	
		// case 3: specialized exception / command mitkThrow(mitk::Exception)
	
		exceptionThrown = false;
		messageText = "";
	
		try
		{
			myExceptionTestObject->throwSpecializedExceptionWithThrowMacro("test123");
		}
		catch (mitk::Exception &e)
		{
			exceptionThrown = true;
			messageText = e.GetDescription();
		}
		CPPUNIT_ASSERT_MESSAGE("Testing special exception with mitkThrow(mitk::Exception)", exceptionThrown && messageText == "test123");
	
		// case 4: specialized exception / command mitkThrow(mitk::SpecializedException)
	
		exceptionThrown = false;
		messageText = "";
	
		try
		{
			myExceptionTestObject->throwSpecializedExceptionWithThrowMacro2("test123");
		}
		catch (SpecializedTestException &e)
		{
			exceptionThrown = true;
			messageText = e.GetDescription();
		}
		CPPUNIT_ASSERT_MESSAGE("Testing special exception with mitkThrow(mitk::SpecializedException)", exceptionThrown && messageText == "test123");
	}
	
	void TestRethrowInformation_Success()
	// this method is ONLY to test methods of mitk::Exception and no code example
	// normally exceptions should only be instantiated and thrown by using the exception macros!
	{
		// first: testing rethrow information methods, when no information is stored
	
		// case 1.1: method GetNumberOfRethrows()
		mitk::Exception e = mitk::Exception("test.cpp", 155, "", "");
		CPPUNIT_ASSERT_MESSAGE("Testing GetNumberOfRethrows() with empty rethrow information", e.GetNumberOfRethrows() == 0);
	
		// case 1.2: GetRethrowData() with negative number
		{
			std::string file = "invalid";
			int line = -1;
			std::string message = "invalid";
			e.GetRethrowData(-1, file, line, message);
			CPPUNIT_ASSERT_MESSAGE("Testing GetRethrowData() with invalid rethrow number (negative).", ((file == "") && (line == 0) && (message == "")));
		}
	
		// case 1.3: GetRethrowData() with number 0
		{
			std::string file = "invalid";
			int line = -1;
			std::string message = "invalid";
			e.GetRethrowData(0, file, line, message);
			CPPUNIT_ASSERT_MESSAGE("Testing GetRethrowData() with non-existing rethrow number (0).", ((file == "") && (line == 0) && (message == "")));
		}
	
		// case 1.4: GetRethrowData() with number 1
		{
			std::string file = "invalid";
			int line = -1;
			std::string message = "invalid";
			e.GetRethrowData(1, file, line, message);
			CPPUNIT_ASSERT_MESSAGE("Testing GetRethrowData() with non-existing rethrow number (1).", ((file == "") && (line == 0) && (message == "")));
		}
	
		// second: add rethrow data
		e.AddRethrowData("test2.cpp", 10, "Rethrow one");
		CPPUNIT_ASSERT_MESSAGE("Testing adding of rethrow data.", e.GetNumberOfRethrows() == 1);
		e.AddRethrowData("test3.cpp", 15, "Rethrow two");
		CPPUNIT_ASSERT_MESSAGE("Testing adding of more rethrow data.", e.GetNumberOfRethrows() == 2);
	
		// third: test if this rethrow data was stored properly
		{
			std::string file = "invalid";
			int line = -1;
			std::string message = "invalid";
			e.GetRethrowData(0, file, line, message);
			CPPUNIT_ASSERT_MESSAGE("Testing stored information of first rethrow.", ((file == "test2.cpp") && (line == 10) && (message == "Rethrow one")));
		}
	
		{
			std::string file = "invalid";
			int line = -1;
			std::string message = "invalid";
			e.GetRethrowData(1, file, line, message);
			CPPUNIT_ASSERT_MESSAGE("Testing stored information of second rethrow.", ((file == "test3.cpp") && (line == 15) && (message == "Rethrow two")));
		}
	}
	
	void TestRethrowMacro_Success()
	{
		bool exceptionThrown = false;
		std::string message = "";
		mitkExceptionTestSuite::Pointer myExceptionTestObject = mitkExceptionTestSuite::New();
	
		// case 1: test throwing
	
		try
		{
			myExceptionTestObject->reThrowExceptionWithReThrowMacro("Test original message.", "Test rethrow message.");
		}
		catch (mitk::Exception &e)
		{
			message = e.GetDescription();
			exceptionThrown = true;
		}
		CPPUNIT_ASSERT_MESSAGE("Testing mitkReThrow()", exceptionThrown);
		CPPUNIT_ASSERT_MESSAGE("Testing message/descriprion after rethrow.", message == "Test original message.Test rethrow message.");
	}
};

MITK_TEST_SUITE_REGISTRATION(mitkException)
