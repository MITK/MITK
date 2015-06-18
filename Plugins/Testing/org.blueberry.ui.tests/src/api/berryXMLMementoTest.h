/*===================================================================

BlueBerry Platform

Copyright (c) German Cancer Research Center,
Division of Medical and Biological Informatics.
All rights reserved.

This software is distributed WITHOUT ANY WARRANTY; without
even the implied warranty of MERCHANTABILITY or FITNESS FOR
A PARTICULAR PURPOSE.

See LICENSE.txt or http://www.mitk.org for details.

===================================================================*/

#ifndef BERRYXMLMEMENTOTEST_H_
#define BERRYXMLMEMENTOTEST_H_

#include <berryTestCase.h>
#include <berryXMLMemento.h>

namespace berry
{

/**
 * Testing XMLMemento
 *
 */
class XMLMementoTest: public TestCase
{

private:

  static const std::string TEST_STRINGS[10];
  static const unsigned int TEST_STRINGS_LENGTH;

public:

  static CppUnit::Test* Suite();

  XMLMementoTest(const std::string& testName);

  /*
   * Class under test for XMLMemento createReadRoot(Reader)
   */
  void TestCreateReadRootReaderExceptionCases();

  void TestCreateReadRootReader() throw (WorkbenchException);

  /*
   * Class under test for XMLMemento createReadRoot(Reader, String)
   */
  //   void testCreateReadRootReaderString() {
  //    // TODO - I don't know how to test this. The method is not called by
  //    // anyone as of 2005/04/05.
  //  }

  void TestCreateWriteRoot();

  void TestSpacesInRootAreIllegal();

  void TestSpacesInKeysAreIllegal();

  void TestCopyChild() throw (WorkbenchException, Poco::IOException);

  /**
   * Helper method to fill a memento to be checked later by checkMemento.
   *
   * @param memento
   */
  void FillMemento(IMemento::Pointer memento);

  /**
   * Helper method to check if the values set by fillMemento are still there.
   * The boolean parameter is needed because in some cases
   * (IMememento#putMemento), the text data gets lost.
   *
   * @param memento
   * @param checkForTextData
   */
  void CheckMemento(IMemento::Pointer memento, bool checkForTextData);

  void TestCreateAndGetChild() throw (WorkbenchException, Poco::IOException);

  void TestGetChildren() throw (WorkbenchException, Poco::IOException);

  void TestGetID() throw (WorkbenchException, Poco::IOException);

  void TestPutAndGetFloat() throw (WorkbenchException, Poco::IOException);

  void TestPutAndGetInteger() throw (WorkbenchException, Poco::IOException);

  void TestPutMemento() throw (WorkbenchException, Poco::IOException);

  void TestPutAndGetString() throw (Poco::IOException, WorkbenchException);

  void TestPutAndGetTextData() throw (WorkbenchException, Poco::IOException);

  void TestLegalKeys() throw (WorkbenchException, Poco::IOException);

  void TestIllegalKeys() throw ();

  void TestPutTextDataWithChildren() throw (WorkbenchException,
      Poco::IOException);

  void TestMementoWithTextContent();

  struct MementoChecker: public CppUnit::TestCase
  {

    MementoChecker() :
      CppUnit::TestCase("MementoChecker")
    {
    }

    virtual void PrepareAndCheckBeforeSerialization(
        XMLMemento::Pointer mementoToSerialize) = 0;

    virtual void CheckAfterDeserialization(
        XMLMemento::Pointer deserializedMemento) = 0;
  };

private:

  void TestPutAndGet(MementoChecker& mementoChecker) throw (Poco::IOException,
      WorkbenchException);

};

}

#endif /* BERRYXMLMEMENTOTEST_H_ */
