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

#include "mitkPreferenceListReaderOptionsFunctor.h"
#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"

#include <limits>

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


  CPPUNIT_TEST_SUITE_END();

private:
  std::string m_ImagePath;
  mitk::PreferenceListReaderOptionsFunctor::ListType preference;
  mitk::PreferenceListReaderOptionsFunctor::ListType black;
  mitk::PreferenceListReaderOptionsFunctor::ListType emptyList;

public:
  void setUp() override
  {
    m_ImagePath = GetTestDataFilePath("TinyCTAbdomen_DICOMReader/100");
    
    preference = { "MITK DICOM Reader v2 (classic config)" };
    black = { "MITK DICOM Reader" };
    emptyList = {};
  }

  void tearDown() override
  {
  }

  void UsePreferenceList()
  {
    mitk::IOUtil::LoadInfo info(m_ImagePath);

    mitk::PreferenceListReaderOptionsFunctor functor = mitk::PreferenceListReaderOptionsFunctor(preference, emptyList);
    CPPUNIT_ASSERT(true == functor(info));
    CPPUNIT_ASSERT_EQUAL(std::string("MITK DICOM Reader v2 (classic config)"), info.m_ReaderSelector.GetSelected().GetDescription());
  }

  void UseNoList()
  {
    mitk::IOUtil::LoadInfo info(m_ImagePath);

    mitk::PreferenceListReaderOptionsFunctor functor = mitk::PreferenceListReaderOptionsFunctor(emptyList, emptyList);
    CPPUNIT_ASSERT(true == functor(info));
    CPPUNIT_ASSERT_EQUAL(std::string("MITK DICOM Reader"), info.m_ReaderSelector.GetSelected().GetDescription());
  }

  void UseBlackList()
  {
    mitk::IOUtil::LoadInfo info(m_ImagePath);

    mitk::PreferenceListReaderOptionsFunctor functor = mitk::PreferenceListReaderOptionsFunctor(emptyList, black);
    CPPUNIT_ASSERT(true == functor(info));
    CPPUNIT_ASSERT(info.m_ReaderSelector.GetSelected().GetDescription() != "MITK DICOM Reader");
  }

  void UseBlackAndPreferenceList()
  {
    mitk::IOUtil::LoadInfo info(m_ImagePath);

    mitk::PreferenceListReaderOptionsFunctor functor = mitk::PreferenceListReaderOptionsFunctor(preference, black);
    CPPUNIT_ASSERT(true == functor(info));
    CPPUNIT_ASSERT_EQUAL(std::string("MITK DICOM Reader v2 (classic config)"), info.m_ReaderSelector.GetSelected().GetDescription());
  }

  void UseOverlappingBlackAndPreferenceList()
  {
    mitk::IOUtil::LoadInfo info(m_ImagePath);

    black.push_back("MITK DICOM Reader v2 (classic config)");

    mitk::PreferenceListReaderOptionsFunctor functor = mitk::PreferenceListReaderOptionsFunctor(preference, black);
    CPPUNIT_ASSERT(true == functor(info));
    CPPUNIT_ASSERT_EQUAL(std::string("MITK DICOM Reader v2 (autoselect)"), info.m_ReaderSelector.GetSelected().GetDescription());
  }

  void UsePreferenceListWithInexistantReaders()
  {
    mitk::IOUtil::LoadInfo info(m_ImagePath);
    preference.push_back("InexistantReader");

    mitk::PreferenceListReaderOptionsFunctor functor = mitk::PreferenceListReaderOptionsFunctor(preference, emptyList);
    CPPUNIT_ASSERT(true == functor(info));
    CPPUNIT_ASSERT_EQUAL(std::string("MITK DICOM Reader v2 (classic config)"), info.m_ReaderSelector.GetSelected().GetDescription());
  }

  void UseAllBlackedList()
  {
    mitk::IOUtil::LoadInfo info(m_ImagePath);

    for (auto reader : info.m_ReaderSelector.Get())
    {
      black.push_back(reader.GetDescription());
    }

    mitk::PreferenceListReaderOptionsFunctor functor = mitk::PreferenceListReaderOptionsFunctor(emptyList, black);
    CPPUNIT_ASSERT_THROW(functor(info), mitk::Exception);
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkPreferenceListReaderOptionsFunctor)
