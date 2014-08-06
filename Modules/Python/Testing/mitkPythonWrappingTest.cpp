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
#include <QmitkPythonSnippets.h>
#include <mitkImageReadAccessor.h>
#include <mitkImageWriteAccessor.h>

#include <vtkSmartPointer.h>
// vtk cone sample snippet
#include <vtkConeSource.h>
// vtk decimate pro snippet
#include <vtkDecimatePro.h>

#include "SimpleITK.h"

namespace sitk = itk::simple;

namespace mitk {
  static bool Equal ( mitk::Image* img1, mitk::Image* img2 );
  static bool Equal ( mitk::Surface* s1, Surface *s2 );
  mitk::Image::Pointer SimpleItkToMitkImage( sitk::Image& sitkImage );
  sitk::Image MitkToSimpleItkImage( mitk::Image* image );
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

  if ( img1->GetGeometry()->GetOrigin()[0] != img2->GetGeometry()->GetOrigin()[0] ||
       img1->GetGeometry()->GetOrigin()[1] != img2->GetGeometry()->GetOrigin()[1] ||
       img1->GetGeometry()->GetOrigin()[2] != img2->GetGeometry()->GetOrigin()[2] )
    return false;

  size_t size = img1Dims[0] * img1Dims[1] * img1Dims[2] * img1->GetPixelType().GetSize();

  // bytewise compare the image
  for ( size_t i = 0; i < size; ++i )
  {
    if ( ((char*)ra1.GetData())[i] != ((char*)ra2.GetData())[i] )
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


sitk::Image mitk::MitkToSimpleItkImage( mitk::Image* image )
{
  const mitk::Vector3D spacing = image->GetGeometry()->GetSpacing();
  mitk::Point3D origin = image->GetGeometry()->GetOrigin();
  mitk::PixelType pixelType = image->GetPixelType();
  mitk::ImageReadAccessor ra(image);
  void* buffer = (void*) ra.GetData();
  sitk::ImportImageFilter importer;

  std::vector<double> sitkSpacing;
  sitkSpacing.push_back(spacing[0]);
  sitkSpacing.push_back(spacing[1]);
  sitkSpacing.push_back(spacing[2]);
  std::vector<double> sitkOrigin;
  sitkOrigin.push_back(origin[0]);
  sitkOrigin.push_back(origin[1]);
  sitkOrigin.push_back(origin[2]);
  std::vector<unsigned int> sitkSize;

  for ( unsigned int i = 0; i < image->GetDimension(); ++i )
    sitkSize.push_back(image->GetDimensions()[i]);

  importer.SetSpacing(sitkSpacing);
  importer.SetSize(sitkSize);
  importer.SetOrigin(sitkOrigin);

  if( pixelType.GetComponentType() == itk::ImageIOBase::DOUBLE ) {
    importer.SetBufferAsDouble((double*) buffer);
  } else if( pixelType.GetComponentType() == itk::ImageIOBase::FLOAT ) {
    importer.SetBufferAsFloat((float*) buffer);
  } else if( pixelType.GetComponentType() == itk::ImageIOBase::SHORT) {
    importer.SetBufferAsInt16((int16_t*) buffer);
  } else if( pixelType.GetComponentType() == itk::ImageIOBase::CHAR ) {
    importer.SetBufferAsInt8((int8_t*) buffer);
  } else if( pixelType.GetComponentType() == itk::ImageIOBase::INT ) {
    importer.SetBufferAsInt32((int32_t*) buffer);
  } else if( pixelType.GetComponentType() == itk::ImageIOBase::LONG ) {
    importer.SetBufferAsInt64((int64_t*) buffer);
  } else if( pixelType.GetComponentType() == itk::ImageIOBase::UCHAR ) {
    importer.SetBufferAsUInt8((uint8_t*) buffer);
  } else if( pixelType.GetComponentType() == itk::ImageIOBase::UINT ) {
    importer.SetBufferAsUInt32((uint32_t*) buffer);
  } else if( pixelType.GetComponentType() == itk::ImageIOBase::ULONG ) {
    importer.SetBufferAsUInt64((uint64_t*) buffer);
  } else if( pixelType.GetComponentType() == itk::ImageIOBase::USHORT ) {
    importer.SetBufferAsUInt16((uint16_t*) buffer);
  }

  return importer.Execute();
}

mitk::Image::Pointer mitk::SimpleItkToMitkImage( sitk::Image& sitkImage )
{
  mitk::Image::Pointer image = mitk::Image::New();
  void* buffer = NULL;
  mitk::PixelType pixelType = MakeScalarPixelType<short>();
  std::vector<double> sitkSpacing = sitkImage.GetSpacing();
  double spacing[3] = { sitkSpacing[0], sitkSpacing[1], sitkSpacing[2] };
  std::vector<double> sitkOrigin = sitkImage.GetOrigin();
  double origin[3] = { sitkOrigin[0], sitkOrigin[1], sitkOrigin[2] };
  std::vector<unsigned int> sitkSize = sitkImage.GetSize();
  unsigned int dimensions[4] = { 1,1,1,1};

  for ( size_t i = 0; i < sitkSize.size(); ++i )
    dimensions[i] = sitkSize[i];

  size_t size = 0;
  if ( sitkImage.GetPixelIDValue() == sitk::sitkInt8 ) {
     pixelType = MakeScalarPixelType<char>();
     buffer = (void*) sitkImage.GetBufferAsInt8();
     size = sizeof(char);
  } else if( sitkImage.GetPixelIDValue() == sitk::sitkInt16 ) {
     pixelType = MakeScalarPixelType<short>();
     buffer = (void*) sitkImage.GetBufferAsInt16();
     size = sizeof(short);
  } else if( sitkImage.GetPixelIDValue() == sitk::sitkInt32 ) {
     pixelType = MakeScalarPixelType<int>();
     buffer = (void*) sitkImage.GetBufferAsInt32();
     size = sizeof(int);
  } else if( sitkImage.GetPixelIDValue() == sitk::sitkInt64 ) {
     pixelType = MakeScalarPixelType<long>();
     buffer = (void*) sitkImage.GetBufferAsInt64();
     size = sizeof(long);
  } else if( sitkImage.GetPixelIDValue() == sitk::sitkUInt8 ) {
     pixelType = MakeScalarPixelType<unsigned char>();
     buffer = (void*) sitkImage.GetBufferAsUInt8();
     size = sizeof(unsigned char);
  } else if( sitkImage.GetPixelIDValue() == sitk::sitkUInt16 ) {
     pixelType = MakeScalarPixelType<unsigned short>();
     buffer = (void*) sitkImage.GetBufferAsUInt16();
     size = sizeof(unsigned short);
  } else if( sitkImage.GetPixelIDValue() == sitk::sitkUInt32 ) {
     pixelType = MakeScalarPixelType<unsigned int>();
     buffer = (void*) sitkImage.GetBufferAsUInt32();
     size = sizeof(unsigned int);
  } else if( sitkImage.GetPixelIDValue() == sitk::sitkUInt64 ) {
     pixelType = MakeScalarPixelType<unsigned long>();
     buffer = (void*) sitkImage.GetBufferAsUInt64();
     size = sizeof(unsigned long);
  } else if( sitkImage.GetPixelIDValue() == sitk::sitkFloat32 ) {
     pixelType = MakeScalarPixelType<float>();
     buffer = (void*) sitkImage.GetBufferAsFloat();
     size = sizeof(float);
  } else if( sitkImage.GetPixelIDValue() == sitk::sitkFloat64 ) {
     pixelType = MakeScalarPixelType<float>();
     buffer = (void*) sitkImage.GetBufferAsDouble();
     size = sizeof(double);
  }

  image->Initialize(pixelType,sitkImage.GetDimension(),dimensions);
  image->SetSpacing(spacing);
  image->SetOrigin(origin);

  for(size_t i = 0; i < sitkSize.size(); ++i )
    size *= sitkSize[i];

  mitk::ImageWriteAccessor wa(image);

  memcpy(wa.GetData(),buffer, size);

  return image;
}


class mitkPythonWrappingTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPythonWrappingTestSuite);
  MITK_TEST(testImageTransfer);
  MITK_TEST(testSurfaceTransfer);
  MITK_TEST(testVtkCreateConePythonSnippet);
  MITK_TEST(testVtkDecimateProPythonSnippet);
  MITK_TEST(testSimpleITKMedianFilterSnippet);
  CPPUNIT_TEST_SUITE_END();

private:
    mitk::PythonService* m_PythonService;
    mitk::Image::Pointer m_Image;
    mitk::Surface::Pointer m_Surface;
    QMap<QString, QString> m_Snippets;

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

    QmitkPythonSnippets::LoadStringMap(QmitkPythonSnippets::DEFAULT_SNIPPET_FILE,m_Snippets);
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
    CPPUNIT_ASSERT_MESSAGE ( "Is VTK Python Wrapping available?", m_PythonService->IsVtkPythonWrappingAvailable() == true );

    CPPUNIT_ASSERT_MESSAGE( "Valid surface copied to python import should return true.",
          m_PythonService->CopyToPythonAsVtkPolyData( m_Surface, varName) == true );

    mitk::Surface::Pointer pythonSurface = m_PythonService->CopyVtkPolyDataFromPython(varName);

    CPPUNIT_ASSERT_MESSAGE( "Compare if surfaces are equal after transfer.", mitk::Equal(pythonSurface,m_Surface) );
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

    mitk::Surface::Pointer pythonSurface = m_PythonService->CopyVtkPolyDataFromPython("cone");

    CPPUNIT_ASSERT_MESSAGE( "Compare if cones are equal.", mitk::Equal(pythonSurface, mitkSurface) );
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

    mitk::Surface::Pointer pythonSurface = m_PythonService->CopyVtkPolyDataFromPython("mitkSurface_new");

    CPPUNIT_ASSERT_MESSAGE( "Compare if surfaces are equal.", mitk::Equal(pythonSurface, mitkSurface) );
  }

  void testSimpleITKMedianFilterSnippet()
  {
    // simple itk median filter in cpp
    sitk::MedianImageFilter medianFilter;
    medianFilter.SetRadius(1);
    sitk::Image sitkImage = medianFilter.Execute(mitk::MitkToSimpleItkImage(m_Image));
    mitk::Image::Pointer mitkImage = mitk::SimpleItkToMitkImage(sitkImage);

    // simple itk median filter in python
    CPPUNIT_ASSERT_MESSAGE ( "Is SimpleItk Python Wrapping available?", m_PythonService->IsSimpleItkPythonWrappingAvailable() == true );

    CPPUNIT_ASSERT_MESSAGE( "Valid image copied to python import should return true.", m_PythonService->CopyToPythonAsSimpleItkImage(m_Image, "mitkImage") == true );

    m_PythonService->Execute( m_Snippets["medianfilter"].toStdString(), mitk::IPythonService::MULTI_LINE_COMMAND );

    mitk::Image::Pointer pythonImage = m_PythonService->CopySimpleItkImageFromPython("mitkImage_new");

    CPPUNIT_ASSERT_MESSAGE( "Compare if images are equal.", mitk::Equal(pythonImage, mitkImage) );
  }
  //TODO opencv median filter
};

MITK_TEST_SUITE_REGISTRATION(mitkPythonWrapping)
