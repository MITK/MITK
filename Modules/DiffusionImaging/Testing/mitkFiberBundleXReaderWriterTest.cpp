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

#include <mitkDiffusionImagingObjectFactory.h>
#include <mitkFiberBundleX.h>
#include <mitkFiberBundleXReader.h>
#include <mitkFiberBundleXWriter.h>
#include <mitkBaseDataIOFactory.h>
#include <mitkBaseData.h>
#include <itksys/SystemTools.hxx>
#include <mitkTestingConfig.h>

/**Documentation
 *  Test for fiber bundle reader and writer
 */
int mitkFiberBundleXReaderWriterTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("mitkFiberBundleXReaderWriterTest");

   std::cout << argv[1]<<std::endl;

  MITK_TEST_CONDITION_REQUIRED(argc>1,"check for filename")

  mitk::FiberBundleXWriter::Pointer writer = mitk::FiberBundleXWriter::New();
  mitk::FiberBundleX::Pointer fib1;
  mitk::FiberBundleX::Pointer fib2;

  // first test: did this work?
  // using MITK_TEST_CONDITION_REQUIRED makes the test stop after failure, since
  // it makes no sense to continue without an object.
  MITK_TEST_CONDITION_REQUIRED(writer.IsNotNull(),"writer instantiation")

  try{
    RegisterDiffusionImagingObjectFactory();

    // test if fib1 can be read
    const std::string s1="", s2="";



    std::vector<mitk::BaseData::Pointer> fibInfile = mitk::BaseDataIO::LoadBaseDataFromFile( argv[1], s1, s2, false );
    mitk::BaseData::Pointer baseData = fibInfile.at(0);
    fib1 = dynamic_cast<mitk::FiberBundleX*>(baseData.GetPointer());
    MITK_TEST_CONDITION_REQUIRED(fib1.IsNotNull(),"check if reader returned null")

    // test if fib1 can be written
    MITK_TEST_CONDITION_REQUIRED( writer->CanWriteBaseDataType(fib1.GetPointer()),"writer can write data")
    writer->SetFileName( std::string(MITK_TEST_OUTPUT_DIR)+"/writerTest.fib" );
    writer->DoWrite( fib1.GetPointer() );

    // test if fib1 can be read again as fib2
    fibInfile = mitk::BaseDataIO::LoadBaseDataFromFile( std::string(MITK_TEST_OUTPUT_DIR)+"/writerTest.fib", s1, s2, false );
    baseData = fibInfile.at(0);
    fib2 = dynamic_cast<mitk::FiberBundleX*>(baseData.GetPointer());
    MITK_TEST_CONDITION_REQUIRED(fib2.IsNotNull(),"reader can read file written before")

    // test if fib1 equals fib2
    MITK_TEST_CONDITION_REQUIRED(fib1->Equals(fib2),"fiber bundles are not changed during reading/writing")
  }
  catch(...) {
    return EXIT_FAILURE;
  }

  // always end with this!
  MITK_TEST_END();
}
