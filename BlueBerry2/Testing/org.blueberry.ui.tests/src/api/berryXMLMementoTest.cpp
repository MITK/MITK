/*=========================================================================
 
 Program:   BlueBerry Platform
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

#define NOMINMAX

#include "berryXMLMementoTest.h"

#include <CppUnit/TestSuite.h>
#include <CppUnit/TestCaller.h>
#include <sstream>
#include <fstream>
#include <limits>

namespace berry
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

  void PrepareAndCheckBeforeSerialization(
      XMLMemento::Pointer mementoToSerialize)
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

struct CopyChildChecker: public XMLMementoTest::MementoChecker
{

  CopyChildChecker(XMLMementoTest* t) :
    mementoTest(t)
  {
  }

  void PrepareAndCheckBeforeSerialization(
      XMLMemento::Pointer mementoToSerialize)
  {
    IMemento::Pointer child = mementoToSerialize->CreateChild("c", "i");
    mementoTest->FillMemento(child);
    IMemento::Pointer copiedChild = mementoToSerialize->CopyChild(child);
    assertEqual("i", copiedChild->GetID());
    mementoTest->CheckMemento(copiedChild, true);
  }

  void CheckAfterDeserialization(XMLMemento::Pointer deserializedMemento)
  {
    IMemento::Pointer child = deserializedMemento->GetChild("c");
    mementoTest->CheckMemento(child, true);
    std::vector<IMemento::Pointer> children(deserializedMemento->GetChildren(
        "c"));
    assertEqual(2, children.size());
    assertEqual("i", children[0]->GetID());
    mementoTest->CheckMemento(children[0], true);
    assertEqual("i", children[1]->GetID());
    mementoTest->CheckMemento(children[1], true);
  }

private:

  XMLMementoTest* mementoTest;
};

struct CreateAndGetChildChecker: public XMLMementoTest::MementoChecker
{

  CreateAndGetChildChecker() :
    type1("type1"), type2("type2"), id("id")
  {
  }

  void PrepareAndCheckBeforeSerialization(
      XMLMemento::Pointer mementoToSerialize)
  {
    // check that nothing is there yet
    assertNullPtr(mementoToSerialize->GetChild(type1).GetPointer());
    assertNullPtr(mementoToSerialize->GetChild(type2).GetPointer());

    // creation without ID
    IMemento::Pointer child1 = mementoToSerialize->CreateChild(type1);
    assertNotNullPtr(child1.GetPointer());
    assertNotNullPtr(mementoToSerialize->GetChild(type1).GetPointer());

    // creation with ID
    IMemento::Pointer child2 = mementoToSerialize->CreateChild(type2, id);
    assertNotNullPtr(child2.GetPointer());
    assertNotNullPtr(mementoToSerialize->GetChild(type2).GetPointer());
    assertEqual(id, child2->GetID());
  }

  void CheckAfterDeserialization(XMLMemento::Pointer deserializedMemento)
  {
    IMemento::Pointer child1 = deserializedMemento->GetChild(type1);
    assertNotNullPtr(child1.GetPointer());
    IMemento::Pointer child2 = deserializedMemento->GetChild(type2);
    assertNotNullPtr(child2.GetPointer());
    assertEqual(id, child2->GetID());
  }

private:

  const std::string type1;
  const std::string type2;
  const std::string id;
};

struct GetChildrenChecker: public XMLMementoTest::MementoChecker
{

  GetChildrenChecker() :
    type("type"), id1("id"), id2("id2")
  {
  }

  void PrepareAndCheckBeforeSerialization(
      XMLMemento::Pointer mementoToSerialize)
  {
    // check that nothing is there yet
    assertNullPtr(mementoToSerialize->GetChild(type).GetPointer());

    IMemento::Pointer child1 = mementoToSerialize->CreateChild(type, id1);
    assertNotNullPtr(child1.GetPointer());
    assertNotNullPtr(mementoToSerialize->GetChild(type).GetPointer());
    assertEqual(id1, child1->GetID());

    // second child with the same type
    IMemento::Pointer child2 = mementoToSerialize->CreateChild(type, id2);
    assertNotNullPtr(child2.GetPointer());
    assertEqual(2, mementoToSerialize->GetChildren(type).size());
    assertEqual(id2, child2->GetID());
  }

  void CheckAfterDeserialization(XMLMemento::Pointer deserializedMemento)
  {
    std::vector<IMemento::Pointer> children(deserializedMemento->GetChildren(
        type));
    assertEqual(2, children.size());

    // this checks that the order is maintained.
    // the spec does not promise this, but clients
    // may rely on the current implementation behaviour.
    assertEqual(id1, children[0]->GetID());
    assertEqual(id2, children[1]->GetID());
  }

private:
  const std::string type;
  const std::string id1;
  const std::string id2;
};

struct GetIDChecker: public XMLMementoTest::MementoChecker
{

  GetIDChecker(const std::string& type, const std::string& id) :
    type(type), id(id)
  {
  }

  void PrepareAndCheckBeforeSerialization(
      XMLMemento::Pointer mementoToSerialize)
  {
    assertNullPtr(mementoToSerialize->GetChild(type).GetPointer());
    IMemento::Pointer child = mementoToSerialize->CreateChild(type, id);
    assertEqual(id, child->GetID());
  }

  void CheckAfterDeserialization(XMLMemento::Pointer deserializedMemento)
  {
    IMemento::Pointer child = deserializedMemento->GetChild(type);
    assertNotNullPtr(child.GetPointer());
    assertEqual(id, child->GetID());
  }

private:

  const std::string type;
  const std::string id;
};

struct PutAndGetFloatChecker: public XMLMementoTest::MementoChecker
{

  PutAndGetFloatChecker(const std::string& key, double value) :
    key(key), value(value)
  {
  }

  void PrepareAndCheckBeforeSerialization(
      XMLMemento::Pointer mementoToSerialize)
  {
    double v;
    assertEqual(false, mementoToSerialize->GetFloat(key, v));
    mementoToSerialize->PutFloat(key, value);
    mementoToSerialize->GetFloat(key, v);
    assertEqual(value, v);
  }

  void CheckAfterDeserialization(XMLMemento::Pointer deserializedMemento)
  {
    double v;
    deserializedMemento->GetFloat(key, v);
    assertEqual(value, v);
  }

private:
  const std::string key;
  const double value;
};

struct PutAndGetIntChecker: public XMLMementoTest::MementoChecker
{

  PutAndGetIntChecker(const std::string& key, const int value) :
    key(key), value(value)
  {
  }

  void PrepareAndCheckBeforeSerialization(
      XMLMemento::Pointer mementoToSerialize)
  {
    int v;
    assertEqual(false, mementoToSerialize->GetInteger(key, v));
    mementoToSerialize->PutInteger(key, value);
    mementoToSerialize->GetInteger(key, v);
    assertEqual(value, v);
  }

  void CheckAfterDeserialization(XMLMemento::Pointer deserializedMemento)
  {
    int v;
    deserializedMemento->GetInteger(key, v);
    assertEqual(value, v);
  }

private:
  const std::string key;
  const int value;
};

struct PutAndGetTextDataChecker: public XMLMementoTest::MementoChecker
{

  PutAndGetTextDataChecker(const std::string& data) :
    data(data)
  {
  }

  void PrepareAndCheckBeforeSerialization(
      XMLMemento::Pointer mementoToSerialize)
  {
    assertEqual("", mementoToSerialize->GetTextData());
    mementoToSerialize->PutTextData(data);
    assertEqual(data, mementoToSerialize->GetTextData());
  }

  void CheckAfterDeserialization(XMLMemento::Pointer deserializedMemento)
  {
    assertEqual(data, deserializedMemento->GetTextData());
  }

private:
  const std::string data;
};

struct LegalKeysChecker: public XMLMementoTest::MementoChecker
{

  LegalKeysChecker(const std::string& key) :
    key(key)
  {
  }

  void PrepareAndCheckBeforeSerialization(
      XMLMemento::Pointer mementoToSerialize)
  {
    std::string v;
    assertEqual(false, mementoToSerialize->GetString(key, v));
    try
    {
      mementoToSerialize->PutString(key, "some string");
    } catch (Poco::RuntimeException& ex)
    {
      std::cout << "offending key: '" + key + "'" << std::endl;
      ex.rethrow();
    }
    mementoToSerialize->GetString(key, v);
    assertEqual("some string", v);
  }

  void CheckAfterDeserialization(XMLMemento::Pointer deserializedMemento)
  {
    std::string v;
    deserializedMemento->GetString(key, v);
    assertEqual("some string", v);
  }

private:
  const std::string key;
};

struct PutTextDataWithChildrenChecker: public XMLMementoTest::MementoChecker
{

  PutTextDataWithChildrenChecker(const std::string& textData) :
    textData(textData)
  {
  }

  void PrepareAndCheckBeforeSerialization(
      XMLMemento::Pointer mementoToSerialize)
  {
    mementoToSerialize->CreateChild("type", "id");
    mementoToSerialize->PutTextData(textData);
    mementoToSerialize->CreateChild("type", "id");
    mementoToSerialize->CreateChild("type", "id");
    assertEqual(textData, mementoToSerialize->GetTextData());
  }

  void CheckAfterDeserialization(XMLMemento::Pointer deserializedMemento)
  {
    assertEqual(textData, deserializedMemento->GetTextData());
  }

private:
  const std::string textData;
};

const std::string XMLMementoTest::TEST_STRINGS[10] =
{ "value", " value with spaces ", "value.with.many.dots",
    "value_with_underscores", "value<with<lessthan", "value>with>greaterthan",
    "value&with&ampersand", "value\"with\"quote", "value#with#hash", "", };

const unsigned int XMLMementoTest::TEST_STRINGS_LENGTH = 10;
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
  CppUnit_addTest(suite, XMLMementoTest, TestCreateReadRootReaderExceptionCases);
  CppUnit_addTest(suite, XMLMementoTest, TestCreateReadRootReader);
  CppUnit_addTest(suite, XMLMementoTest, TestCreateWriteRoot);
  CppUnit_addTest(suite, XMLMementoTest, TestSpacesInRootAreIllegal);
  CppUnit_addTest(suite, XMLMementoTest, TestSpacesInKeysAreIllegal);
  CppUnit_addTest(suite, XMLMementoTest, TestCopyChild);
  CppUnit_addTest(suite, XMLMementoTest, TestCreateAndGetChild);
  CppUnit_addTest(suite, XMLMementoTest, TestGetChildren);
  CppUnit_addTest(suite, XMLMementoTest, TestGetID);
  CppUnit_addTest(suite, XMLMementoTest, TestPutAndGetFloat);
  CppUnit_addTest(suite, XMLMementoTest, TestPutAndGetInteger);
  CppUnit_addTest(suite, XMLMementoTest, TestPutAndGetTextData);
  CppUnit_addTest(suite, XMLMementoTest, TestLegalKeys);
  CppUnit_addTest(suite, XMLMementoTest, TestIllegalKeys);
  CppUnit_addTest(suite, XMLMementoTest, TestPutTextDataWithChildren);
  CppUnit_addTest(suite, XMLMementoTest, TestMementoWithTextContent);

  return suite;
}

XMLMementoTest::XMLMementoTest(const std::string& testName) :
  TestCase(testName)
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
  assertEqualDelta(0.4, floatVal, 0.0001);

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

  for (unsigned int i = 0; i < TEST_STRINGS_LENGTH; ++i)
  {
    const std::string value;
    PutAndGetStringChecker checker(key, TEST_STRINGS[i]);
    TestPutAndGet(checker);
  }
}

void XMLMementoTest::TestCreateReadRootReaderExceptionCases()
{
  try
  {
    std::stringstream ss("Invalid format");

    XMLMemento::CreateReadRoot(ss);
    failmsg("should throw WorkbenchException because of invalid format");
  } catch (WorkbenchException e)
  {
    // expected
  }
  try
  {
    std::stringstream ss("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>");
    XMLMemento::CreateReadRoot(ss);
    failmsg("should throw WorkbenchException because there is no element");
  } catch (WorkbenchException e)
  {
    // expected
  }
  try
  {
    std::ifstream file("__123thisshouldnotexist_238waer");
    XMLMemento::CreateReadRoot(file);
    failmsg("should throw WorkbenchException because of IOException");
  } catch (WorkbenchException e)
  {
    // expected
  }
}

void XMLMementoTest::TestCreateReadRootReader() throw (WorkbenchException)
{
  std::stringstream
      ss(
          "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?><simple>some text data</simple>");
  XMLMemento::Pointer memento = XMLMemento::CreateReadRoot(ss);
  assertEqual("some text data", memento->GetTextData());
}

void XMLMementoTest::TestCreateWriteRoot()
{
  std::string rootTypes[] =
  { "type", "type.with.dots", "type_with_underscores" };
  for (unsigned int i = 0; i < 3; i++)
  {
    XMLMemento::Pointer memento = XMLMemento::CreateWriteRoot(rootTypes[i]);
    assertNotNullPtr(memento.GetPointer());
  }
}

void XMLMementoTest::TestSpacesInRootAreIllegal()
{
  try
  {
    XMLMemento::CreateWriteRoot("with space");
    failmsg("should fail");
  } catch (std::exception& e)
  {
    // expected
  }
}

void XMLMementoTest::TestSpacesInKeysAreIllegal()
{
  XMLMemento::Pointer memento = XMLMemento::CreateWriteRoot("foo");
  try
  {
    memento->CreateChild("with space", "bar");
    failmsg("should fail");
  } catch (std::exception& e)
  {
    // expected
  }
  try
  {
    memento->PutString("with space", "bar");
    failmsg("should fail");
  } catch (std::exception& e)
  {
    // expected
  }
}

void XMLMementoTest::TestCopyChild() throw (WorkbenchException,
    Poco::IOException)
{
  CopyChildChecker checker(this);
  TestPutAndGet(checker);
}

void XMLMementoTest::TestCreateAndGetChild() throw (WorkbenchException,
    Poco::IOException)
{
  CreateAndGetChildChecker checker;
  TestPutAndGet(checker);
}

void XMLMementoTest::TestGetChildren() throw (WorkbenchException,
    Poco::IOException)
{
  GetChildrenChecker checker;
  TestPutAndGet(checker);
}

void XMLMementoTest::TestGetID() throw (WorkbenchException, Poco::IOException)
{
  const std::string type = "type";

  const std::string ids[] =
  { "id", "", "id.with.many.dots", "id_with_underscores", "id<with<lessthan",
      "id>with>greaterthan", "id&with&ampersand", "id\"with\"quote",
      "id#with#hash" };

  for (unsigned int i = 0; i < 9; i++)
  {
    GetIDChecker checker(type, ids[i]);
    TestPutAndGet(checker);
  }
}

void XMLMementoTest::TestPutAndGetFloat() throw (WorkbenchException,
    Poco::IOException)
{
  const std::string key = "key";

  const double values[] =
  { -3.1415, 1, 0, 4554.45235, std::numeric_limits<double>::max(),
      std::numeric_limits<double>::min(),
      std::numeric_limits<double>::quiet_NaN(),
      std::numeric_limits<double>::infinity() };

  for (unsigned int i = 0; i < 8; i++)
  {
    PutAndGetFloatChecker checker(key, values[i]);
    TestPutAndGet(checker);
  }
}

void XMLMementoTest::TestPutAndGetInteger() throw (WorkbenchException,
    Poco::IOException)
{
  const std::string key = "key";

  const int values[] =
  { 36254, 0, 1, -36254, std::numeric_limits<int>::max(), std::numeric_limits<
      int>::min() };

  for (unsigned int i = 0; i < 6; i++)
  {
    PutAndGetIntChecker checker(key, values[i]);
    TestPutAndGet(checker);
  }

}

void XMLMementoTest::TestPutAndGetTextData() throw (WorkbenchException,
    Poco::IOException)
{
  for (unsigned int i = 0; i < TEST_STRINGS_LENGTH; i++)
  {
    PutAndGetTextDataChecker checker(TEST_STRINGS[i]);
    TestPutAndGet(checker);
  }
}

void XMLMementoTest::TestLegalKeys() throw (WorkbenchException,
    Poco::IOException)
{
  const std::string legalKeys[] =
  { "value", "value.with.many.dots", "value_with_underscores" };

  for (unsigned int i = 0; i < 3; i++)
  {
    LegalKeysChecker checker(legalKeys[i]);
    TestPutAndGet(checker);
  }

}

void XMLMementoTest::TestIllegalKeys() throw ()
{
  const std::string illegalKeys[] =
  { "", " ", " key", "key ", "key key", "\t", "\tkey", "key\t", "key\tkey",
      "\n", "\nkey", "key\n", "key\nkey", "key<with<lessthan",
      "key>with>greaterthan", "key&with&ampersand", "key#with#hash",
      "key\"with\"quote", "\"" };

  for (unsigned int i = 0; i < 19; i++)
  {
    XMLMemento::Pointer memento = XMLMemento::CreateWriteRoot("foo");
    try
    {
      memento->PutString(illegalKeys[i], "some string");
      failmsg("putString with illegal key should fail");
    } catch (std::exception& ex)
    {
      // expected
    }
  }
}

void XMLMementoTest::TestPutTextDataWithChildren() throw (WorkbenchException,
    Poco::IOException)
{
  const std::string textData =
      "\n\tThis is\ntext data\n\t\twith newlines and \ttabs\t\n\t ";
  PutTextDataWithChildrenChecker checker(textData);
  TestPutAndGet(checker);
}

void XMLMementoTest::TestMementoWithTextContent()
{
  IMemento::Pointer memento = XMLMemento::CreateWriteRoot("root");
  IMemento::Pointer mementoWithChild = XMLMemento::CreateWriteRoot("root");
  IMemento::Pointer child = mementoWithChild->CreateChild("child");
  child->PutTextData("text");
  memento->PutMemento(mementoWithChild);
  IMemento::Pointer copiedChild = memento->GetChild("child");
  assertEqual("text", copiedChild->GetTextData());
}

}
