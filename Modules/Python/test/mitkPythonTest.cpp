/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include <usModuleContext.h>
#include <usServiceReference.h>
#include <usGetModuleContext.h>
#include <mitkIPythonService.h>
#include <mitkCommon.h>
#include <mitkTestingMacros.h>
#include <mitkTestFixture.h>
#include <mitkStandardFileLocations.h>
#include <mitkIOUtil.h>




class mitkPythonTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPythonTestSuite);
  MITK_TEST(TestEvaluateOperationWithResult);
  MITK_TEST(TestCopyImageToAndBackPython);
  CPPUNIT_TEST_SUITE_END();

public:

  void setUp()
  { 
    mitk::IPythonService::ForceLoadModule(); 
  }

  void TestEvaluateOperationWithResult()
  {
    std::string result = "";
    us::ModuleContext *context = us::GetModuleContext();
    std::string filter = "(Name=PythonService)";
    auto m_PythonServiceRefs = context->GetServiceReferences<mitk::IPythonService>(filter);
    if (!m_PythonServiceRefs.empty())
    {
      mitk::IPythonService *m_PythonService =
        dynamic_cast<mitk::IPythonService *>(context->GetService<mitk::IPythonService>(m_PythonServiceRefs.front()));
      m_PythonService->Activate();
      result = m_PythonService->ExecuteString("print(5+5)\n");
    }
    else
    {
      CPPUNIT_FAIL("Error occured while TestEvaluateOperationWithResult");
    }
  }

  void TestCopyImageToAndBackPython()
  {
    std::string result = "";
    us::ModuleContext *context = us::GetModuleContext();
    std::string filter = "(Name=PythonService)";
    auto m_PythonServiceRefs = context->GetServiceReferences<mitk::IPythonService>(filter);
    if (!m_PythonServiceRefs.empty())
    {
      mitk::IPythonService *m_PythonService =
        dynamic_cast<mitk::IPythonService *>(context->GetService<mitk::IPythonService>(m_PythonServiceRefs.front()));
      m_PythonService->Activate();
      mitk::Image::Pointer image_in = mitk::IOUtil::Load<mitk::Image>(GetTestDataFilePath("Pic3D.nrrd"));
      mitk::Image::Pointer image_out;
      try
      {
        m_PythonService->TransferBaseDataToPython(image_in.GetPointer());
      }
      catch (const mitk::Exception &e)
      {
        MITK_ERROR << e.GetDescription();
        CPPUNIT_FAIL("Error in copying mitk image to Python");
      }
      try
      {
        image_out = m_PythonService->LoadImageFromPython("_mitk_image");
      }
      catch (const mitk::Exception &e)
      {
        MITK_ERROR << e.GetDescription();
        CPPUNIT_FAIL("Error in copying mitk image from Python");
      }
      CPPUNIT_ASSERT_MESSAGE("copy an image to python and back should result in equal image",
                             mitk::Equal(*image_in, *image_out, mitk::eps, true));
    }
    else
    {
      CPPUNIT_FAIL("Error occured while TestCopyImageToAndBackPython");
    }
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkPython)
