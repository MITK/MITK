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
#include <list>
#include <string>

// MITK includes
#include "mitkIOUtil.h"
#include "mitkPointSet.h"
#include "mitkStandardFileLocations.h"

// VTK includes
#include <vtkDebugLeaks.h>

// stream includes
#include <fstream>
#include <iostream>

class mitkPointSetLocaleTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPointSetLocaleTestSuite);

  MITK_TEST(TestIfGermanLocaleUsed_Success);

  CPPUNIT_TEST_SUITE_END();

private:
  typedef std::list<std::string> StringList;
  StringList m_AllLocales;

  mitk::PointSet::Pointer m_RefPointSet;
  mitk::Point3D m_RefPoint;

  mitk::Point3D m_Point;

  mitk::PointSet::Pointer m_PointSet;

  bool ChangeLocale(const std::string &locale)
  {
    try
    {
      MITK_TEST_OUTPUT(<< "\n** Changing locale from " << setlocale(LC_ALL, nullptr) << " to '" << locale << "'");
      setlocale(LC_ALL, locale.c_str());

      std::locale l(locale.c_str());
      std::cin.imbue(l);
      std::cout.imbue(l);
      return true;
    }
    catch (...)
    {
      MITK_TEST_OUTPUT(<< "Could not activate locale " << locale << "\n");
      return false;
    }
  }

  void ReaderLocaleTest(mitk::Point3D &refPoint, std::string filename)
  {
    MITK_TEST_OUTPUT(<< "---- Reader Test ---- ");

    m_PointSet = mitk::IOUtil::Load<mitk::PointSet>(filename);

    if (m_PointSet->GetPointIfExists(0, &m_Point))
    {
      CPPUNIT_ASSERT_MESSAGE("read x correct", fabs(refPoint[0] - m_Point[0]) < 0.00001);
      CPPUNIT_ASSERT_MESSAGE("read y correct", fabs(refPoint[1] - m_Point[1]) < 0.00001);
      CPPUNIT_ASSERT_MESSAGE("read z correct", fabs(refPoint[2] - m_Point[2]) < 0.00001);
    }
    else
    {
      MITK_TEST_FAILED_MSG(<< "File " << filename << " can not be read - test will not applied.");
      return;
    }
  }

  void WriterLocaleTest(mitk::Point3D &refPoint, std::string filename)
  {
    MITK_TEST_OUTPUT(<< "---- Writer Test---- ");
    // create pointset
    m_RefPointSet = mitk::PointSet::New();
    m_RefPointSet->InsertPoint(0, refPoint);

    std::string tmpFilePath = mitk::IOUtil::CreateTemporaryFile("testPointSet_XXXXXX.mps");

    // write point set
    mitk::IOUtil::Save(m_RefPointSet, tmpFilePath);

    std::ifstream stream(tmpFilePath.c_str());

    // compare two .mps files
    std::ifstream refStream(filename.c_str());

    CPPUNIT_ASSERT_MESSAGE("Read reference point set", refStream);
    CPPUNIT_ASSERT_MESSAGE("Read point set", stream);

    bool differ = false;
    if (stream.is_open() && refStream.is_open())
    {
      std::string streamLine;
      std::string refStreamLine;
      while (!stream.eof() && !refStream.eof())
      {
        getline(stream, streamLine);
        getline(refStream, refStreamLine);
        if (streamLine.compare(refStreamLine) != 0)
        {
          differ = true;
          break;
        }
      }
      stream.close();
      refStream.close();
    }
    CPPUNIT_ASSERT_MESSAGE("Write point set correct", !differ);
  }

public:
  void setUp() override
  {
    m_RefPointSet = mitk::PointSet::New();

    // create locale list
    m_AllLocales.push_back("de_DE");
    m_AllLocales.push_back("de_DE.utf8");
    m_AllLocales.push_back("de_DE.UTF-8");
    m_AllLocales.push_back("de_DE@euro");
    m_AllLocales.push_back("German_Germany");

    m_RefPoint[0] = 32.2946;
    m_RefPoint[1] = -17.7359;
    m_RefPoint[2] = 29.6502;
  }

  void tearDown() override
  {
    m_RefPoint[0] = 0;
    m_RefPoint[1] = 0;
    m_RefPoint[2] = 0;

    m_AllLocales.clear();
  }

  void TestIfGermanLocaleUsed_Success()
  {
    // create reference point set
    m_RefPointSet->SetPoint(0, m_RefPoint);

    // QuickFix for MAC OS X
    // See for more the Bug #3894 comments
#if defined(__APPLE__) || defined(MACOSX)
    m_AllLocales.push_back("C");
#endif

    // write a reference file using the "C" locale once
    ChangeLocale("C");
    std::string referenceFilePath = mitk::IOUtil::CreateTemporaryFile("refPointSet_XXXXXX.mps");
    MITK_INFO << "Reference PointSet in " << referenceFilePath;

    // write point set
    mitk::IOUtil::Save(m_RefPointSet, referenceFilePath);

    unsigned int numberOfTestedGermanLocales(0);
    for (auto iter = m_AllLocales.begin(); iter != m_AllLocales.end(); ++iter)
    {
      if (ChangeLocale(*iter))
      {
        ++numberOfTestedGermanLocales;
        WriterLocaleTest(m_RefPoint, referenceFilePath);
        ReaderLocaleTest(m_RefPoint, referenceFilePath);
      }
    }

    if (numberOfTestedGermanLocales == 0)
    {
      MITK_TEST_OUTPUT(<< "Warning: No German locale was found on the system.");
    }
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkPointSetLocale)
