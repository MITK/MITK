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

/*
 This test is meant to reproduce the following error:

 - The machine or current user has a German locale.
 - This esp. means that stream IO expects the decimal separator as a comma: ","
 - DICOM files use a point "." as the decimal separator to be locale independent
 - The parser used by MITK (ITK's GDCM) seems to use the current locale instead of the "C" or "POSIX" locale
 - This leads to spacings (and probably other numbers) being trimmed/rounded,
   e.g. the correct spacing of 0.314 is read as 1.0 etc.

*/

#include "mitkStandardFileLocations.h"
#include "mitkDicomSeriesReader.h"
#include "mitkIOUtil.h"
#include "mitkImage.h"

#include "mitkTestingMacros.h"
#include "mitkTestFixture.h"

#include <list>
#include <locale>
#include <locale.h>

class mitkDICOMLocaleTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkDICOMLocaleTestSuite);
  CPPUNIT_TEST_SUITE_ADD_CUSTOM_TESTS(addDICOMLocaleWithReferenceImageTests);
  CPPUNIT_TEST_SUITE_END();

private:

  // A custom method for adding a combination of filename and locale tests
  static void addDICOMLocaleWithReferenceImageTests(TestSuiteBuilderContextType& context)
  {
    std::vector<std::string> fileArgs;
    fileArgs.push_back("spacing-ok-ct.dcm");
    fileArgs.push_back("spacing-ok-mr.dcm");
    fileArgs.push_back("spacing-ok-sc.dcm");

    // load a reference DICOM file with German locales being set
    std::vector<std::string> localeArgs;
    localeArgs.push_back("C");
    localeArgs.push_back("de_DE");
    localeArgs.push_back("de_DE.utf8");
    localeArgs.push_back("de_DE.UTF8");
    localeArgs.push_back("de_DE@euro");
    localeArgs.push_back("German_Germany");

    for (std::size_t fileIndex = 0; fileIndex < fileArgs.size(); ++fileIndex)
    {
      for (std::size_t localeIndex = 0; localeIndex < localeArgs.size(); ++localeIndex)
      {
        MITK_PARAMETERIZED_TEST_2(testLocaleWithReferenceImage, fileArgs[fileIndex], localeArgs[localeIndex]);
      }
    }
  }

private:

  std::string m_FileName;
  std::string m_Locale;
  bool m_SkipImageTest;

  char* m_OldLocale;

  void SetTestParameter()
  {
    std::vector<std::string> parameter = GetTestParameter();
    CPPUNIT_ASSERT(parameter.size() == 2);
    m_FileName = GetTestDataFilePath(parameter[0]);
    m_Locale = parameter[1];
  }

public:

  mitkDICOMLocaleTestSuite() : m_OldLocale(NULL) {}



  // Change the current locale to m_Locale
  void setUp()
  {
    m_SkipImageTest = false;
    m_OldLocale = NULL;
    SetTestParameter();

    try
    {
      m_OldLocale = setlocale(LC_ALL, NULL);
      MITK_TEST_OUTPUT(<< " ** Changing locale from " << m_OldLocale << " to '" << m_Locale << "'")
      setlocale(LC_ALL, m_Locale.c_str());
      std::cin.imbue(std::locale(m_Locale.c_str()));
    }
    catch(...)
    {
      MITK_TEST_OUTPUT(<< "Could not activate locale " << m_Locale)
      m_SkipImageTest = true;
    }
  }

  void tearDown()
  {
    if (m_OldLocale)
    {
      setlocale(LC_ALL, m_OldLocale);
      std::cin.imbue(std::locale(m_OldLocale));
    }
  }

  void testLocaleWithReferenceImage()
  {
    if (m_SkipImageTest) return;

    mitk::Image::Pointer image = mitk::IOUtil::LoadImage(m_FileName);
    CPPUNIT_ASSERT(image.IsNotNull());

    // note importance of minor differences in spacings:
    // DICOM has order y-spacing, x-spacing, while in MITK we assume x-spacing, y-spacing (both meant for 0 and 1 index in array)
    CPPUNIT_ASSERT_MESSAGE("incorrect x spacing", mitk::Equal(image->GetGeometry()->GetSpacing()[0], 0.3141592));
    CPPUNIT_ASSERT_MESSAGE("incorrect y spacing ", mitk::Equal(image->GetGeometry()->GetSpacing()[1], 0.3411592));
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkDICOMLocale)
