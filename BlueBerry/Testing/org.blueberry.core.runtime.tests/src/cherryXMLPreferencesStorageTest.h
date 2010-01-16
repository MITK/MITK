/*=========================================================================
 
 Program:   openCherry Platform
 Language:  C++
 Date:      $Date: 2009-09-07 12:05:58 +0200 (Mo, 07 Sep 2009) $
 Version:   $Revision: 18832 $
 
 Copyright (c) German Cancer Research Center, Division of Medical and
 Biological Informatics. All rights reserved.
 See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.
 
 This software is distributed WITHOUT ANY WARRANTY; without even
 the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 PURPOSE.  See the above copyright notices for more information.
 
 =========================================================================*/


#ifndef CHERRYXMLPREFERENCESSTORAGETEST_H_
#define CHERRYXMLPREFERENCESSTORAGETEST_H_

#include <cherryTestCase.h>
#include <Poco/File.h>

namespace cherry {

class XMLPreferencesStorageTest : public cherry::TestCase
{
public:

  XMLPreferencesStorageTest(const std::string& testName);
  virtual ~XMLPreferencesStorageTest();
  static CppUnit::Test* Suite();

  virtual void DoSetUp();
  virtual void DoTearDown();

  void TestLoadFile();
  void TestCreateFile();
protected:
  Poco::File m_File;
};

}

#endif /* CHERRYXMLPREFERENCESSTORAGETEST_H_ */
