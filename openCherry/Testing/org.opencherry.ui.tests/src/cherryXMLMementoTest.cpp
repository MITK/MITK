/*=========================================================================
 
 Program:   openCherry Platform
 Language:  C++
 Date:      $Date$
 Version:   $Revision$
 
 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.
 
 =========================================================================*/

#include "cherryXMLMementoTest.h"

#include <CppUnit/TestSuite.h>
#include <CppUnit/TestCaller.h>
#include <sstream>

namespace cherry
{

struct PutMementoChecker: public XMLMementoTest::MementoChecker
{

  PutMementoChecker(XMLMementoTest* t) :
    mementoTest(t)
  {
  }

  void PrepareAndCheckBeforeSerialization(
      XMLMemento::Pointer mementoToSerialize)
  {
    mementoToSerialize->PutTextData("unchanged text data");
    mementoToSerialize->PutString("neverlost", "retained value");

    IMemento::Pointer aMemento = XMLMemento::CreateWriteRoot("foo");
    mementoTest->FillMemento(aMemento);

    // note that this does not copy the text data:
    mementoToSerialize->PutMemento(aMemento);

    // do not check for text data:
    mementoTest->CheckMemento(mementoToSerialize, false);

    assertEqual("unchanged text data", mementoToSerialize->GetTextData());
    std::string str;
    mementoToSerialize->GetString("neverlost", str);
    assertEqual("retained value", str);
  }

  void CheckAfterDeserialization(XMLMemento::Pointer deserializedMemento)
  {
    // do not check for text data:
    mementoTest->CheckMemento(deserializedMemento, false);

    assertEqual("unchanged text data", deserializedMemento
        ->GetTextData());
    std::string str;
    deserializedMemento->GetString("neverlost", str);
    assertEqual("retained value", str);
  }

private:

  XMLMementoTest* mementoTest;
};

struct PutAndGetStringChecker: public XMLMementoTest::MementoChecker
{

  PutAndGetStringChecker(const std::string& key, const std::string& value) :
    key(key), value(value)
  {
  }

  void PrepareAndCheckBeforeSerialization(XMLMemento::Pointer mementoToSerialize)
  {
    std::string helper;
    assertEqual(false, mementoToSerialize->GetString(key, helper));
    mementoToSerialize->PutString(key, value);
    mementoToSerialize->GetString(key, helper);
    assertEqual(value, helper);
  }

  void CheckAfterDeserialization(XMLMemento::Pointer deserializedMemento)
  {
    std::string helper;
    deserializedMemento->GetString(key, helper);
    assertEqual(value, helper);
  }

private:
  const std::string& key;
  const std::string& value;
};

const std::string XMLMementoTest::TEST_STRINGS[10] =
{ "value", " value with spaces ", "value.with.many.dots",
    "value_with_underscores", "value<with<lessthan", "value>with>greaterthan",
    "value&with&ampersand", "value\"with\"quote", "value#with#hash", "", };
    /*
     * the following cases are for bug 93720
     */
//    "\nvalue\nwith\nnewlines\n", "\tvalue\twith\ttab\t",
//    "\rvalue\rwith\rreturn\r", };

CppUnit::Test* XMLMementoTest::Suite()
{
  CppUnit::TestSuite* suite = new CppUnit::TestSuite("XMLMementoTest");

  CppUnit_addTest(suite, XMLMementoTest, TestPutMemento);
  CppUnit_addTest(suite, XMLMementoTest, TestPutAndGetString);

  return suite;
}

XMLMementoTest::XMLMementoTest(const std::string& testName) :
  CppUnit::TestCase(testName)
{

}

void XMLMementoTest::FillMemento(IMemento::Pointer memento)
{
  memento->PutFloat("floatKey", 0.4f);
  memento->PutInteger("integerKey", 324765);
  memento->PutString("stringKey", "a string");
  memento->PutTextData("some text data");
  memento->CreateChild("child1");
  memento->CreateChild("child2", "child2id1");
  memento->CreateChild("child2", "child2id2");
}

void XMLMementoTest::TestPutAndGet(MementoChecker& mementoChecker)
    throw (Poco::IOException, WorkbenchException)
{
  XMLMemento::Pointer mementoToSerialize = XMLMemento::CreateWriteRoot(
      "XMLMementoTest");

  mementoChecker.PrepareAndCheckBeforeSerialization(mementoToSerialize);

  std::stringstream writer;
  mementoToSerialize->Save(writer);
  writer.flush();

  XMLMemento::Pointer deserializedMemento = XMLMemento::CreateReadRoot(writer);

  mementoChecker.CheckAfterDeserialization(deserializedMemento);
}

void XMLMementoTest::CheckMemento(IMemento::Pointer memento,
    bool checkForTextData)
{
  double floatVal;
  memento->GetFloat("floatKey", floatVal);
  assertEqualDelta(0.4, floatVal, 0.0);

  int intVal;
  memento->GetInteger("integerKey", intVal);
  assertEqual(324765, intVal);

  std::string strVal;
  memento->GetString("stringKey", strVal);
  assertEqual("a string", strVal);

  if (checkForTextData)
  {
    assertEqual("some text data", memento->GetTextData());
  }

  IMemento::Pointer child1 = memento->GetChild("child1");
  assertNotNullPtr(child1.GetPointer());
  IMemento::Pointer child2 = memento->GetChild("child2");
  assertNotNullPtr(child2.GetPointer());
  assertEqual("child2id1", child2->GetID());
  std::vector<IMemento::Pointer> children(memento->GetChildren("child2"));
  assertEqual(2, children.size());
  assertEqual("child2id1", children[0]->GetID());
  assertEqual("child2id2", children[1]->GetID());
}

void XMLMementoTest::TestPutMemento() throw (WorkbenchException,
    Poco::IOException)
{
  PutMementoChecker checker(this);
  TestPutAndGet(checker);
}

void XMLMementoTest::TestPutAndGetString() throw (Poco::IOException,
    WorkbenchException)
{
  const std::string key = "key";

  for (unsigned int i = 0; i < 10; ++i)
  {
    const std::string value;
    PutAndGetStringChecker checker(key, TEST_STRINGS[i]);
    TestPutAndGet(checker);
  }
}


}
