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

#include "mitkWhiteListReaderOptionsFunctor.h"
#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"

#include <limits>

class mitkWhiteListReaderOptionsFunctorTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkWhiteListReaderOptionsFunctorTestSuite);

  MITK_TEST(UseWhiteList);
  MITK_TEST(UseBlackList);
  MITK_TEST(UseNoList);

  CPPUNIT_TEST_SUITE_END();

private:
  std::string m_ImagePath;
  mitk::WhiteListReaderOptionsFunctor::ListType white;
  mitk::WhiteListReaderOptionsFunctor::ListType black;


public:
  void setUp() override
  {
    m_ImagePath = GetTestDataFilePath("TinyCTAbdomen_DICOMReader/100");
    
    white = { "MITK DICOM Reader v2 (classic config)" };
    black = { "MITK DICOM Reader" };
  }

  void tearDown() override
  {
  }

  void UseWhiteList()
  {
    mitk::IOUtil::LoadInfo info(m_ImagePath);

    mitk::WhiteListReaderOptionsFunctor functor = mitk::WhiteListReaderOptionsFunctor(white, mitk::WhiteListReaderOptionsFunctor::ListType());
    CPPUNIT_ASSERT(true == functor(info));
    CPPUNIT_ASSERT_EQUAL(std::string("MITK DICOM Reader v2 (classic config)"), info.m_ReaderSelector.GetSelected().GetDescription());
  }

  void UseNoList()
  {
    mitk::IOUtil::LoadInfo info(m_ImagePath);

    mitk::WhiteListReaderOptionsFunctor functor = mitk::WhiteListReaderOptionsFunctor(mitk::WhiteListReaderOptionsFunctor::ListType(), mitk::WhiteListReaderOptionsFunctor::ListType());
    CPPUNIT_ASSERT(true == functor(info));
    CPPUNIT_ASSERT_EQUAL(std::string("MITK DICOM Reader"), info.m_ReaderSelector.GetSelected().GetDescription());
  }

  void UseBlackList()
  {
    mitk::IOUtil::LoadInfo info(m_ImagePath);

    mitk::WhiteListReaderOptionsFunctor functor = mitk::WhiteListReaderOptionsFunctor(mitk::WhiteListReaderOptionsFunctor::ListType(), black);
    CPPUNIT_ASSERT(true == functor(info));
    CPPUNIT_ASSERT(info.m_ReaderSelector.GetSelected().GetDescription() != "MITK DICOM Reader");
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkWhiteListReaderOptionsFunctor)
