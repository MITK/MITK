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
#include <mitkImage.h>
#include <mitkSurface.h>
#include <mitkIOUtil.h>
#include <mitkTestingMacros.h>
#include <mitkTestFixture.h>

#include <usModuleContext.h>
#include <usGetModuleContext.h>
#include <usServiceReference.h>
#include <usModuleRegistry.h>
#include <usModule.h>
#include <usServiceReference.h>
#include <mitkPythonService.h>
#include <mitkIPythonService.h>
#include <vtkPolyData.h>

#include <mitkImagePixelReadAccessor.h>

namespace mitk {
  static bool Equal ( mitk::Image* img1, mitk::Image* img2 );
  static bool Equal (mitk::Surface* s1, Surface *s2 );
}

bool mitk::Equal ( mitk::Image* img1, mitk::Image* img2 )
{
  mitk::ImageReadAccessor ra1(img1);
  mitk::ImageReadAccessor ra2(img2);
  const unsigned int* img1Dims = img1->GetDimensions();

  if ( img1->GetDimension() != img2->GetDimension() )
    return false;

  if( img1Dims[0] != img2->GetDimensions()[0] ||
      img1Dims[1] != img2->GetDimensions()[1] ||
      img1Dims[2] != img2->GetDimensions()[2] )
    return false;

  if ( img1->GetPixelType().GetPixelType() != img2->GetPixelType().GetPixelType())
    return false;

  if ( img1->GetGeometry()->GetSpacing()[0] != img2->GetGeometry()->GetSpacing()[0] ||
       img1->GetGeometry()->GetSpacing()[1] != img2->GetGeometry()->GetSpacing()[1] ||
       img1->GetGeometry()->GetSpacing()[2] != img2->GetGeometry()->GetSpacing()[2] )
    return false;

  size_t size = img1Dims[0] * img1Dims[1] * img1Dims[2] * img1->GetPixelType().GetSize();

  // bytewise compare the image
  for ( size_t i = 0; i < size; ++i )
  {
    if ( ((char*)ra1.GetData())[i] != ((char*) ra2.GetData())[i] )
        return false;
  }

  return true;
}

bool mitk::Equal( mitk::Surface* s1, mitk::Surface* s2 )
{
  vtkPolyData* poly1 = s1->GetVtkPolyData();
  vtkPolyData* poly2 = s2->GetVtkPolyData();
  double p1[3] = { 0.0,0.0,0.0 };
  double p2[3] = { 0.0,0.0,0.0 };

  if ( poly1->GetNumberOfPoints() != poly2->GetNumberOfPoints() ) {
      MITK_WARN << "Size does not match : s1 = " << poly1->GetNumberOfPoints()
                << ", s2 = " << poly2->GetNumberOfPoints();
      return false;
  }

  for ( vtkIdType i = 0; i < poly1->GetNumberOfPoints(); ++i )
  {
    poly1->GetPoint(i,p1);
    poly2->GetPoint(i,p2);

    if  ( !mitk::Equal(p1[0],p2[0]) ||
          !mitk::Equal(p1[1],p2[1]) ||
          !mitk::Equal(p1[2],p2[2]) )
    {
      MITK_WARN << "Points do not match: i: "<< i << "p1("
                << p1[0] << "," << p1[1] << "," << p1[2] << "), p2("
                << p2[0] << "," << p2[1] << "," << p2[2] << ")" ;
      return false;
    }
  }

  return true;
}


class mitkPythonWrappingTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPythonWrappingTestSuite);
  MITK_TEST(testImageTransfer);
  MITK_TEST(testSurfaceTransfer);
  CPPUNIT_TEST_SUITE_END();

private:
    mitk::PythonService* m_PythonService;
    mitk::Image::Pointer m_Image;
    mitk::Surface::Pointer m_Surface;
public:

  void setUp()
  {
    //get the context of the python module
    us::Module* module = us::ModuleRegistry::GetModule("MitkPython");
    us::ModuleContext* context = module->GetModuleContext();
    //get the service which is generated in the PythonModuleActivator
    us::ServiceReference<mitk::IPythonService> serviceRef = context->GetServiceReference<mitk::IPythonService>();
    m_PythonService = dynamic_cast<mitk::PythonService*>( context->GetService(serviceRef) );

    m_Image = mitk::IOUtil::LoadImage(GetTestDataFilePath("Pic3D.nrrd"));
    m_Surface = mitk::IOUtil::LoadSurface(GetTestDataFilePath("binary.stl"));
  }

  void tearDown()
  {
    m_Image = NULL;
    m_Surface = NULL;
  }

  void testImageTransfer()
  {
    std::string varName("mitkImage");
    CPPUNIT_ASSERT_MESSAGE ( "Is SimpleITK Python Wrapping available?",
          m_PythonService->IsSimpleItkPythonWrappingAvailable() == true );

    CPPUNIT_ASSERT_MESSAGE( "Valid image copied to python import should return true.",
          m_PythonService->CopyToPythonAsSimpleItkImage( m_Image, varName) == true );

    mitk::Image::Pointer pythonImage = m_PythonService->CopySimpleItkImageFromPython(varName);

    CPPUNIT_ASSERT_MESSAGE( "Compare if images are equal after transfer.",
                            mitk::Equal(pythonImage,m_Image) );
  }

  void testSurfaceTransfer()
  {
    std::string varName("mitkSurface");
    CPPUNIT_ASSERT_MESSAGE ( "Is SimpleITK Python Wrapping available?",
                      m_PythonService->IsVtkPythonWrappingAvailable() == true );

    CPPUNIT_ASSERT_MESSAGE( "Valid surface copied to python import should return true.",
          m_PythonService->CopyToPythonAsVtkPolyData( m_Surface, varName) == true );

    mitk::Surface::Pointer pythonSurface = m_PythonService->CopyVtkPolyDataFromPython(varName);

    CPPUNIT_ASSERT_MESSAGE( "Compare if surfaces are equal after transfer.",
                            mitk::Equal(pythonSurface,m_Surface) );
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkPythonWrapping)
