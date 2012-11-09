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


#include "mitkExceptionMacro.h"
#include "mitkTestingMacros.h"
#include <itkObject.h>
#include <itkObjectFactory.h>
#include <mitkCommon.h>

class SpecializedTestException : public mitk::Exception
  {
  public:
    mitkExceptionClassMacro(SpecializedTestException,mitk::Exception);
  };

class ExceptionTestClass : public itk::Object
{
public:
  mitkClassMacro( ExceptionTestClass , itk::Object );
  itkNewMacro(Self);

  void throwExceptionManually()
  //this method is ONLY to test the constructor and no code example
  //normally exceptions should only be thrown by using the exception macro!
  {
  throw mitk::Exception("test.cpp",155,"","");
  }

  void throwSpecializedExceptionManually()
  //this method is ONLY to test the constructor and no code example
  //normally exceptions should only be thrown by using the exception macro!
  {
  throw SpecializedTestException("test.cpp",155,"","");
  }

  void throwExceptionManually(std::string message1, std::string message2)
  //this method is ONLY to test methods of mitk::Exception and no code example
  //normally exceptions should only be thrown by using the exception macro!
  {
  throw mitk::Exception("testfile.cpp",155,message1.c_str(),"") << message2;
  }

  void throwExceptionWithThrowMacro()
  {
  mitkThrow()<<"TEST EXCEPION THROWING WITH mitkThrow()";
  }

  void throwExceptionWithThrowMacro(std::string message)
  {
  mitkThrow()<<message.c_str();
  }

  void throwSpecializedExceptionWithThrowMacro(std::string message)
  {
  mitkThrowException(mitk::Exception)<<message;
  }

  void throwSpecializedExceptionWithThrowMacro2(std::string message)
  {
  mitkThrowException(SpecializedTestException)<<message;
  }

  void reThrowExceptionWithReThrowMacro(std::string messageThrow, std::string messageReThrow)
  {
  try
    {
    throwExceptionWithThrowMacro(messageThrow);
    }
  catch(mitk::Exception e)
    {
    mitkReThrow(e) << messageReThrow;
    }

  }

  static void TestExceptionConstructor()
    {
    bool exceptionThrown = false;
    ExceptionTestClass::Pointer myExceptionTestObject = ExceptionTestClass::New();
    try
       {
       myExceptionTestObject->throwExceptionManually();
       }
    catch(mitk::Exception)
       {
       exceptionThrown = true;
       }
    MITK_TEST_CONDITION_REQUIRED(exceptionThrown,"Testing constructor of mitkException");


    exceptionThrown = false;
    try
       {
       myExceptionTestObject->throwSpecializedExceptionManually();
       }
    catch(SpecializedTestException)
       {
       exceptionThrown = true;
       }
    MITK_TEST_CONDITION_REQUIRED(exceptionThrown,"Testing constructor specialized exception (deriving from mitkException)");
    }

  static void TestExceptionMessageStream()
    {
    //##### this method is ONLY to test the streaming operators of the exceptions and
    //##### NO code example. Please do not instantiate exceptions by yourself in normal code!
    //##### Normally exceptions should only be thrown by using the exception macro!
    mitk::Exception myException = mitk::Exception("testfile.cpp",111,"testmessage");
    myException << " and additional stream";
    MITK_TEST_CONDITION_REQUIRED(myException.GetDescription() == std::string("testmessage and additional stream"),"Testing mitkException message stream (adding std::string)");

    myException.SetDescription("testmessage2");
    myException << ' ' << 'a' << 'n' << 'd' << ' ' << 'c' << 'h' << 'a' << 'r' << 's';
    MITK_TEST_CONDITION_REQUIRED(myException.GetDescription() == std::string("testmessage2 and chars"),"Testing mitkException message stream (adding single chars)");

    myException.SetDescription("testmessage3");
    myException << myException; //adding the object itself makes no sense but should work
    MITK_TEST_CONDITION_REQUIRED(myException.GetDescription() != std::string(""),"Testing mitkException message stream (adding object)");

    SpecializedTestException mySpecializedException = SpecializedTestException("testfile.cpp",111,"testmessage","test");
    mySpecializedException << " and additional stream";
    MITK_TEST_CONDITION_REQUIRED(mySpecializedException.GetDescription() == std::string("testmessage and additional stream"),"Testing specialized exception message stream (adding std::string)");
    }

  static void TestExceptionMessageStreamThrowing()
    {
    bool exceptionThrown = false;
    ExceptionTestClass::Pointer myExceptionTestObject = ExceptionTestClass::New();
    std::string thrownMessage = "";
    try
     {
     myExceptionTestObject->throwExceptionManually("message1"," and message2");
     }
    catch(mitk::Exception e)
     {
     thrownMessage = e.GetDescription();
     exceptionThrown = true;
     }
    MITK_TEST_CONDITION_REQUIRED(exceptionThrown && (thrownMessage == std::string("message1 and message2")),"Testing throwing and streaming of mitk::Exception together.")
    }

  static void TestMitkThrowMacro()
    {

    bool exceptionThrown = false;
    ExceptionTestClass::Pointer myExceptionTestObject = ExceptionTestClass::New();

    //case 1: test throwing

    try
     {
     myExceptionTestObject->throwExceptionWithThrowMacro();
     }
    catch(mitk::Exception)
     {
     exceptionThrown = true;
     }
    MITK_TEST_CONDITION_REQUIRED(exceptionThrown,"Testing mitkThrow()");

    //case 2: test message text

    exceptionThrown = false;
    std::string messageText = "";

    try
     {
     myExceptionTestObject->throwExceptionWithThrowMacro("test123");
     }
    catch(mitk::Exception e)
     {
     exceptionThrown = true;
     messageText = e.GetDescription();

     }
    MITK_TEST_CONDITION_REQUIRED((exceptionThrown && (messageText=="test123")),"Testing message test of mitkThrow()");

    //case 3: specialized exception / command mitkThrow(mitk::Exception)

    exceptionThrown = false;
    messageText = "";

    try
     {
     myExceptionTestObject->throwSpecializedExceptionWithThrowMacro("test123");
     }
    catch(mitk::Exception e)
     {
     exceptionThrown = true;
     messageText = e.GetDescription();
     }
    MITK_TEST_CONDITION_REQUIRED(exceptionThrown && messageText=="test123","Testing special exception with mitkThrow(mitk::Exception)");

    //case 4: specialized exception / command mitkThrow(mitk::SpecializedException)

    exceptionThrown = false;
    messageText = "";

    try
     {
     myExceptionTestObject->throwSpecializedExceptionWithThrowMacro2("test123");
     }
    catch(SpecializedTestException e)
     {
     exceptionThrown = true;
     messageText = e.GetDescription();
     }
    MITK_TEST_CONDITION_REQUIRED(exceptionThrown && messageText=="test123","Testing special exception with mitkThrow(mitk::SpecializedException)");

    }

static void TestRethrowInformation()
    //this method is ONLY to test methods of mitk::Exception and no code example
    //normally exceptions should only be instantiated and thrown by using the exception macros!
    {
    //first: testing rethrow information methods, when no information is stored

    //case 1.1: method GetNumberOfRethrows()
    mitk::Exception e = mitk::Exception("test.cpp",155,"","");
    MITK_TEST_CONDITION_REQUIRED(e.GetNumberOfRethrows()==0,"Testing GetNumberOfRethrows() with empty rethrow information");

    //case 1.2: GetRethrowData() with negative number
    {
    std::string file = "invalid";
    int line = -1;
    std::string message = "invalid";
    e.GetRethrowData(-1,file,line,message);
    MITK_TEST_CONDITION_REQUIRED(((file == "")&&(line==0)&&(message == "")),"Testing GetRethrowData() with invalid rethrow number (negative).");
    }

    //case 1.3: GetRethrowData() with number 0
    {
    std::string file = "invalid";
    int line= -1;
    std::string message = "invalid";
    e.GetRethrowData(0,file,line,message);
    MITK_TEST_CONDITION_REQUIRED(((file == "")&&(line==0)&&(message == "")),"Testing GetRethrowData() with non-existing rethrow number (0).");
    }

    //case 1.4: GetRethrowData() with number 1
    {
    std::string file = "invalid";
    int line= -1;
    std::string message = "invalid";
    e.GetRethrowData(1,file,line,message);
    MITK_TEST_CONDITION_REQUIRED(((file == "")&&(line==0)&&(message == "")),"Testing GetRethrowData() with non-existing rethrow number (1).");
    }


    //second: add rethrow data
    e.AddRethrowData("test2.cpp",10,"Rethrow one");
    MITK_TEST_CONDITION_REQUIRED(e.GetNumberOfRethrows()==1,"Testing adding of rethrow data.");
    e.AddRethrowData("test3.cpp",15,"Rethrow two");
    MITK_TEST_CONDITION_REQUIRED(e.GetNumberOfRethrows()==2,"Testing adding of more rethrow data.");

    //third: test if this rethrow data was stored properly
    {
    std::string file = "invalid";
    int line= -1;
    std::string message = "invalid";
    e.GetRethrowData(0,file,line,message);
    MITK_TEST_CONDITION_REQUIRED(((file == "test2.cpp")&&(line==10)&&(message == "Rethrow one")),"Testing stored information of first rethrow.");
    }

    {
    std::string file = "invalid";
    int line= -1;
    std::string message = "invalid";
    e.GetRethrowData(1,file,line,message);
    MITK_TEST_CONDITION_REQUIRED(((file == "test3.cpp")&&(line==15)&&(message == "Rethrow two")),"Testing stored information of second rethrow.");
    }


    }

static void TestRethrowMacro()
    {
    bool exceptionThrown = false;
    std::string message = "";
    ExceptionTestClass::Pointer myExceptionTestObject = ExceptionTestClass::New();

    //case 1: test throwing

    try
     {
     myExceptionTestObject->reThrowExceptionWithReThrowMacro("Test original message.","Test rethrow message.");
     }
    catch(mitk::Exception e)
     {
     message = e.GetDescription();
     exceptionThrown = true;
     }
    MITK_TEST_CONDITION_REQUIRED(exceptionThrown,"Testing mitkReThrow()");
    MITK_TEST_CONDITION_REQUIRED(message == "Test original message.Test rethrow message.", "Testing message/descriprion after rethrow.")

    }
};

int mitkExceptionTest(int /*argc*/, char* /*argv*/[])
{
  MITK_TEST_BEGIN("MITKException");
  ExceptionTestClass::TestExceptionConstructor();
  ExceptionTestClass::TestExceptionMessageStream();
  ExceptionTestClass::TestExceptionMessageStreamThrowing();
  ExceptionTestClass::TestMitkThrowMacro();
  ExceptionTestClass::TestRethrowInformation();
  ExceptionTestClass::TestRethrowMacro();
  MITK_TEST_END();

}
