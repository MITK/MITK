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

#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
// vtk cone sample snippet
#include <vtkConeSource.h>
// vtk decimate pro snippet
#include <vtkDecimatePro.h>

#include <mitkCommonPythonTest.h>

class mitkVtkPythonTestSuite : public mitk::CommonPythonTestSuite
{
  CPPUNIT_TEST_SUITE(mitkVtkPythonTestSuite);
  MITK_TEST(testSurfaceTransfer);
  MITK_TEST(testVtkCreateConePythonSnippet);
  MITK_TEST(testVtkDecimateProPythonSnippet);
  CPPUNIT_TEST_SUITE_END();

public:

  void testSurfaceTransfer()
  {
    std::string varName("mitkSurface");
    CPPUNIT_ASSERT_MESSAGE ( "Is VTK Python Wrapping available?", m_PythonService->IsVtkPythonWrappingAvailable() == true );

    CPPUNIT_ASSERT_MESSAGE( "Valid surface copied to python import should return true.",
          m_PythonService->CopyToPythonAsVtkPolyData( m_Surface, varName) == true );

    mitk::Surface::Pointer pythonSurface = m_PythonService->CopyVtkPolyDataFromPython(varName);

    CPPUNIT_ASSERT_MESSAGE( "Compare if surfaces are equal after transfer.", mitk::Equal(*pythonSurface.GetPointer(),*m_Surface.GetPointer(),mitk::eps,true) );
  }

  void testVtkCreateConePythonSnippet()
  {
    // cone in cpp
    mitk::Surface::Pointer mitkSurface = mitk::Surface::New();
    vtkSmartPointer<vtkConeSource> coneSrc = vtkSmartPointer<vtkConeSource>::New();
    coneSrc->SetResolution(60);
    coneSrc->SetCenter(-2,0,0);
    coneSrc->Update();
    mitkSurface->SetVtkPolyData(coneSrc->GetOutput());

    // run python code
    CPPUNIT_ASSERT_MESSAGE ( "Is VTK Python Wrapping available?", m_PythonService->IsVtkPythonWrappingAvailable() == true );

    m_PythonService->Execute( m_Snippets["vtk: create cone"].toStdString(), mitk::IPythonService::MULTI_LINE_COMMAND );
    CPPUNIT_ASSERT_MESSAGE( "Python execute error occured.", !m_PythonService->PythonErrorOccured());

    mitk::Surface::Pointer pythonSurface = m_PythonService->CopyVtkPolyDataFromPython("cone");

    CPPUNIT_ASSERT_MESSAGE( "Compare if cones are equal.", mitk::Equal(*pythonSurface.GetPointer(), *mitkSurface.GetPointer(), mitk::eps,true) );
  }

  void testVtkDecimateProPythonSnippet()
  {
    // decimate pro in cpp
    mitk::Surface::Pointer mitkSurface = mitk::Surface::New();
    vtkSmartPointer<vtkDecimatePro> deci = vtkSmartPointer<vtkDecimatePro>::New();
    deci->SetInputData(m_Surface->GetVtkPolyData());
    deci->SetTargetReduction(0.9);
    deci->PreserveTopologyOn();
    deci->Update();
    mitkSurface->SetVtkPolyData(deci->GetOutput());

    // decimate pro in python
    CPPUNIT_ASSERT_MESSAGE ( "Is VTK Python Wrapping available?", m_PythonService->IsVtkPythonWrappingAvailable() == true );

    CPPUNIT_ASSERT_MESSAGE( "Valid surface copied to python import should return true.", m_PythonService->CopyToPythonAsVtkPolyData( m_Surface, "mitkSurface") == true );

    m_PythonService->Execute( m_Snippets["vtk.vtkDecimatePro"].toStdString(), mitk::IPythonService::MULTI_LINE_COMMAND );
    CPPUNIT_ASSERT_MESSAGE( "Python execute error occured.", !m_PythonService->PythonErrorOccured());

    mitk::Surface::Pointer pythonSurface = m_PythonService->CopyVtkPolyDataFromPython("mitkSurface_new");

    CPPUNIT_ASSERT_MESSAGE( "Compare if surfaces are equal.", mitk::Equal(*pythonSurface.GetPointer(), *mitkSurface.GetPointer(), mitk::eps,true) );
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkVtkPython)
