/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <mitkDICOMFilesHelper.h>

#include <mitkFileSystem.h>
#include <mitkIOUtil.h>

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <itksys/SystemTools.hxx>

#include <fstream>

namespace
{
  void CreateFile(const std::string& path)
  {
    std::ofstream stream(path);
  }
}

/**
 * \brief Covers FindListedFile, which resolves the file name a caller opened
 *        against a directory listing built independently (e.g. by GDCM),
 *        so a name spelled in a different case still matches its listed entry.
 */
class mitkDICOMFilesHelperTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkDICOMFilesHelperTestSuite);

  MITK_TEST(ExactSpellingIsFound);
  MITK_TEST(DifferentlyCasedNameIsFound);
  MITK_TEST(UnlistedFileIsNotFound);
  MITK_TEST(SameNameInAnotherDirectoryIsNotFound);

  CPPUNIT_TEST_SUITE_END();

private:
  std::string m_TempDir;

public:
  void setUp() override
  {
    m_TempDir = mitk::IOUtil::CreateTemporaryDirectory("mitkDICOMFilesHelperTestXXXXXX");
  }

  void tearDown() override
  {
    if (!m_TempDir.empty())
    {
      itksys::SystemTools::RemoveADirectory(m_TempDir);
      m_TempDir.clear();
    }
  }

  void ExactSpellingIsFound()
  {
    const std::string a = m_TempDir + "/a.dcm";
    const std::string b = m_TempDir + "/b.dcm";
    CreateFile(a);
    CreateFile(b);

    const mitk::DICOMFilePathList listed = { a, b };
    const auto found = mitk::FindListedFile(b, listed);

    CPPUNIT_ASSERT_MESSAGE("An exactly spelled name is found", found.has_value());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The listed entry is returned", b, found.value());
  }

  /** Skips on a case-sensitive filesystem, where the differently cased path
      names a file that does not exist and there is nothing to resolve. */
  void DifferentlyCasedNameIsFound()
  {
    const std::string b = m_TempDir + "/b.dcm";
    CreateFile(b);

    const std::string differentlyCased = m_TempDir + "/B.dcm";
    if (!fs::exists(differentlyCased))
    {
      return;
    }

    const mitk::DICOMFilePathList listed = { b };
    const auto found = mitk::FindListedFile(differentlyCased, listed);

    CPPUNIT_ASSERT_MESSAGE("A differently cased name is found", found.has_value());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("The listed spelling is returned", b, found.value());
  }

  void UnlistedFileIsNotFound()
  {
    const std::string b = m_TempDir + "/b.dcm";
    const std::string c = m_TempDir + "/c.dcm";
    CreateFile(b);
    CreateFile(c);

    const mitk::DICOMFilePathList listed = { b };
    const auto found = mitk::FindListedFile(c, listed);

    CPPUNIT_ASSERT_MESSAGE("A file that is not in the listing is not found", !found.has_value());
  }

  /** The case-insensitive fallback passes the prefilter by name alone, so this
      pins that fs::equivalent still tells the two same-named files apart. */
  void SameNameInAnotherDirectoryIsNotFound()
  {
    const std::string b = m_TempDir + "/b.dcm";
    CreateFile(b);

    const std::string otherDir = m_TempDir + "/other";
    itksys::SystemTools::MakeDirectory(otherDir);
    const std::string otherB = otherDir + "/b.dcm";
    CreateFile(otherB);

    const mitk::DICOMFilePathList listed = { b };
    const auto found = mitk::FindListedFile(otherB, listed);

    CPPUNIT_ASSERT_MESSAGE("A same-named file in another directory is not found", !found.has_value());
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkDICOMFilesHelper)
