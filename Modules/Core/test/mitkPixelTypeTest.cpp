/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// Testing
#include "mitkTestFixture.h"
#include "mitkTestingMacros.h"
// std includes
#include <string>
// MITK includes
#include "mitkPixelType.h"
#include <mitkLogMacros.h>
// ITK includes
#include "itkImage.h"
#include <itkVectorImage.h>
// VTK includes
#include <vtkDebugLeaks.h>

struct MyObscurePixelType
{
  typedef float ValueType;
  static const size_t Length = 2;
};

//## Documentation
//## main testing method
class mitkPixelTypeTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(mitkPixelTypeTestSuite);

  MITK_TEST(GetComponentType_Success);
  MITK_TEST(GetPixelTypeAsString_Success);
  MITK_TEST(GetBpePtype_Success);
  MITK_TEST(GetNumberOfComponentsPtype_Success);
  MITK_TEST(GetBitsPerComponentPtype_Success);
  MITK_TEST(GetBpeItkPtype_Success);
  MITK_TEST(GetNumberOfComponentsItkPtype_Success);
  MITK_TEST(GetBitsPerComponentItkPtype_Success);
  MITK_TEST(ConstructorWithBrackets_Success);
  MITK_TEST(InitializeWithEqualSign_Success);
  MITK_TEST(EqualityOperator_Success);
  MITK_TEST(NotEqualityOperator_Success);
  MITK_TEST(GetPixelTypeUnknown_Success);
  MITK_TEST(SetLengthCorrectly_Success);
  MITK_TEST(ValueTypeCorresponds_Success);
  MITK_TEST(ImageTypeTraitInt3D_Success);
  MITK_TEST(ImageTypeTraitShort2D_Success);
  MITK_TEST(ImageTypeTraitVectorShort3D_Success);
  MITK_TEST(ImageTypeTraitItkInt3D_Success);
  MITK_TEST(ImageTypeTraitItkShort2D_Success);
  MITK_TEST(ImageTypeTraitItkVectorShort3D_Success);

  CPPUNIT_TEST_SUITE_END();

private:
  typedef itk::Image<itk::FixedArray<int, 5>, 3> ItkImageType;
  typedef itk::Image<MyObscurePixelType> MyObscureImageType;
  typedef itk::VectorImage<short, 3> VectorImageType;
  typedef itk::Image<itk::Vector<short, 7>> FixedVectorImageType;


public:
  void setUp() override
  {
  }

  void tearDown() override
  {
  }

  void GetComponentType_Success()
  {
    mitk::PixelType ptype = mitk::MakePixelType<int, int, 5>();
    MITK_INFO << "m_Ptype = mitk::MakePixelType<int, int, 5>();";
    MITK_INFO
      << "m_ItkPtype = mitk::MakePixelType<ItkImageType>();\n with  itk::Image<itk::FixedArray< int, 5>, 3> ItkImageType";
    CPPUNIT_ASSERT_MESSAGE("GetComponentType()", ptype.GetComponentType() == itk::ImageIOBase::INT);
  }

  void GetPixelTypeAsString_Success()
  {
    mitk::PixelType ptype = mitk::MakePixelType<int, int, 5>();
      MITK_INFO << ptype.GetPixelTypeAsString();
      MITK_INFO << typeid(ItkImageType).name();
  }

  void GetBpePtype_Success()
  {
    mitk::PixelType ptype = mitk::MakePixelType<int, int, 5>();
    CPPUNIT_ASSERT_MESSAGE("[m_Ptype] GetBpe()", ptype.GetBpe() == 8 * sizeof(int) * 5);
  }

  void GetNumberOfComponentsPtype_Success()
  {
    mitk::PixelType ptype = mitk::MakePixelType<int, int, 5>();
    CPPUNIT_ASSERT_MESSAGE("[ptype]GetNumberOfComponents()", ptype.GetNumberOfComponents() == 5);
  }

  void GetBitsPerComponentPtype_Success()
  {
    mitk::PixelType ptype = mitk::MakePixelType<int, int, 5>();
    CPPUNIT_ASSERT_MESSAGE("[ptype]GetBitsPerComponent()", ptype.GetBitsPerComponent() == 8 * sizeof(int));
  }

  void GetBpeItkPtype_Success()
  {
    mitk::PixelType itkPtype = mitk::MakePixelType<ItkImageType>();
    CPPUNIT_ASSERT_MESSAGE("[itkPType] GetBpe()", itkPtype.GetBpe() == 8 * sizeof(int) * 5);
  }

  void GetNumberOfComponentsItkPtype_Success()
  {
    mitk::PixelType itkPtype = mitk::MakePixelType<ItkImageType>();
    CPPUNIT_ASSERT_MESSAGE("[itkPType] GetNumberOfComponents()", itkPtype.GetNumberOfComponents() == 5);
  }

  void GetBitsPerComponentItkPtype_Success()
  {
    mitk::PixelType itkPtype = mitk::MakePixelType<ItkImageType>();
    CPPUNIT_ASSERT_MESSAGE("[itkPType] GetBitsPerComponent()", itkPtype.GetBitsPerComponent() == 8 * sizeof(int));
  }

  void ConstructorWithBrackets_Success()
  {
    mitk::PixelType ptype = mitk::MakePixelType<int, int, 5>();
    mitk::PixelType ptype2(ptype);
    CPPUNIT_ASSERT_MESSAGE("ptype2( ptype)- GetComponentType()", ptype2.GetComponentType() == itk::ImageIOBase::INT);
    CPPUNIT_ASSERT_MESSAGE("ptype2( ptype)-GetPixelType(", ptype2.GetPixelType() == ptype.GetPixelType());
    CPPUNIT_ASSERT_MESSAGE("ptype2( ptype)-GetComponentType(", ptype2.GetComponentType() == ptype.GetComponentType());
    CPPUNIT_ASSERT_MESSAGE("ptype2( ptype)-GetBpe()", ptype2.GetBpe() == 8 * sizeof(int) * 5);
    CPPUNIT_ASSERT_MESSAGE("ptype2( ptype)-GetNumberOfComponents()", ptype2.GetNumberOfComponents() == 5);
    CPPUNIT_ASSERT_MESSAGE("ptype2( ptype)-GetBitsPerComponent()", ptype2.GetBitsPerComponent() == 8 * sizeof(int));
  }

  void InitializeWithEqualSign_Success()
  {
    mitk::PixelType ptype = mitk::MakePixelType<int, int, 5>();
    mitk::PixelType ptype2 = ptype;
    CPPUNIT_ASSERT_MESSAGE("ptype2 = ptype- GetComponentType()", ptype2.GetComponentType() == itk::ImageIOBase::INT);
    CPPUNIT_ASSERT_MESSAGE("ptype2 = ptype- GetPixelType(", ptype2.GetPixelType() == ptype.GetPixelType());
    CPPUNIT_ASSERT_MESSAGE("ptype2( ptype)-GetComponentType(", ptype2.GetComponentType() == ptype.GetComponentType());
    CPPUNIT_ASSERT_MESSAGE("ptype2 = ptype- GetBpe()", ptype2.GetBpe() == 8 * sizeof(int) * 5);
    CPPUNIT_ASSERT_MESSAGE("ptype2 = ptype- GetNumberOfComponents()", ptype2.GetNumberOfComponents() == 5);
    CPPUNIT_ASSERT_MESSAGE("ptype2 = ptype- GetBitsPerComponent()", ptype2.GetBitsPerComponent() == 8 * sizeof(int));
  }

  void EqualityOperator_Success()
  {
    mitk::PixelType ptype = mitk::MakePixelType<int, int, 5>();
    mitk::PixelType ptype2 = ptype;
    CPPUNIT_ASSERT_MESSAGE("operator ==", ptype == ptype2);
  }

  void NotEqualityOperator_Success()
  {
    mitk::PixelType ptype = mitk::MakePixelType<int, int, 5>();
    mitk::PixelType ptype3 = mitk::MakePixelType<char, char ,5>();
    CPPUNIT_ASSERT_MESSAGE("operator !=", ptype != ptype3);
  }

  void GetPixelTypeUnknown_Success()
  {
    // test instantiation
    mitk::PixelType obscurePixelType = mitk::MakePixelType<MyObscureImageType>();
    CPPUNIT_ASSERT_MESSAGE("PixelTypeId is 'UNKNOWN' ", obscurePixelType.GetPixelType() == itk::ImageIOBase::UNKNOWNPIXELTYPE);
  }

  void SetLengthCorrectly_Success()
  {
    mitk::PixelType obscurePixelType = mitk::MakePixelType<MyObscureImageType>();
    CPPUNIT_ASSERT_MESSAGE("Lenght was set correctly", obscurePixelType.GetNumberOfComponents() == MyObscurePixelType::Length);
  }

  void ValueTypeCorresponds_Success()
  {
    mitk::PixelType obscurePixelType = mitk::MakePixelType<MyObscureImageType>();
    CPPUNIT_ASSERT_MESSAGE("ValueType corresponds.",

      obscurePixelType.GetComponentType() == mitk::MapPixelComponentType<MyObscurePixelType::ValueType>::value);
  }

  void ImageTypeTraitInt3D_Success()
  {
    // test ImageTypeTrait traits class
    CPPUNIT_ASSERT_MESSAGE("ImageTypeTrait typeid int 3D equals ITK typeid", 
      
      typeid(mitk::ImageTypeTrait<int, 3>::ImageType) == typeid(itk::Image<int, 3>));
    CPPUNIT_ASSERT_MESSAGE("ImageTypeTrait is no vector image", (mitk::ImageTypeTrait<int, 3>::IsVectorImage == false));
  }

  void ImageTypeTraitShort2D_Success()
  {
    CPPUNIT_ASSERT_MESSAGE("ImageTypeTrait typeid short 2D equals ITK typeid",
                          typeid(mitk::ImageTypeTrait<itk::FixedArray<short, 2>, 3>::ImageType) ==
                          typeid(itk::Image<itk::FixedArray<short, 2>, 3>));
    CPPUNIT_ASSERT_MESSAGE("ImageTypeTrait is no vector image", (mitk::ImageTypeTrait<itk::FixedArray<short, 2>, 3>::IsVectorImage == false));
  }

  void ImageTypeTraitVectorShort3D_Success()
  {
    CPPUNIT_ASSERT_MESSAGE("ImageTypeTrait typeid short 3D equals ITK typeid",
        typeid(mitk::ImageTypeTrait<itk::VariableLengthVector<short>, 3>::ImageType) == typeid(itk::VectorImage<short, 3>));
    CPPUNIT_ASSERT_MESSAGE("ImageTypeTrait is a vector image", (mitk::ImageTypeTrait<itk::VariableLengthVector<short>, 3>::IsVectorImage == true));
  }

  void ImageTypeTraitItkInt3D_Success()
  {
    CPPUNIT_ASSERT_MESSAGE("ImageTypeTrait typeid ITK int 3D equals ITK typeid",
      typeid(mitk::ImageTypeTrait<itk::Image<int, 3>>::ImageType) == typeid(itk::Image<int, 3>));
    CPPUNIT_ASSERT_MESSAGE("ImageTypeTrait is no vector image", (mitk::ImageTypeTrait<itk::Image<int, 3>>::IsVectorImage == false));
  }

  void ImageTypeTraitItkShort2D_Success()
  {
    CPPUNIT_ASSERT_MESSAGE("ImageTypeTrait typeid ITK short 2D equals ITK typeid",
                          typeid(mitk::ImageTypeTrait<itk::Image<itk::FixedArray<short, 2>, 3>>::ImageType) ==
                          typeid(itk::Image<itk::FixedArray<short, 2>, 3>));
    CPPUNIT_ASSERT_MESSAGE("ImageTypeTrait is no vector image",

      (mitk::ImageTypeTrait<itk::Image<itk::FixedArray<short, 2>, 3>>::IsVectorImage == false));
  }

  void ImageTypeTraitItkVectorShort3D_Success()
  {
    CPPUNIT_ASSERT_MESSAGE("ImageTypeTrait typeid ITK short 3D equals ITK typeid",
                           typeid(mitk::ImageTypeTrait<itk::VectorImage<short, 3>>::ImageType) == typeid(itk::VectorImage<short, 3>));
    CPPUNIT_ASSERT_MESSAGE("ImageTypeTrait is a vector image",
                            (mitk::ImageTypeTrait<itk::VectorImage<short, 3>>::IsVectorImage == true));
  }
};
MITK_TEST_SUITE_REGISTRATION(mitkPixelType)


