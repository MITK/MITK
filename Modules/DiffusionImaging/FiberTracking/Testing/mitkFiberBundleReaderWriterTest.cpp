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

#include "mitkTestingMacros.h"
#include <mitkFiberBundle.h>
#include <mitkBaseData.h>
#include <itksys/SystemTools.hxx>
#include <mitkTestingConfig.h>
#include <mitkIOUtil.h>

#include "mitkTestFixture.h"

class mitkFiberBundleReaderWriterTestSuite : public mitk::TestFixture
{

  CPPUNIT_TEST_SUITE(mitkFiberBundleReaderWriterTestSuite);
  MITK_TEST(Equal_SaveLoad_ReturnsTrue);
  CPPUNIT_TEST_SUITE_END();

private:

  /** Members used inside the different (sub-)tests. All members are initialized via setUp().*/
  mitk::FiberBundle::Pointer fib1;
  mitk::FiberBundle::Pointer fib2;

public:

  void setUp() override
  {
    fib1 = nullptr;
    fib2 = nullptr;

    std::string filename = GetTestDataFilePath("DiffusionImaging/fiberBundleX.fib");

    std::vector<mitk::BaseData::Pointer> fibInfile = mitk::IOUtil::Load( filename);
    mitk::BaseData::Pointer baseData = fibInfile.at(0);

    fib1 = dynamic_cast<mitk::FiberBundle*>(baseData.GetPointer());
  }

  void tearDown() override
  {
    fib1 = nullptr;
    fib2 = nullptr;
  }

  void Equal_SaveLoad_ReturnsTrue()
  {
    mitk::IOUtil::Save(fib1.GetPointer(), std::string(MITK_TEST_OUTPUT_DIR)+"/writerTest.fib");
    std::vector<mitk::BaseData::Pointer> baseData = mitk::IOUtil::Load(std::string(MITK_TEST_OUTPUT_DIR)+"/writerTest.fib");
    fib2 = dynamic_cast<mitk::FiberBundle*>(baseData[0].GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Should be equal", fib1->Equals(fib2));
    //MITK_ASSERT_EQUAL(fib1, fib2, "A saved and re-loaded file should be equal");
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkFiberBundleReaderWriter)
