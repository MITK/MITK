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
#include <mitkBaseDataIOFactory.h>
#include <mitkBaseData.h>
#include <itksys/SystemTools.hxx>
#include <mitkTestingConfig.h>
#include <math.h>

/**Documentation
 *  Test for fiber bundle reader and writer
 */
int mitkFiberBundleXTest(int argc, char* argv[])
{
  MITK_TEST_BEGIN("mitkFiberBundleXTest");

  MITK_TEST_CONDITION_REQUIRED(argc>1,"check for filename")

  mitk::FiberBundleXReader::Pointer reader = mitk::FiberBundleXReader::New();
  mitk::FiberBundleX::Pointer fib1, fib2;

  // first test: did this work?
  // using MITK_TEST_CONDITION_REQUIRED makes the test stop after failure, since
  // it makes no sense to continue without an object.
  MITK_TEST_CONDITION_REQUIRED(reader.IsNotNull(),"reader instantiation")

  try{
    RegisterDiffusionImagingObjectFactory();

    // test if fib1 can be read
    const std::string s1="", s2="";
    std::vector<mitk::BaseData::Pointer> fibInfile = mitk::BaseDataIO::LoadBaseDataFromFile( argv[1], s1, s2, false );
    mitk::BaseData::Pointer baseData = fibInfile.at(0);
    fib1 = dynamic_cast<mitk::FiberBundleX*>(baseData.GetPointer());
    MITK_TEST_CONDITION_REQUIRED(fib1.IsNotNull(),"check if reader 1 returned null")

    fibInfile = mitk::BaseDataIO::LoadBaseDataFromFile( argv[1], s1, s2, false );
    baseData = fibInfile.at(0);
    fib2 = dynamic_cast<mitk::FiberBundleX*>(baseData.GetPointer());
    MITK_TEST_CONDITION_REQUIRED(fib2.IsNotNull(),"check if reader 2 returned null")

    MITK_TEST_CONDITION_REQUIRED(fib1->Equals(fib2),"check if equals method is working");

    int randNum = rand()%20;
    MITK_INFO << "DoFiberSmoothing(" << randNum << ")" << randNum; fib2->DoFiberSmoothing(randNum);
    MITK_TEST_CONDITION_REQUIRED(!fib1->Equals(fib2),"check if fiber resampling method does something");

    mitk::FiberBundleX::Pointer fib3 = fib1->AddBundle(fib2);
    MITK_TEST_CONDITION_REQUIRED(!fib1->Equals(fib3),"check if A+B!=A");
//    fib3 = fib3->SubtractBundle(fib2);
//    MITK_TEST_CONDITION_REQUIRED(fib1->Equals(fib3),"check if A+B-B==A");

    fib1->AddBundle(NULL);
    MITK_INFO << "GenerateFiberIds"; fib1->GenerateFiberIds();
    MITK_INFO << "GetFiberPolyData"; fib1->GetFiberPolyData();
    MITK_INFO << "GetAvailableColorCodings"; fib1->GetAvailableColorCodings();
    MITK_INFO << "GetCurrentColorCoding"; fib1->GetCurrentColorCoding();
    MITK_INFO << "SetFiberPolyData"; fib1->SetFiberPolyData(NULL);
    MITK_INFO << "ExtractFiberSubset"; fib1->ExtractFiberSubset(NULL);
    MITK_INFO << "ExtractFiberIdSubset"; fib1->ExtractFiberIdSubset(NULL);
    std::vector< long > tmp;
    MITK_INFO << "GeneratePolyDataByIds"; fib1->GeneratePolyDataByIds(tmp);
    MITK_INFO << "SetColorCoding"; fib1->SetColorCoding(NULL);
    MITK_INFO << "SetFAMap"; fib1->SetFAMap(NULL);
    MITK_INFO << "DoColorCodingOrientationBased"; fib1->DoColorCodingOrientationBased();
    MITK_INFO << "DoColorCodingFaBased"; fib1->DoColorCodingFaBased();
    MITK_INFO << "DoUseFaFiberOpacity"; fib1->DoUseFaFiberOpacity();
    MITK_INFO << "ResetFiberOpacity"; fib1->ResetFiberOpacity();

    float randFloat = rand()%300;
    MITK_INFO << "RemoveShortFibers(" << randFloat << ")"; fib1->RemoveShortFibers(randFloat);
  }
  catch(...)
  {
    //this means that a wrong exception (i.e. no itk:Exception) has been thrown
    std::cout << "Wrong exception (i.e. no itk:Exception) caught during write [FAILED]" << std::endl;
    return EXIT_FAILURE;
  }

  // always end with this!
  MITK_TEST_END();
}
