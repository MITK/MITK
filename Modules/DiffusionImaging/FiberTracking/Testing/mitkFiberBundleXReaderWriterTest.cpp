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
#include <mitkFiberBundleX.h>
#include <mitkBaseData.h>
#include <itksys/SystemTools.hxx>
#include <mitkTestingConfig.h>
#include <mitkIOUtil.h>

#include "mitkTestFixture.h"

class mitkFiberBundleXReaderWriterTestSuite : public mitk::TestFixture
{

  CPPUNIT_TEST_SUITE(mitkFiberBundleXReaderWriterTestSuite);
  MITK_TEST(Equal_SaveLoad_ReturnsTrue);
  CPPUNIT_TEST_SUITE_END();

private:

  /** Members used inside the different (sub-)tests. All members are initialized via setUp().*/
  mitk::FiberBundleX::Pointer fib1;
  mitk::FiberBundleX::Pointer fib2;

public:

  void setUp()
  {
    fib1 = NULL;
    fib2 = NULL;

    std::vector<mitk::BaseData::Pointer> baseData = mitk::IOUtil::Load(GetTestDataFilePath("DiffusionImaging/fiberBundleX.fib"));
    fib1 = dynamic_cast<mitk::FiberBundleX*>(baseData.at(0).GetPointer());
  }

  void tearDown()
  {
    fib1 = NULL;
    fib2 = NULL;
  }

  void Equal_SaveLoad_ReturnsTrue()
  {
    mitk::IOUtil::Save(fib1.GetPointer(), std::string(MITK_TEST_OUTPUT_DIR)+"/writerTest.fib");
    std::vector<mitk::BaseData::Pointer> fibInfile = mitk::IOUtil::Load(std::string(MITK_TEST_OUTPUT_DIR)+"/writerTest.fib");
    mitk::BaseData::Pointer baseData = fibInfile.at(0);
    fib2 = dynamic_cast<mitk::FiberBundleX*>(baseData.GetPointer());
    CPPUNIT_ASSERT_MESSAGE("Should be equal", fib1->Equals(fib2));
    //MITK_ASSERT_EQUAL(fib1, fib2, "A saved and re-loaded file should be equal");
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkFiberBundleXReaderWriter)
