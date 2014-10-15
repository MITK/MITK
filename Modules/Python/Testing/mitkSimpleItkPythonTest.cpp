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
#include <mitkImageReadAccessor.h>
#include <mitkImageWriteAccessor.h>
#include <mitkCommonPythonTest.h>

#include "SimpleITK.h"

namespace sitk = itk::simple;

namespace mitk {
  static mitk::Image::Pointer SimpleItkToMitkImage( sitk::Image& sitkImage );
  static sitk::Image MitkToSimpleItkImage( mitk::Image* image );
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


class mitkSimpleItkPythonTestSuite : public mitk::CommonPythonTestSuite
{
  CPPUNIT_TEST_SUITE(mitkSimpleItkPythonTestSuite);
  MITK_TEST(testSimpleItkImageTransfer);
  MITK_TEST(testSimpleITKMedianFilterSnippet);
  CPPUNIT_TEST_SUITE_END();


public:

  void testSimpleItkImageTransfer()
  {
    std::string varName("mitkImage");
    CPPUNIT_ASSERT_MESSAGE ( "Is SimpleITK Python Wrapping available?",
          m_PythonService->IsSimpleItkPythonWrappingAvailable() == true );

    CPPUNIT_ASSERT_MESSAGE( "Valid image copied to python import should return true.",
          m_PythonService->CopyToPythonAsSimpleItkImage( m_Image, varName) == true );

    mitk::Image::Pointer pythonImage = m_PythonService->CopySimpleItkImageFromPython(varName);

    CPPUNIT_ASSERT_MESSAGE( "Compare if images are equal after transfer.",
                            mitk::Equal(*pythonImage.GetPointer(),*m_Image.GetPointer(), mitk::eps,true) );
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
    CPPUNIT_ASSERT_MESSAGE( "Python execute error occured.", !m_PythonService->PythonErrorOccured());

    mitk::Image::Pointer pythonImage = m_PythonService->CopySimpleItkImageFromPython("mitkImage_new");

    CPPUNIT_ASSERT_MESSAGE( "Compare if images are equal.", mitk::Equal(*pythonImage.GetPointer(), *mitkImage.GetPointer(),mitk::eps,true) );
  }
};

MITK_TEST_SUITE_REGISTRATION(mitkSimpleItkPython)
