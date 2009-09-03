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


#ifndef CHERRYXMLMEMENTOTEST_H_
#define CHERRYXMLMEMENTOTEST_H_

#include <CppUnit/TestCase.h>

#include <cherryXMLMemento.h>

namespace cherry {

/**
 * Testing XMLMemento
 *
 */
class XMLMementoTest : public CppUnit::TestCase {

  private:

  static const std::string TEST_STRINGS[10];

  public:

  static CppUnit::Test* Suite();

  XMLMementoTest(const std::string& testName);

  /*
   * Class under test for XMLMemento createReadRoot(Reader)
   */
//   void TestCreateReadRootReaderExceptionCases() {
//    try {
//      XMLMemento.createReadRoot(new StringReader("Invalid format"));
//      fail("should throw WorkbenchException because of invalid format");
//    } catch (WorkbenchException e) {
//      // expected
//    }
//    try {
//      XMLMemento.createReadRoot(new StringReader(
//          "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>"));
//      fail("should throw WorkbenchException because there is no element");
//    } catch (WorkbenchException e) {
//      // expected
//    }
//    try {
//      XMLMemento.createReadRoot(new Reader() {
//
//        public: void close() throws IOException {
//          throw new IOException();
//        }
//
//        public: int read(char[] arg0, int arg1, int arg2)
//            throws IOException {
//          throw new IOException();
//        }
//      });
//      fail("should throw WorkbenchException because of IOException");
//    } catch (WorkbenchException e) {
//      // expected
//    }
//  }

//   void testCreateReadRootReader() throws WorkbenchException {
//    XMLMemento memento = XMLMemento
//        .createReadRoot(new StringReader(
//            "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?><simple>some text data</simple>"));
//    assertEquals("some text data", memento.getTextData());
//  }

  /*
   * Class under test for XMLMemento createReadRoot(Reader, String)
   */
//   void testCreateReadRootReaderString() {
//    // TODO - I don't know how to test this. The method is not called by
//    // anyone as of 2005/04/05.
//  }

//   void testCreateWriteRoot() {
//    std::string[] rootTypes = { "type", "type.with.dots",
//        "type_with_underscores" };
//    for (int i = 0; i < rootTypes.length; i++) {
//      std::string type = rootTypes[i];
//      XMLMemento memento = XMLMemento.createWriteRoot(type);
//      assertNotNull(memento);
//    }
//  }

//   void testSpacesInRootAreIllegal() {
//    try {
//      XMLMemento.createWriteRoot("with space");
//      fail("should fail");
//    } catch (Exception e) {
//      // expected
//    }
//  }

//   void testSpacesInKeysAreIllegal() {
//    XMLMemento memento = XMLMemento.createWriteRoot("foo");
//    try {
//      memento.createChild("with space", "bar");
//      fail("should fail");
//    } catch (Exception e) {
//      // expected
//    }
//    try {
//      memento.putString("with space", "bar");
//      fail("should fail");
//    } catch (Exception e) {
//      // expected
//    }
//  }

//   void testCopyChild() throws WorkbenchException, IOException {
//
//    testPutAndGet(new MementoChecker() {
//
//      public: void prepareAndCheckBeforeSerialization(
//          XMLMemento mementoToSerialize) {
//        IMemento child = mementoToSerialize.createChild("c", "i");
//        fillMemento(child);
//        IMemento copiedChild = mementoToSerialize.copyChild(child);
//        assertEquals("i", copiedChild.getID());
//        checkMemento(copiedChild, true);
//      }

//      public: void checkAfterDeserialization(XMLMemento deserializedMemento) {
//        IMemento child = deserializedMemento.getChild("c");
//        checkMemento(child, true);
//        IMemento[] children = deserializedMemento.getChildren("c");
//        assertEquals(2, children.length);
//        assertEquals("i", children[0].getID());
//        checkMemento(children[0], true);
//        assertEquals("i", children[1].getID());
//        checkMemento(children[1], true);
//      }
//    });
//  }

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

//  public: void testCreateAndGetChild() throws WorkbenchException, IOException {
//    final std::string type1 = "type1";
//    final std::string type2 = "type2";
//    final std::string id = "id";
//
//    testPutAndGet(new MementoChecker() {
//
//      public: void prepareAndCheckBeforeSerialization(
//          XMLMemento mementoToSerialize) {
//        // check that nothing is there yet
//        assertEquals(null, mementoToSerialize.getChild(type1));
//        assertEquals(null, mementoToSerialize.getChild(type2));
//
//        // creation without ID
//        IMemento child1 = mementoToSerialize.createChild(type1);
//        assertNotNull(child1);
//        assertNotNull(mementoToSerialize.getChild(type1));
//
//        // creation with ID
//        IMemento child2 = mementoToSerialize.createChild(type2, id);
//        assertNotNull(child2);
//        assertNotNull(mementoToSerialize.getChild(type2));
//        assertEquals(id, child2.getID());
//      }
//
//      public: void checkAfterDeserialization(XMLMemento deserializedMemento) {
//        IMemento child1 = deserializedMemento.getChild(type1);
//        assertNotNull(child1);
//        IMemento child2 = deserializedMemento.getChild(type2);
//        assertNotNull(child2);
//        assertEquals(id, child2.getID());
//      }
//    });
//  }

//  void testGetChildren() throws WorkbenchException, IOException {
//    final std::string type = "type";
//    final std::string id1 = "id";
//    final std::string id2 = "id2";
//
//    testPutAndGet(new MementoChecker() {
//
//      public: void prepareAndCheckBeforeSerialization(
//          XMLMemento mementoToSerialize) {
//        // check that nothing is there yet
//        assertEquals(null, mementoToSerialize.getChild(type));
//
//        IMemento child1 = mementoToSerialize.createChild(type, id1);
//        assertNotNull(child1);
//        assertNotNull(mementoToSerialize.getChild(type));
//        assertEquals(id1, child1.getID());
//
//        // second child with the same type
//        IMemento child2 = mementoToSerialize.createChild(type, id2);
//        assertNotNull(child2);
//        assertEquals(2, mementoToSerialize.getChildren(type).length);
//        assertEquals(id2, child2.getID());
//      }
//
//      public: void checkAfterDeserialization(XMLMemento deserializedMemento) {
//        IMemento[] children = deserializedMemento.getChildren(type);
//        assertNotNull(children);
//        assertEquals(2, children.length);
//
//        // this checks that the order is maintained.
//        // the spec does not promise this, but clients
//        // may rely on the current implementation behaviour.
//        assertEquals(id1, children[0].getID());
//        assertEquals(id2, children[1].getID());
//      }
//    });
//  }

//  void testGetID() throws WorkbenchException, IOException {
//    final std::string type = "type";
//
//    std::string[] ids = { "id", "", "id.with.many.dots", "id_with_underscores",
//        "id<with<lessthan", "id>with>greaterthan", "id&with&ampersand",
//        "id\"with\"quote", "id#with#hash" };
//
//    for (int i = 0; i < ids.length; i++) {
//      final std::string id = ids[i];
//
//      testPutAndGet(new MementoChecker() {
//
//        public: void prepareAndCheckBeforeSerialization(
//            XMLMemento mementoToSerialize) {
//          assertEquals(null, mementoToSerialize.getChild(type));
//          IMemento child = mementoToSerialize.createChild(type, id);
//          assertEquals(id, child.getID());
//        }
//
//        public: void checkAfterDeserialization(
//            XMLMemento deserializedMemento) {
//          IMemento child = deserializedMemento.getChild(type);
//          assertNotNull(child);
//          assertEquals(id, child.getID());
//        }
//      });
//    }
//  }

//  void testPutAndGetFloat() throws WorkbenchException, IOException {
//    final std::string key = "key";
//
//    final Float[] values = new Float[] { new Float(-3.1415), new Float(1),
//        new Float(0), new Float(4554.45235),
//        new Float(Float.MAX_VALUE), new Float(Float.MIN_VALUE),
//        new Float(Float.NaN), new Float(Float.POSITIVE_INFINITY),
//        new Float(Float.NEGATIVE_INFINITY) };
//
//    for (int i = 0; i < values.length; i++) {
//      final Float value = values[i];
//      testPutAndGet(new MementoChecker() {
//
//        public: void prepareAndCheckBeforeSerialization(
//            XMLMemento mementoToSerialize) {
//          assertEquals(null, mementoToSerialize.getFloat(key));
//          mementoToSerialize.putFloat(key, value.floatValue());
//          assertEquals(value, mementoToSerialize.getFloat(key));
//        }
//
//        public: void checkAfterDeserialization(
//            XMLMemento deserializedMemento) {
//          assertEquals(value, deserializedMemento.getFloat(key));
//        }
//      });
//    }
//  }

//  void testPutAndGetInteger() throws WorkbenchException, IOException {
//    final std::string key = "key";
//
//    Integer[] values = new Integer[] { new Integer(36254), new Integer(0),
//        new Integer(1), new Integer(-36254),
//        new Integer(Integer.MAX_VALUE), new Integer(Integer.MIN_VALUE) };
//
//    for (int i = 0; i < values.length; i++) {
//      final Integer value = values[i];
//
//      testPutAndGet(new MementoChecker() {
//
//        public: void prepareAndCheckBeforeSerialization(
//            XMLMemento mementoToSerialize) {
//          assertEquals(null, mementoToSerialize.getInteger(key));
//          mementoToSerialize.putInteger(key, value.intValue());
//          assertEquals(value, mementoToSerialize.getInteger(key));
//        }
//
//        public: void checkAfterDeserialization(
//            XMLMemento deserializedMemento) {
//          assertEquals(value, deserializedMemento.getInteger(key));
//        }
//      });
//    }
//
//  }

  void TestPutMemento() throw(WorkbenchException, Poco::IOException);

  void TestPutAndGetString() throw(Poco::IOException, WorkbenchException);

//  void testPutAndGetTextData() throws WorkbenchException, IOException {
//    std::string[] values = TEST_STRINGS;
//
//    for (int i = 0; i < values.length; i++) {
//      final std::string data = values[i];
//      testPutAndGet(new MementoChecker() {
//
//        public: void prepareAndCheckBeforeSerialization(
//            XMLMemento mementoToSerialize) {
//          assertEquals(null, mementoToSerialize.getTextData());
//          mementoToSerialize.putTextData(data);
//          assertEquals(data, mementoToSerialize.getTextData());
//        }
//
//        public: void checkAfterDeserialization(
//            XMLMemento deserializedMemento) {
//          if (data.equals("")) {
//            // this comes back as null...
//            assertEquals(null, deserializedMemento.getTextData());
//          } else {
//            assertEquals(data, deserializedMemento.getTextData());
//          }
//        }
//      });
//    }
//  }

//  void testLegalKeys() throws WorkbenchException, IOException {
//    std::string[] legalKeys = { "value", "value.with.many.dots",
//        "value_with_underscores" };
//
//    for (int i = 0; i < legalKeys.length; i++) {
//      final std::string key = legalKeys[i];
//      testPutAndGet(new MementoChecker() {
//
//        public: void prepareAndCheckBeforeSerialization(
//            XMLMemento mementoToSerialize) {
//          assertEquals(null, mementoToSerialize.getString(key));
//          try {
//            mementoToSerialize.putString(key, "some string");
//          } catch (RuntimeException ex) {
//            System.out.println("offending key: '" + key + "'");
//            throw ex;
//          }
//          assertEquals("some string", mementoToSerialize
//              .getString(key));
//        }
//
//        public: void checkAfterDeserialization(
//            XMLMemento deserializedMemento) {
//          assertEquals("some string", deserializedMemento
//              .getString(key));
//        }
//      });
//    }
//
//  }

//  void testIllegalKeys() {
//    std::string[] illegalKeys = { "", " ", " key", "key ", "key key", "\t",
//        "\tkey", "key\t", "key\tkey", "\n", "\nkey", "key\n",
//        "key\nkey", "key<with<lessthan", "key>with>greaterthan",
//        "key&with&ampersand", "key#with#hash", "key\"with\"quote", "\"" };
//
//    for (int i = 0; i < illegalKeys.length; i++) {
//      final std::string key = illegalKeys[i];
//      XMLMemento memento = XMLMemento.createWriteRoot("foo");
//      try {
//        memento.putString(key, "some string");
//        fail("putString with illegal key should fail");
//      } catch (Exception ex) {
//        // expected
//      }
//    }
//  }

//  void testPutTextDataWithChildrenBug93718()
//      throws WorkbenchException, IOException {
//    final std::string textData = "\n\tThis is\ntext data\n\t\twith newlines and \ttabs\t\n\t ";
//    testPutAndGet(new MementoChecker() {
//
//      public: void prepareAndCheckBeforeSerialization(
//          XMLMemento mementoToSerialize) {
//        mementoToSerialize.createChild("type", "id");
//        mementoToSerialize.putTextData(textData);
//        mementoToSerialize.createChild("type", "id");
//        mementoToSerialize.createChild("type", "id");
//        assertEquals(textData, mementoToSerialize.getTextData());
//      }
//
//      public: void checkAfterDeserialization(XMLMemento deserializedMemento) {
//        assertEquals(textData, deserializedMemento.getTextData());
//      }
//    });
//  }

//     void testMementoWithTextContent113659() throws Exception {
//          IMemento memento = XMLMemento.createWriteRoot("root");
//          IMemento mementoWithChild = XMLMemento.createWriteRoot("root");
//          IMemento child = mementoWithChild.createChild("child");
//          child.putTextData("text");
//          memento.putMemento(mementoWithChild);
//          IMemento copiedChild = memento.getChild("child");
//          assertEquals("text", copiedChild.getTextData());
//      }

  struct MementoChecker : public CppUnit::TestCase {

    MementoChecker() : CppUnit::TestCase("MementoChecker") {}

    virtual void PrepareAndCheckBeforeSerialization(XMLMemento::Pointer mementoToSerialize) = 0;

    virtual void CheckAfterDeserialization(XMLMemento::Pointer deserializedMemento) = 0;
  };

  private:

  void TestPutAndGet(MementoChecker& mementoChecker)
      throw(Poco::IOException, WorkbenchException);


};

}

#endif /* CHERRYXMLMEMENTOTEST_H_ */
