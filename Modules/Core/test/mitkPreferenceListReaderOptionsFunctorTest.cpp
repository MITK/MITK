/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "mitkPreferenceListReaderOptionsFunctor.h"
#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"
#include <mitkAbstractFileReader.h>
#include <mitkCustomMimeType.h>
#include <mitkIOMimeTypes.h>
#include <usModuleContext.h>

#include <limits>

namespace mitk
{
  class TestFileReaderService : public mitk::AbstractFileReader
  {
  public:
    TestFileReaderService(const std::string &description)
    : AbstractFileReader(CustomMimeType("TestMimeType"), description)
    {
      Options options;
      options["o1"] = 0;
      this->SetDefaultOptions(options);
      m_ServiceRegistration = RegisterService();
    };

    ~TestFileReaderService() override
    {
    };

    using AbstractFileReader::Read;

    std::vector<itk::SmartPointer<BaseData>> DoRead() override
    {
      std::vector<itk::SmartPointer<BaseData>> result;
      return result;
    };

    ConfidenceLevel GetConfidenceLevel() const override
    {
      return Supported;
    };

  private:
    TestFileReaderService * Clone() const override
    {
      return new TestFileReaderService(*this);
    };

    us::ServiceRegistration<IFileWriter> m_ServiceRegistration;
  };

} // namespace mitk


class mitkPreferenceListReaderOptionsFunctorTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPreferenceListReaderOptionsFunctorTestSuite);

  MITK_TEST(UsePreferenceList);
  MITK_TEST(UseBlackList);
  MITK_TEST(UseNoList);
  MITK_TEST(UseBlackAndPreferenceList);
  MITK_TEST(UseOverlappingBlackAndPreferenceList);
  MITK_TEST(UsePreferenceListWithInexistantReaders);
  MITK_TEST(UseAllBlackedList);
  MITK_TEST(SetOptions);

  CPPUNIT_TEST_SUITE_END();

private:
  std::string m_ImagePath;
  mitk::PreferenceListReaderOptionsFunctor::ListType preference;
  mitk::PreferenceListReaderOptionsFunctor::ListType black;
  mitk::PreferenceListReaderOptionsFunctor::ListType emptyList;

  mitk::TestFileReaderService* m_NormalService;
  mitk::TestFileReaderService* m_PrefService;
  mitk::TestFileReaderService* m_BlackService;
  mitk::CustomMimeType* m_TestMimeType;

public:
  void setUp() override
  {
    m_ImagePath = GetTestDataFilePath("BallBinary30x30x30.nrrd");

    preference = { "Preferred Test Service" };
    black = { "Unwanted Test Service" };
    emptyList = {};

    m_TestMimeType = new mitk::CustomMimeType("TestMimeType");
    m_TestMimeType->AddExtension("nrrd");
    m_TestMimeType->SetCategory(mitk::IOMimeTypes::CATEGORY_IMAGES());
    m_TestMimeType->SetComment("Test mime type");

    us::ModuleContext *context = us::GetModuleContext();
    us::ServiceProperties props;
    props[us::ServiceConstants::SERVICE_RANKING()] = 100;
    context->RegisterService(m_TestMimeType, props);

    m_NormalService = new mitk::TestFileReaderService("Normal Test Service");
    m_PrefService = new mitk::TestFileReaderService("Preferred Test Service");
    m_BlackService = new mitk::TestFileReaderService("Unwanted Test Service");
  }

  void tearDown() override
  {
    delete m_PrefService;
    delete m_BlackService;
    delete m_NormalService;
    delete m_TestMimeType;
  }

  void UsePreferenceList()
  {
    mitk::IOUtil::LoadInfo info(m_ImagePath);

    mitk::PreferenceListReaderOptionsFunctor functor = mitk::PreferenceListReaderOptionsFunctor(preference, emptyList);
    CPPUNIT_ASSERT(true == functor(info));
    auto description = info.m_ReaderSelector.GetSelected().GetDescription();
    CPPUNIT_ASSERT_EQUAL(std::string("Preferred Test Service"), description);
    CPPUNIT_ASSERT("0" == info.m_ReaderSelector.GetSelected().GetReader()->GetOptions()["o1"].ToString());
  }

  void UseNoList()
  {
    mitk::IOUtil::LoadInfo info(m_ImagePath);

    mitk::PreferenceListReaderOptionsFunctor functor = mitk::PreferenceListReaderOptionsFunctor(emptyList, emptyList);
    CPPUNIT_ASSERT(true == functor(info));
    auto description = info.m_ReaderSelector.GetSelected().GetDescription();
    CPPUNIT_ASSERT_EQUAL(std::string("Normal Test Service"), description);
  }

  void UseBlackList()
  {
    mitk::IOUtil::LoadInfo info(m_ImagePath);

    mitk::PreferenceListReaderOptionsFunctor functor = mitk::PreferenceListReaderOptionsFunctor(emptyList, black);
    CPPUNIT_ASSERT(true == functor(info));
    auto description = info.m_ReaderSelector.GetSelected().GetDescription();
    CPPUNIT_ASSERT(description != "Unwanted Test Service");
  }

  void UseBlackAndPreferenceList()
  {
    mitk::IOUtil::LoadInfo info(m_ImagePath);

    mitk::PreferenceListReaderOptionsFunctor functor = mitk::PreferenceListReaderOptionsFunctor(preference, black);
    CPPUNIT_ASSERT(true == functor(info));
    auto description = info.m_ReaderSelector.GetSelected().GetDescription();
    CPPUNIT_ASSERT_EQUAL(std::string("Preferred Test Service"), description);
  }

  void UseOverlappingBlackAndPreferenceList()
  {
    mitk::IOUtil::LoadInfo info(m_ImagePath);

    black.push_back("Preferred Test Service");
    black.push_back("Normal Test Service");

    mitk::PreferenceListReaderOptionsFunctor functor = mitk::PreferenceListReaderOptionsFunctor(preference, black);
    CPPUNIT_ASSERT(true == functor(info));
    auto description = info.m_ReaderSelector.GetSelected().GetDescription();
    CPPUNIT_ASSERT_EQUAL(std::string("ITK NrrdImageIO"), description);
  }

  void UsePreferenceListWithInexistantReaders()
  {
    mitk::IOUtil::LoadInfo info(m_ImagePath);
    preference.push_back("InexistantReader");

    mitk::PreferenceListReaderOptionsFunctor functor = mitk::PreferenceListReaderOptionsFunctor(preference, emptyList);
    CPPUNIT_ASSERT(true == functor(info));
    auto description = info.m_ReaderSelector.GetSelected().GetDescription();
    CPPUNIT_ASSERT_EQUAL(std::string("Preferred Test Service"), description);
  }

  void UseAllBlackedList()
  {
    mitk::IOUtil::LoadInfo info(m_ImagePath);

    for (const auto &reader : info.m_ReaderSelector.Get())
    {
      black.push_back(reader.GetDescription());
    }

    mitk::PreferenceListReaderOptionsFunctor functor = mitk::PreferenceListReaderOptionsFunctor(emptyList, black);
    CPPUNIT_ASSERT_THROW(functor(info), mitk::Exception);
  }

  void SetOptions()
  {
    mitk::IOUtil::LoadInfo info(m_ImagePath);

    mitk::IFileReader::Options options;
    options.insert(std::make_pair("o1", 42));
  
    mitk::PreferenceListReaderOptionsFunctor functor = mitk::PreferenceListReaderOptionsFunctor(preference, options);

    CPPUNIT_ASSERT(true == functor(info));
    auto description = info.m_ReaderSelector.GetSelected().GetDescription();
    CPPUNIT_ASSERT_EQUAL(std::string("Preferred Test Service"), description);
    CPPUNIT_ASSERT("42" == info.m_ReaderSelector.GetSelected().GetReader()->GetOption("o1").ToString());
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkPreferenceListReaderOptionsFunctor)
