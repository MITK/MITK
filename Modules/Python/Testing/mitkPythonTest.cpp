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
#include <mitkGetModuleContext.h>
#include <mitkTestingMacros.h>
#include <mitkIPythonService.h>

int mitkPythonTest(int /*argc*/, char* /*argv*/[])
{
  MITK_TEST_BEGIN("DICOMTestingSanity")

  mitk::ModuleContext* context = mitk::GetModuleContext();
  mitk::ServiceReference serviceRef = context->GetServiceReference<mitk::IPythonService>();
  mitk::IPythonService* _PythonService = context->GetService<mitk::IPythonService>(serviceRef);

  std::string result = _PythonService->Execute( "5+5", mitk::IPythonService::EVAL_COMMAND );
  MITK_TEST_CONDITION( result == "10", "Testing if running python code 5+5 results in 10" )

  MITK_TEST_END()
}
