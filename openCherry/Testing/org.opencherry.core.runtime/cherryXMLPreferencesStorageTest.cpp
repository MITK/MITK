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

#include "cherryTestingMacros.h"
#include "cherryXMLPreferencesStorage.h"

using namespace std;

void testLoadFile(Poco::File& file)
{
  // file exists, test parsing etc.
  cherry::XMLPreferencesStorage storage(file);
  cherry::IPreferences::Pointer prefRoot = storage.GetRoot();
  CHERRY_TEST_CONDITION(prefRoot.IsNotNull(), "prefRoot.IsNotNull()")

  // try to find nodes that were create by testCreateFile
  CHERRY_TEST_CONDITION(prefRoot->NodeExists("/Text Editor"), "prefRoot.NodeExists(\"/Text Editor\")")
  CHERRY_TEST_CONDITION(prefRoot->NodeExists("/Text Editor"), "prefRoot.NodeExists(\"/Text Editor/C++\")")
  CHERRY_TEST_CONDITION(prefRoot->NodeExists("/Text Editor"), "prefRoot.NodeExists(\"/Text Editor/Java\")")
  CHERRY_TEST_CONDITION(prefRoot->NodeExists("/Text Editor"), "prefRoot.NodeExists(\"/Text Editor/Java/Javascript\")")

  // try to get some properties
  cherry::IPreferences::Pointer cppEditor = prefRoot->Node("/Text Editor/C++");
  CHERRY_TEST_CONDITION(cppEditor->GetBool("Show Line Numbers", false) == true, "cppEditor->GetBool(\"Show Line Numbers\")")
  CHERRY_TEST_CONDITION(cppEditor->Get("File Extension", "") == "cpp cxx", "cppEditor->Get(\"File Extension\") == \"cpp cxx\"")

  cherry::IPreferences::Pointer jsEditor = prefRoot->Node("/Text Editor/Java/Javascript");
  CHERRY_TEST_CONDITION(jsEditor->GetBool("Show Line Numbers", false) == true, "jsEditor->GetBool(\"Show Line Numbers\")")
  CHERRY_TEST_CONDITION(jsEditor->Get("File Extension", "") == "js", "jsEditor->Get(\"File Extension\") == \"js\"")
  CHERRY_TEST_CONDITION(jsEditor->GetInt("Show margin at column", 0) == 70, "jsEditor->GetInt(\"Show margin at column\", 0) == 70")
}

void testCreateFile(Poco::File& file)
{
  // file does not exist, test with empty preferences
  cherry::XMLPreferencesStorage storage(file);
  cherry::IPreferences::Pointer prefRoot = storage.GetRoot();
  CHERRY_TEST_CONDITION(prefRoot.IsNotNull(), "prefRoot.IsNotNull()")

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
  CHERRY_TEST_CONDITION(file.exists(), "file.exists()")
}

int cherryXMLPreferencesStorageTest(int /*argc*/, char* /*argv*/[])
{
  CHERRY_TEST_BEGIN("XMLPreferencesStorage")

  Poco::Path path("prefs.xml");
  Poco::File file(path);
  
  if(file.exists())
    file.remove();

  testCreateFile(file);
  testLoadFile(file);

  CHERRY_TEST_END()
}
