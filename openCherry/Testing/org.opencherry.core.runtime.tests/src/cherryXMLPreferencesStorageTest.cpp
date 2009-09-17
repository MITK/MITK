/*=========================================================================

 Program:   openCherry Platform
 Language:  C++
 Date:      $Date: 2009-02-20 18:47:40 +0100 (Fr, 20 Feb 2009) $
 Version:   $Revision: 16370 $

 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.

 =========================================================================*/

#include "cherryXMLPreferencesStorageTest.h"
#include <internal/cherryXMLPreferencesStorage.h>

#include <CppUnit/TestSuite.h>
#include <CppUnit/TestCaller.h>

using namespace std;

namespace cherry
{

  XMLPreferencesStorageTest::XMLPreferencesStorageTest(const std::string& testName)
  : CppUnit::TestCase(testName)
  {}

  CppUnit::Test* XMLPreferencesStorageTest::Suite()
  {
    CppUnit::TestSuite* suite = new CppUnit::TestSuite("XMLPreferencesStorageTest");

    CppUnit_addTest(suite, XMLPreferencesStorageTest, TestCreateFile);
    CppUnit_addTest(suite, XMLPreferencesStorageTest, TestLoadFile);
    return suite;
  }

  void XMLPreferencesStorageTest::TestCreateFile()
  {

    // m_File does not exist, test with empty preferences
    cherry::XMLPreferencesStorage storage(m_File);
    cherry::IPreferences::Pointer prefRoot = storage.GetRoot();
    assert(prefRoot.IsNotNull());

    // add some preferences ( tested in the preferences test -> no conditions here )
    cherry::IPreferences::Pointer textEditor = prefRoot->Node("/Text Editor");
    textEditor->PutBool("Show Line Numbers", true);
    textEditor->Put("File Extension", "txt rtf xml");

    cherry::IPreferences::Pointer cppEditor = prefRoot->Node("/Text Editor/C++");
    cppEditor->PutBool("Show Line Numbers", true);
    cppEditor->Put("File Extension", "cpp cxx");
    cppEditor->PutInt("Show margin at column", 80);

    cherry::IPreferences::Pointer javaEditor = prefRoot->Node("/Text Editor/Java");
    javaEditor->PutBool("Show Line Numbers", false);
    javaEditor->Put("File Extension", "java");
    javaEditor->PutInt("Show margin at column", 90);

    cherry::IPreferences::Pointer jsEditor = prefRoot->Node("/Text Editor/Java/Javascript");
    jsEditor->PutBool("Show Line Numbers", true);
    jsEditor->Put("File Extension", "js");
    jsEditor->PutInt("Show margin at column", 70);

    // try to flush it with an arbitrary node (should take the root node)
    storage.Flush(jsEditor.GetPointer());
    assert(m_File.exists());
  }

  void XMLPreferencesStorageTest::TestLoadFile()
  {
    // file exists, test parsing etc.
    cherry::XMLPreferencesStorage storage(m_File);
    cherry::IPreferences::Pointer prefRoot = storage.GetRoot();
    assert(prefRoot.IsNotNull());

    // try to find nodes that were create by testCreateFile
    assert(prefRoot->NodeExists("/Text Editor"));

    // try to get some properties
    cherry::IPreferences::Pointer cppEditor = prefRoot->Node("/Text Editor/C++");
    assert(cppEditor->GetBool("Show Line Numbers", false) == true);
    assert(cppEditor->Get("File Extension", "") == "txt rtf xml");
    assert(cppEditor->GetInt("Show margin at column", 0) == 80);

    cherry::IPreferences::Pointer jsEditor = prefRoot->Node("/Text Editor/Java/Javascript");
    assert(jsEditor->GetBool("Show Line Numbers", false) == true);
    assert(jsEditor->Get("File Extension", "") == "js");
    assert(jsEditor->GetInt("Show margin at column", 0) == 70);
  }
  
  void cherry::XMLPreferencesStorageTest::setUp()
  {
    Poco::Path path("prefs.xml");
    Poco::File file(path);

    if(file.exists())
      file.remove();

    m_File = file;
  }

  void cherry::XMLPreferencesStorageTest::tearDown()
  {

  }

}