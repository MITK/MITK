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
#include <mitkCommon.h>
#include <usModuleContext.h>
#include <usServiceReference.h>
#include <usGetModuleContext.h>
#include <mitkIPythonService.h>
#include <mitkTestingMacros.h>
#include <mitkTestFixture.h>
#include <mitkIPythonService.h>
#include <QmitkPythonSnippets.h>

#include <mitkCommonPythonTest.h>

class mitkPythonTestSuite : public mitk::CommonPythonTestSuite
{
  CPPUNIT_TEST_SUITE(mitkPythonTestSuite);
  MITK_TEST(TestPython);
  CPPUNIT_TEST_SUITE_END();

public:

  void TestPython()
  {
    MITK_INFO << "=================================== hallo";
    //us::Module* module = us::ModuleRegistry::GetModule("MitkPythonService");
    //us::ModuleContext* context = module->GetModuleContext();
    //////get the service which is generated in the PythonModuleActivator
    //us::ServiceReference<mitk::IPythonService> serviceRef = context->GetServiceReference<mitk::IPythonService>();
    //mitk::IPythonService* service = dynamic_cast<mitk::IPythonService*>( context->GetService(serviceRef) );

    std::string result = m_PythonService->Execute( "5+5", mitk::IPythonService::EVAL_COMMAND );
    MITK_TEST_CONDITION( result == "10", "Testing if running python code 5+5 results in 10" )
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkPython)

//int mitkPythonTest(int /*argc*/, char* /*argv*/[])
//{
//  MITK_TEST_BEGIN("PythonTest")
//
////  itk::SmartPointer<mitk::PythonService> _PythonService(new mitk::PythonService());
////
////  std::string result = _PythonService->Execute( "5+5", mitk::IPythonService::EVAL_COMMAND );
////  MITK_TEST_CONDITION( result == "10", "Testing if running python code 5+5 results in 10" )
//    MITK_INFO << "===================================";
//    MITK_INFO << "===================================";
//    MITK_INFO << "===================================";
//    MITK_INFO << "===================================";
//    MITK_INFO << "===================================";
//    MITK_INFO << "===================================";
//    MITK_INFO << "===================================";
//    MITK_INFO << "===================================";
//    MITK_INFO << "===================================";
//    MITK_INFO << "===================================";
//    MITK_INFO << "===================================";
//
//  MITK_TEST_END()
//}
