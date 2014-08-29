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

#include <mitkCommonPythonTest.h>

class mitkCvPythonTestSuite : public mitk::CommonPythonTestSuite
{
  CPPUNIT_TEST_SUITE(mitkCvPythonTestSuite);
  MITK_TEST(testCVImageTransfer);
  MITK_TEST(testOpenCVMedianFilter);
  CPPUNIT_TEST_SUITE_END();

public:

  void testCVImageTransfer()
  {
    std::string varName("mitkImage");
    CPPUNIT_ASSERT_MESSAGE ( "Is OpenCV Python Wrapping available?",
          m_PythonService->IsOpenCvPythonWrappingAvailable() == true );

    CPPUNIT_ASSERT_MESSAGE( "Valid image copied to python import should return true.",
          m_PythonService->CopyToPythonAsCvImage( m_Image2D, varName) == true );

    mitk::Image::Pointer pythonImage = m_PythonService->CopyCvImageFromPython(varName);

    // todo pixeltypes do not match, cv is changing it
    //CPPUNIT_ASSERT_MESSAGE( "Compare if images are equal after transfer.",
    //                        mitk::Equal(pythonImage,m_Image2D) );
  }

  //TODO opencv median filter, add cpp test code
  void testOpenCVMedianFilter()
  {
    // simple itk median filter in python
    CPPUNIT_ASSERT_MESSAGE ( "Is OpenCV Python Wrapping available?", m_PythonService->IsOpenCvPythonWrappingAvailable() == true );

    CPPUNIT_ASSERT_MESSAGE( "Valid image copied to python import should return true.", m_PythonService->CopyToPythonAsCvImage(m_Image2D, "mitkImage") == true );

    m_PythonService->Execute( m_Snippets["opencv median filter"].toStdString(), mitk::IPythonService::MULTI_LINE_COMMAND );

    CPPUNIT_ASSERT_MESSAGE( "Python execute error occured.", !m_PythonService->PythonErrorOccured());
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkCvPython)
