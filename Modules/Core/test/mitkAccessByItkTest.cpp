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

#include <stdexcept>

#include "mitkTestingMacros.h"

#include <mitkITKImageImport.h>
#include <mitkImageAccessByItk.h>

#define TestImageType(type, dim) \
  MITK_TEST_CONDITION(typeid(type) == typeid(TPixel) && dim == VDimension, "Checking for correct type itk::Image<" #type "," #dim ">")

#define TestVectorImageType(type, dim) \
  MITK_TEST_CONDITION(typeid(type) == typeid(TPixel) && dim == VDimension && \
                      typeid(itk::VariableLengthVector<type>) == typeid(typename ImageType::PixelType), "Checking for correct type itk::VectorImage<" #type "," #dim ">")


class AccessByItkTest
{
public:

  typedef AccessByItkTest Self;

  typedef itk::Image<int, 2> IntImage2D;
  typedef itk::Image<int, 3> IntImage3D;
  typedef itk::Image<float, 2> FloatImage2D;
  typedef itk::Image<float, 3> FloatImage3D;

  typedef itk::VectorImage<int, 3> IntVectorImage3D;

  enum EImageType {
    Unknown = 0,
    Int2D,
    Int3D,
    Float2D,
    Float3D
  };

  void testAccessByItk()
  {
    mitk::Image::Pointer mitkIntImage2D = createMitkImage<IntImage2D>();
    mitk::Image::ConstPointer mitkIntImage3D(createMitkImage<IntImage3D>());
    mitk::Image::ConstPointer mitkFloatImage2D(createMitkImage<FloatImage2D>());
    mitk::Image::Pointer mitkFloatImage3D = createMitkImage<FloatImage3D>();

    AccessByItk(mitkIntImage2D, AccessItkImage);
    AccessByItk(mitkIntImage3D, AccessItkImage);
    AccessByItk(mitkFloatImage2D, AccessItkImage);
    AccessByItk(mitkFloatImage3D, AccessItkImage);

    AccessByItk_n(mitkIntImage2D, AccessItkImage, (Int2D, 2));
    AccessByItk_n(mitkIntImage3D, AccessItkImage, (Int3D, 2));
    AccessByItk_n(mitkFloatImage2D, AccessItkImage, (Float2D, 2));
    AccessByItk_n(mitkFloatImage3D, AccessItkImage, (Float3D, 2));

    mitk::Image::Pointer mitkIntVectorImage3D = createMitkImage<IntVectorImage3D>(2);

    // Test for wrong pixel type (the AccessByItk macro multi-plexes integral
    // types only by default)
    MITK_TEST_FOR_EXCEPTION_BEGIN(const mitk::AccessByItkException&)
    AccessByItk(mitkIntVectorImage3D, AccessItkImage);
    MITK_TEST_FOR_EXCEPTION_END(const mitk::AccessByItkException&)

    // Test for correct handling of vector images
    AccessVectorPixelTypeByItk(mitkIntVectorImage3D, AccessItkImage);
    AccessVectorPixelTypeByItk_n(mitkIntVectorImage3D, AccessItkImage, (Int3D, 2));
  }

  void testAccessFixedDimensionByItk()
  {
    mitk::Image::Pointer mitkIntImage2D = createMitkImage<IntImage2D>();
    mitk::Image::ConstPointer mitkIntImage3D(createMitkImage<IntImage3D>());
    mitk::Image::ConstPointer mitkFloatImage2D(createMitkImage<FloatImage2D>());
    mitk::Image::Pointer mitkFloatImage3D = createMitkImage<FloatImage3D>();

    AccessFixedDimensionByItk(mitkIntImage2D, AccessItkImage, 2);
    AccessFixedDimensionByItk(mitkIntImage3D, AccessItkImage, 3);
    AccessFixedDimensionByItk(mitkFloatImage2D, AccessItkImage, 2);
    AccessFixedDimensionByItk(mitkFloatImage3D, AccessItkImage, 3);

    AccessFixedDimensionByItk_n(mitkIntImage2D, AccessItkImage, 2, (Int2D, 2));
    AccessFixedDimensionByItk_n(mitkIntImage3D, AccessItkImage, 3, (Int3D, 2));
    AccessFixedDimensionByItk_n(mitkFloatImage2D, AccessItkImage, 2, (Float2D, 2));
    AccessFixedDimensionByItk_n(mitkFloatImage3D, AccessItkImage, 3, (Float3D, 2));

    // Test for wrong dimension
    MITK_TEST_FOR_EXCEPTION_BEGIN(const mitk::AccessByItkException&)
    AccessFixedDimensionByItk(mitkFloatImage3D, AccessItkImage, 2);
    MITK_TEST_FOR_EXCEPTION_END(const mitk::AccessByItkException&)

    MITK_TEST_FOR_EXCEPTION_BEGIN(const mitk::AccessByItkException&)
    AccessFixedDimensionByItk_n(mitkFloatImage3D, AccessItkImage, 2, (Float3D, 2));
    MITK_TEST_FOR_EXCEPTION_END(const mitk::AccessByItkException&)
  }

  void testAccessFixedPixelTypeByItk()
  {
    mitk::Image::Pointer mitkIntImage2D = createMitkImage<IntImage2D>();
    mitk::Image::ConstPointer mitkIntImage3D(createMitkImage<IntImage3D>());
    mitk::Image::ConstPointer mitkFloatImage2D(createMitkImage<FloatImage2D>());
    mitk::Image::Pointer mitkFloatImage3D = createMitkImage<FloatImage3D>();

    AccessFixedPixelTypeByItk(mitkIntImage2D, AccessItkImage, (int)(float));
    AccessFixedPixelTypeByItk(mitkIntImage3D, AccessItkImage, (int)(float));
    AccessFixedPixelTypeByItk(mitkFloatImage2D, AccessItkImage, (int)(float));
    AccessFixedPixelTypeByItk(mitkFloatImage3D, AccessItkImage, (int)(float));

    AccessFixedPixelTypeByItk_n(mitkIntImage2D, AccessItkImage, (int)(float), (Int2D, 2));
    AccessFixedPixelTypeByItk_n(mitkIntImage3D, AccessItkImage, (int)(float), (Int3D, 2));
    AccessFixedPixelTypeByItk_n(mitkFloatImage2D, AccessItkImage, (int)(float), (Float2D, 2));
    AccessFixedPixelTypeByItk_n(mitkFloatImage3D, AccessItkImage, (int)(float), (Float3D, 2));

    // Test for wrong pixel type
    MITK_TEST_FOR_EXCEPTION_BEGIN(const mitk::AccessByItkException&)
    AccessFixedPixelTypeByItk(mitkFloatImage3D, AccessItkImage, (int));
    MITK_TEST_FOR_EXCEPTION_END(const mitk::AccessByItkException&)

    MITK_TEST_FOR_EXCEPTION_BEGIN(const mitk::AccessByItkException&)
    AccessFixedPixelTypeByItk_n(mitkFloatImage3D, AccessItkImage, (int), (Float3D, 2));
    MITK_TEST_FOR_EXCEPTION_END(const mitk::AccessByItkException&)

  }

  void testAccessFixedTypeByItk()
  {
    mitk::Image::Pointer mitkIntImage2D = createMitkImage<IntImage2D>();
    mitk::Image::ConstPointer mitkIntImage3D(createMitkImage<IntImage3D>());
    mitk::Image::ConstPointer mitkFloatImage2D(createMitkImage<FloatImage2D>());
    mitk::Image::Pointer mitkFloatImage3D = createMitkImage<FloatImage3D>();

    AccessFixedTypeByItk(mitkIntImage2D, AccessItkImage, (int)(float), (2)(3));
    AccessFixedTypeByItk(mitkIntImage3D, AccessItkImage, (int)(float), (2)(3));
    AccessFixedTypeByItk(mitkFloatImage2D, AccessItkImage, (int)(float), (2)(3));
    AccessFixedTypeByItk(mitkFloatImage3D, AccessItkImage, (int)(float), (2)(3));

    AccessFixedTypeByItk_n(mitkIntImage2D, AccessItkImage, (int)(float), (2)(3), (Int2D, 2));
    AccessFixedTypeByItk_n(mitkIntImage3D, AccessItkImage, (int)(float), (2)(3), (Int3D, 2));
    AccessFixedTypeByItk_n(mitkFloatImage2D, AccessItkImage, (int)(float), (2)(3), (Float2D, 2));
    AccessFixedTypeByItk_n(mitkFloatImage3D, AccessItkImage, (int)(float), (2)(3), (Float3D, 2));

    // Test for wrong dimension
    MITK_TEST_FOR_EXCEPTION_BEGIN(const mitk::AccessByItkException&)
    AccessFixedTypeByItk(mitkFloatImage3D, AccessItkImage, (float), (2));
    MITK_TEST_FOR_EXCEPTION_END(const mitk::AccessByItkException&)

    MITK_TEST_FOR_EXCEPTION_BEGIN(const mitk::AccessByItkException&)
    AccessFixedTypeByItk_n(mitkFloatImage3D, AccessItkImage, (float), (2), (Float3D, 2));
    MITK_TEST_FOR_EXCEPTION_END(const mitk::AccessByItkException&)

    // Test for wrong pixel type
    MITK_TEST_FOR_EXCEPTION_BEGIN(const mitk::AccessByItkException&)
    AccessFixedTypeByItk(mitkFloatImage3D, AccessItkImage, (int), (3));
    MITK_TEST_FOR_EXCEPTION_END(const mitk::AccessByItkException&)

    MITK_TEST_FOR_EXCEPTION_BEGIN(const mitk::AccessByItkException&)
    AccessFixedTypeByItk_n(mitkFloatImage3D, AccessItkImage, (int), (3), (Float3D, 2));
    MITK_TEST_FOR_EXCEPTION_END(const mitk::AccessByItkException&)
  }

  void testAccessTwoImagesFixedDimensionByItk()
  {
    mitk::Image::Pointer mitkIntImage2D = createMitkImage<IntImage2D>();
    mitk::Image::ConstPointer mitkFloatImage2D(createMitkImage<FloatImage2D>());

    AccessTwoImagesFixedDimensionByItk(mitkIntImage2D, mitkFloatImage2D, AccessTwoItkImages, 2);
  }

  template<typename TPixel, unsigned int VDimension>
  void AccessItkImage(const itk::Image<TPixel, VDimension>*,
                      EImageType param1 = Unknown, int param2 = 0, int param3 = 0)
  {
    switch (param1)
    {
    case Int2D:   TestImageType(int , 2) break;
    case Int3D:   TestImageType(int, 3) break;
    case Float2D: TestImageType(float, 2) break;
    case Float3D: TestImageType(float, 3) break;
    default: break;
    }

    if (param2)
    {
      MITK_TEST_CONDITION(param2 == 2, "Checking for correct second parameter")
    }
    if (param3)
    {
      MITK_TEST_CONDITION(param3 == 3, "Checking for correct third parameter")
    }
  }

  template<typename TPixel, unsigned int VDimension>
  void AccessItkImage(itk::VectorImage<TPixel, VDimension>*,
                      EImageType param1 = Unknown, int param2 = 0, int param3 = 0)
  {
    typedef itk::VectorImage<TPixel, VDimension> ImageType;
    switch (param1)
    {
    case Int2D:   TestVectorImageType(int , 2) break;
    case Int3D:   TestVectorImageType(int, 3) break;
    case Float2D: TestVectorImageType(float, 2) break;
    case Float3D: TestVectorImageType(float, 3) break;
    default: break;
    }

    if (param2)
    {
      MITK_TEST_CONDITION(param2 == 2, "Checking for correct second parameter")
    }
    if (param3)
    {
      MITK_TEST_CONDITION(param3 == 3, "Checking for correct third parameter")
    }
  }

private:

  template<typename TPixel1, unsigned int VDimension1, typename TPixel2, unsigned int VDimension2>
  void AccessTwoItkImages(itk::Image<TPixel1,VDimension1>* /*itkImage1*/, itk::Image<TPixel2,VDimension2>* /*itkImage2*/)
  {
    if (!(typeid(int) == typeid(TPixel1) && typeid(float) == typeid(TPixel2) &&
          VDimension1 == 2 && VDimension2 == 2))
    {
      throw std::runtime_error("Image type mismatch");
    }
  }

  template<typename ImageType>
  mitk::Image::Pointer createMitkImage()
  {
    typename ImageType::Pointer itkImage = ImageType::New();
    typename ImageType::IndexType start;
    start.Fill(0);
    typename ImageType::SizeType size;
    size.Fill(3);
    typename ImageType::RegionType region;
    region.SetSize(size);
    region.SetIndex(start);
    itkImage->SetRegions(region);
    itkImage->Allocate();
    return mitk::GrabItkImageMemory(itkImage);
  }

  template<typename ImageType>
  mitk::Image::Pointer createMitkImage(std::size_t vectorLength)
  {
    typename ImageType::Pointer itkImage = ImageType::New();
    typename ImageType::IndexType start;
    start.Fill(0);
    typename ImageType::SizeType size;
    size.Fill(3);
    typename ImageType::RegionType region;
    region.SetSize(size);
    region.SetIndex(start);
    itkImage->SetRegions(region);
    itkImage->SetVectorLength(vectorLength);
    itkImage->Allocate();
    return mitk::GrabItkImageMemory(itkImage);
  }

};

int mitkAccessByItkTest(int /*argc*/, char* /*argv*/[])
{

  MITK_TEST_BEGIN("AccessByItk")

  AccessByItkTest accessTest;

  MITK_TEST_OUTPUT(<< "Testing AccessByItk macro")
  accessTest.testAccessByItk();

  MITK_TEST_OUTPUT(<< "Testing AccessFixedDimensionByItk macro")
  accessTest.testAccessFixedDimensionByItk();

  MITK_TEST_OUTPUT(<< "Testing AccessFixedTypeByItk macro")
  accessTest.testAccessFixedTypeByItk();

  MITK_TEST_OUTPUT(<< "Testing AccessFixedPixelTypeByItk macro")
  accessTest.testAccessFixedPixelTypeByItk();

  MITK_TEST_OUTPUT(<< "Testing AccessTwoImagesFixedDimensionByItk macro")
  accessTest.testAccessTwoImagesFixedDimensionByItk();

  MITK_TEST_END()
}
