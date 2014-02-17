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
#include <usGetModuleContext.h>
#include <usServiceReference.h>
#include <mitkTestingMacros.h>
#include <mitkPythonService.h>
#include <mitkIPythonService.h>
#include <mitkIOUtil.h>

#include <usModuleRegistry.h>
#include <usModule.h>
#include <usServiceReference.h>
#include <mitkImagePixelReadAccessor.h>

int mitkCopyToPythonAsItkImageTest(int /*argc*/, char* argv[])
{
  MITK_TEST_BEGIN("mitkCopyToPythonAsItkImageTest")

  //get the context of the python module
  us::Module* module = us::ModuleRegistry::GetModule("mitkPython");
  us::ModuleContext* context = module->GetModuleContext();
  //get the service which is generated in the PythonModuleActivator
  us::ServiceReference<mitk::IPythonService> serviceRef = context->GetServiceReference<mitk::IPythonService>();
  mitk::PythonService* pythonService = dynamic_cast<mitk::PythonService*>( context->GetService(serviceRef) );
  MITK_TEST_CONDITION(pythonService->IsItkPythonWrappingAvailable() == true, "Is Python available?");

  mitk::Image::Pointer testImage = mitk::IOUtil::LoadImage(std::string(argv[1]));

  //give it a name in python
  std::string nameOfImageInPython("mitkImage");

  MITK_TEST_CONDITION( pythonService->CopyToPythonAsItkImage( testImage, nameOfImageInPython) == true, "Valid image copied to python import should return true.");
  mitk::Image::Pointer pythonImage = pythonService->CopyItkImageFromPython(nameOfImageInPython);


  mitk::Index3D index;
  index[0] = 128;
  index[1] = 128;
  index[2] = 24;

  try{
    // pic3D of type char
    mitk::ImagePixelReadAccessor<char,3> pythonImageAccesor(pythonImage);

    //TODO Use the assert comparison methods once we have them implemented and remove GetPixelValueByIndex
    MITK_TEST_CONDITION( pythonImageAccesor.GetDimension(0) == 256, "Is the 1st dimension of Pic3D still 256?");
    MITK_TEST_CONDITION( pythonImageAccesor.GetDimension(1) == 256, "Is the 2nd dimension of Pic3D still 256?");
    MITK_TEST_CONDITION( pythonImageAccesor.GetDimension(2) == 49, "Is the 3rd dimension of Pic3D still 49?");

    MITK_TEST_CONDITION( pythonImageAccesor.GetPixelByIndex(index) == 96, "Is the value of Pic3D at (128,128,24) still 96?");
  }catch(...)
  {
    MITK_TEST_CONDITION( false, "Image is not readable! ");
  }

  MITK_TEST_END()
}
