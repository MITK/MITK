/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2009-08-05 17:32:40 +0200 (Mi, 05 Aug 2009) $
Version:   $Revision: 7837 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "mitkTestingMacros.h"

#include <mitkDiffusionImagingObjectFactory.h>
#include <mitkFiberBundleX.h>
#include <mitkFiberBundleXReader.h>
#include <mitkFiberBundleXWriter.h>
#include <mitkBaseDataIOFactory.h>
#include <mitkBaseData.h>
#include <itksys/SystemTools.hxx>

/**Documentation
 *  Test for fiber bundle reader and writer
 */
int mitkFiberBundleXReaderWriterTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("mitkFiberBundleXReaderWriterTest");

  MITK_TEST_CONDITION_REQUIRED(argc>1,"Is filename missing?")

  mitk::FiberBundleXReader::Pointer reader = mitk::FiberBundleXReader::New();
  mitk::FiberBundleXWriter::Pointer writer = mitk::FiberBundleXWriter::New();
  mitk::FiberBundleX::Pointer fib1;
  mitk::FiberBundleX::Pointer fib2;

  // first test: did this work?
  // using MITK_TEST_CONDITION_REQUIRED makes the test stop after failure, since
  // it makes no sense to continue without an object.
  MITK_TEST_CONDITION_REQUIRED(reader.IsNotNull(),"Reader instantiation")
  MITK_TEST_CONDITION_REQUIRED(writer.IsNotNull(),"Writer instantiation")

  try{
    RegisterDiffusionImagingObjectFactory();

    // test if fib1 can be read
    const std::string s1="", s2="";
    std::vector<mitk::BaseData::Pointer> fibInfile = mitk::BaseDataIO::LoadBaseDataFromFile( argv[1], s1, s2, false );
    mitk::BaseData::Pointer baseData = fibInfile.at(0);
    fib1 = dynamic_cast<mitk::FiberBundleX*>(baseData.GetPointer());
    MITK_TEST_CONDITION_REQUIRED(fib1.IsNotNull(),"FiberBundleX reader returned NULL")

    // test if fib1 can be written
    MITK_TEST_CONDITION_REQUIRED( writer->CanWriteBaseDataType(fib1.GetPointer()),"FiberBundleX writer can write data")
    writer->SetFileName( "/usr/bin/writerTest.fib" );
    writer->DoWrite( fib1.GetPointer() );

    // test if fib1 can be read again as fib2
    fibInfile = mitk::BaseDataIO::LoadBaseDataFromFile( "/usr/bin/writerTest.fib", s1, s2, false );
    baseData = fibInfile.at(0);
    fib2 = dynamic_cast<mitk::FiberBundleX*>(baseData.GetPointer());
    MITK_TEST_CONDITION_REQUIRED(fib2.IsNotNull(),"FiberBundleX reader can read file written before")

    // test if fib1 equals fib2
    MITK_TEST_CONDITION_REQUIRED(fib1->Equals(fib2),"Fiber bundles are not changed during reading/writing")
  }
  catch(...) {
    //this means that a wrong exception (i.e. no itk:Exception) has been thrown
    std::cout << "Wrong exception (i.e. no itk:Exception) caught during write [FAILED]" << std::endl;
    return EXIT_FAILURE;
  }

  // always end with this!
  MITK_TEST_END();
}
