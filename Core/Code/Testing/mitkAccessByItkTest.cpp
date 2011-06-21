/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 17495 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include <stdexcept>

#include "mitkTestingMacros.h"

#include <mitkITKImageImport.h>
#include <mitkImageAccessByItk.h>

#define TestImageType(type, dim) \
  MITK_TEST_CONDITION(typeid(type) == typeid(TPixel) && dim == VDimension, "Checking for correct type itk::Image<" #type "," #dim ">")

class AccessByItkTest
{
public:

  typedef AccessByItkTest Self;

  typedef itk::Image<int, 2> IntImage2D;
  typedef itk::Image<int, 3> IntImage3D;
  typedef itk::Image<float, 2> FloatImage2D;
  typedef itk::Image<float, 3> FloatImage3D;

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

    AccessByItk_1(mitkIntImage2D, AccessItkImage, Int2D);
    AccessByItk_1(mitkIntImage3D, AccessItkImage, Int3D);
    AccessByItk_1(mitkFloatImage2D, AccessItkImage, Float2D);
    AccessByItk_1(mitkFloatImage3D, AccessItkImage, Float3D);

    AccessByItk_2(mitkIntImage2D, AccessItkImage, Int2D, 2);
    AccessByItk_2(mitkIntImage3D, AccessItkImage, Int3D, 2);
    AccessByItk_2(mitkFloatImage2D, AccessItkImage, Float2D, 2);
    AccessByItk_2(mitkFloatImage3D, AccessItkImage, Float3D, 2);

    AccessByItk_3(mitkIntImage2D, AccessItkImage, Int2D, 2, 3);
    AccessByItk_3(mitkIntImage3D, AccessItkImage, Int3D, 2, 3);
    AccessByItk_3(mitkFloatImage2D, AccessItkImage, Float2D, 2, 3);
    AccessByItk_3(mitkFloatImage3D, AccessItkImage, Float3D, 2, 3);
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

    AccessFixedDimensionByItk_1(mitkIntImage2D, AccessItkImage, 2, Int2D);
    AccessFixedDimensionByItk_1(mitkIntImage3D, AccessItkImage, 3, Int3D);
    AccessFixedDimensionByItk_1(mitkFloatImage2D, AccessItkImage, 2, Float2D);
    AccessFixedDimensionByItk_1(mitkFloatImage3D, AccessItkImage, 3, Float3D);

    AccessFixedDimensionByItk_2(mitkIntImage2D, AccessItkImage, 2, Int2D, 2);
    AccessFixedDimensionByItk_2(mitkIntImage3D, AccessItkImage, 3, Int3D, 2);
    AccessFixedDimensionByItk_2(mitkFloatImage2D, AccessItkImage, 2, Float2D, 2);
    AccessFixedDimensionByItk_2(mitkFloatImage3D, AccessItkImage, 3, Float3D, 2);

    AccessFixedDimensionByItk_3(mitkIntImage2D, AccessItkImage, 2, Int2D, 2, 3);
    AccessFixedDimensionByItk_3(mitkIntImage3D, AccessItkImage, 3, Int3D, 2, 3);
    AccessFixedDimensionByItk_3(mitkFloatImage2D, AccessItkImage, 2, Float2D, 2, 3);
    AccessFixedDimensionByItk_3(mitkFloatImage3D, AccessItkImage, 3, Float3D, 2, 3);

    // Test for wrong dimension
    MITK_TEST_FOR_EXCEPTION_BEGIN(const mitk::AccessByItkException&)
    AccessFixedDimensionByItk(mitkFloatImage3D, AccessItkImage, 2);
    MITK_TEST_FOR_EXCEPTION_END(const mitk::AccessByItkException&)

    MITK_TEST_FOR_EXCEPTION_BEGIN(const mitk::AccessByItkException&)
    AccessFixedDimensionByItk_1(mitkFloatImage3D, AccessItkImage, 2, Float3D);
    MITK_TEST_FOR_EXCEPTION_END(const mitk::AccessByItkException&)

    MITK_TEST_FOR_EXCEPTION_BEGIN(const mitk::AccessByItkException&)
    AccessFixedDimensionByItk_2(mitkFloatImage3D, AccessItkImage, 2, Float3D, 2);
    MITK_TEST_FOR_EXCEPTION_END(const mitk::AccessByItkException&)

    MITK_TEST_FOR_EXCEPTION_BEGIN(const mitk::AccessByItkException&)
    AccessFixedDimensionByItk_3(mitkFloatImage3D, AccessItkImage, 2, Float3D, 2, 3);
    MITK_TEST_FOR_EXCEPTION_END(const mitk::AccessByItkException&)
  }

  void testAccessFixedPixelTypeByItk()
  {
    mitk::Image::Pointer mitkIntImage2D = createMitkImage<IntImage2D>();
    mitk::Image::ConstPointer mitkIntImage3D(createMitkImage<IntImage3D>());
    mitk::Image::ConstPointer mitkFloatImage2D(createMitkImage<FloatImage2D>());
    mitk::Image::Pointer mitkFloatImage3D = createMitkImage<FloatImage3D>();

    AccessFixedPixelTypeByItk(mitkIntImage2D, AccessItkImage, int);
    AccessFixedPixelTypeByItk(mitkIntImage3D, AccessItkImage, int);
    AccessFixedPixelTypeByItk(mitkFloatImage2D, AccessItkImage, float);
    AccessFixedPixelTypeByItk(mitkFloatImage3D, AccessItkImage, float);

    AccessFixedPixelTypeByItk_1(mitkIntImage2D, AccessItkImage, int, Int2D);
    AccessFixedPixelTypeByItk_1(mitkIntImage3D, AccessItkImage, int, Int3D);
    AccessFixedPixelTypeByItk_1(mitkFloatImage2D, AccessItkImage, float, Float2D);
    AccessFixedPixelTypeByItk_1(mitkFloatImage3D, AccessItkImage, float, Float3D);

    AccessFixedPixelTypeByItk_2(mitkIntImage2D, AccessItkImage, int, Int2D, 2);
    AccessFixedPixelTypeByItk_2(mitkIntImage3D, AccessItkImage, int, Int3D, 2);
    AccessFixedPixelTypeByItk_2(mitkFloatImage2D, AccessItkImage, float, Float2D, 2);
    AccessFixedPixelTypeByItk_2(mitkFloatImage3D, AccessItkImage, float, Float3D, 2);

    AccessFixedPixelTypeByItk_3(mitkIntImage2D, AccessItkImage, int, Int2D, 2, 3);
    AccessFixedPixelTypeByItk_3(mitkIntImage3D, AccessItkImage, int, Int3D, 2, 3);
    AccessFixedPixelTypeByItk_3(mitkFloatImage2D, AccessItkImage, float, Float2D, 2, 3);
    AccessFixedPixelTypeByItk_3(mitkFloatImage3D, AccessItkImage, float, Float3D, 2, 3);

    // Test for wrong dimension
    MITK_TEST_FOR_EXCEPTION_BEGIN(const mitk::AccessByItkException&)
    AccessFixedPixelTypeByItk(mitkFloatImage3D, AccessItkImage, int);
    MITK_TEST_FOR_EXCEPTION_END(const mitk::AccessByItkException&)

    MITK_TEST_FOR_EXCEPTION_BEGIN(const mitk::AccessByItkException&)
    AccessFixedPixelTypeByItk_1(mitkFloatImage3D, AccessItkImage, int, Float3D);
    MITK_TEST_FOR_EXCEPTION_END(const mitk::AccessByItkException&)

    MITK_TEST_FOR_EXCEPTION_BEGIN(const mitk::AccessByItkException&)
    AccessFixedPixelTypeByItk_2(mitkFloatImage3D, AccessItkImage, int, Float3D, 2);
    MITK_TEST_FOR_EXCEPTION_END(const mitk::AccessByItkException&)

    MITK_TEST_FOR_EXCEPTION_BEGIN(const mitk::AccessByItkException&)
    AccessFixedPixelTypeByItk_3(mitkFloatImage3D, AccessItkImage, int, Float3D, 2, 3);
    MITK_TEST_FOR_EXCEPTION_END(const mitk::AccessByItkException&)
  }

  void testAccessFixedTypeByItk()
  {
    mitk::Image::Pointer mitkIntImage2D = createMitkImage<IntImage2D>();
    mitk::Image::ConstPointer mitkIntImage3D(createMitkImage<IntImage3D>());
    mitk::Image::ConstPointer mitkFloatImage2D(createMitkImage<FloatImage2D>());
    mitk::Image::Pointer mitkFloatImage3D = createMitkImage<FloatImage3D>();

    AccessFixedTypeByItk(mitkIntImage2D, AccessItkImage, int, 2);
    AccessFixedTypeByItk(mitkIntImage3D, AccessItkImage, int, 3);
    AccessFixedTypeByItk(mitkFloatImage2D, AccessItkImage, float, 2);
    AccessFixedTypeByItk(mitkFloatImage3D, AccessItkImage, float, 3);

    AccessFixedTypeByItk_1(mitkIntImage2D, AccessItkImage, int, 2, Int2D);
    AccessFixedTypeByItk_1(mitkIntImage3D, AccessItkImage, int, 3, Int3D);
    AccessFixedTypeByItk_1(mitkFloatImage2D, AccessItkImage, float, 2, Float2D);
    AccessFixedTypeByItk_1(mitkFloatImage3D, AccessItkImage, float, 3, Float3D);

    AccessFixedTypeByItk_2(mitkIntImage2D, AccessItkImage, int, 2, Int2D, 2);
    AccessFixedTypeByItk_2(mitkIntImage3D, AccessItkImage, int, 3, Int3D, 2);
    AccessFixedTypeByItk_2(mitkFloatImage2D, AccessItkImage, float, 2, Float2D, 2);
    AccessFixedTypeByItk_2(mitkFloatImage3D, AccessItkImage, float, 3, Float3D, 2);

    AccessFixedTypeByItk_3(mitkIntImage2D, AccessItkImage, int, 2, Int2D, 2, 3);
    AccessFixedTypeByItk_3(mitkIntImage3D, AccessItkImage, int, 3, Int3D, 2, 3);
    AccessFixedTypeByItk_3(mitkFloatImage2D, AccessItkImage, float, 2, Float2D, 2, 3);
    AccessFixedTypeByItk_3(mitkFloatImage3D, AccessItkImage, float, 3, Float3D, 2, 3);

    // Test for wrong dimension
    MITK_TEST_FOR_EXCEPTION_BEGIN(const mitk::AccessByItkException&)
    AccessFixedTypeByItk(mitkFloatImage3D, AccessItkImage, float, 2);
    MITK_TEST_FOR_EXCEPTION_END(const mitk::AccessByItkException&)

    MITK_TEST_FOR_EXCEPTION_BEGIN(const mitk::AccessByItkException&)
    AccessFixedTypeByItk_1(mitkFloatImage3D, AccessItkImage, float, 2, Float3D);
    MITK_TEST_FOR_EXCEPTION_END(const mitk::AccessByItkException&)

    MITK_TEST_FOR_EXCEPTION_BEGIN(const mitk::AccessByItkException&)
    AccessFixedTypeByItk_2(mitkFloatImage3D, AccessItkImage, float, 2, Float3D, 2);
    MITK_TEST_FOR_EXCEPTION_END(const mitk::AccessByItkException&)

    MITK_TEST_FOR_EXCEPTION_BEGIN(const mitk::AccessByItkException&)
    AccessFixedTypeByItk_3(mitkFloatImage3D, AccessItkImage, float, 2, Float3D, 2, 3);
    MITK_TEST_FOR_EXCEPTION_END(const mitk::AccessByItkException&)

    // Test for wrong type
    MITK_TEST_FOR_EXCEPTION_BEGIN(const mitk::AccessByItkException&)
    AccessFixedTypeByItk(mitkFloatImage3D, AccessItkImage, int, 3);
    MITK_TEST_FOR_EXCEPTION_END(const mitk::AccessByItkException&)

    MITK_TEST_FOR_EXCEPTION_BEGIN(const mitk::AccessByItkException&)
    AccessFixedTypeByItk_1(mitkFloatImage3D, AccessItkImage, int, 3, Float3D);
    MITK_TEST_FOR_EXCEPTION_END(const mitk::AccessByItkException&)

    MITK_TEST_FOR_EXCEPTION_BEGIN(const mitk::AccessByItkException&)
    AccessFixedTypeByItk_2(mitkFloatImage3D, AccessItkImage, int, 3, Float3D, 2);
    MITK_TEST_FOR_EXCEPTION_END(const mitk::AccessByItkException&)

    MITK_TEST_FOR_EXCEPTION_BEGIN(const mitk::AccessByItkException&)
    AccessFixedTypeByItk_3(mitkFloatImage3D, AccessItkImage, int, 3, Float3D, 2, 3);
    MITK_TEST_FOR_EXCEPTION_END(const mitk::AccessByItkException&)
  }

  void testAccessDefaultPixelTypesByItk()
  {
    mitk::Image::Pointer mitkIntImage2D = createMitkImage<IntImage2D>();
    mitk::Image::ConstPointer mitkIntImage3D(createMitkImage<IntImage3D>());
    mitk::Image::ConstPointer mitkFloatImage2D(createMitkImage<FloatImage2D>());
    mitk::Image::Pointer mitkFloatImage3D = createMitkImage<FloatImage3D>();

    AccessDefaultPixelTypesByItk(mitkIntImage2D);
    AccessDefaultPixelTypesByItk(mitkIntImage3D);
    AccessDefaultPixelTypesByItk(mitkFloatImage2D);
    AccessDefaultPixelTypesByItk(mitkFloatImage3D);

    AccessDefaultPixelTypesByItk_1(mitkIntImage2D, EImageType, Int2D);
    AccessDefaultPixelTypesByItk_1(mitkIntImage3D, EImageType, Int3D);
    AccessDefaultPixelTypesByItk_1(mitkFloatImage2D, EImageType, Float2D);
    AccessDefaultPixelTypesByItk_1(mitkFloatImage3D, EImageType, Float3D);

    AccessDefaultPixelTypesByItk_2(mitkIntImage2D, EImageType, Int2D, int, 2);
    AccessDefaultPixelTypesByItk_2(mitkIntImage3D, EImageType, Int3D, int, 2);
    AccessDefaultPixelTypesByItk_2(mitkFloatImage2D, EImageType, Float2D, int, 2);
    AccessDefaultPixelTypesByItk_2(mitkFloatImage3D, EImageType, Float3D, int, 2);

    AccessDefaultPixelTypesByItk_3(mitkIntImage2D, EImageType, Int2D, int, 2, int, 3);
    AccessDefaultPixelTypesByItk_3(mitkIntImage3D, EImageType, Int3D, int, 2, int, 3);
    AccessDefaultPixelTypesByItk_3(mitkFloatImage2D, EImageType, Float2D, int, 2, int, 3);
    AccessDefaultPixelTypesByItk_3(mitkFloatImage3D, EImageType, Float3D, int, 2, int, 3);
  }

  void testAccessSpecificPixelTypesByItk()
  {
    mitk::Image::Pointer mitkIntImage2D = createMitkImage<IntImage2D>();
    mitk::Image::ConstPointer mitkIntImage3D(createMitkImage<IntImage3D>());
    mitk::Image::ConstPointer mitkFloatImage2D(createMitkImage<FloatImage2D>());
    mitk::Image::Pointer mitkFloatImage3D = createMitkImage<FloatImage3D>();

    AccessIntegralPixelTypesByItk(mitkIntImage2D);
    AccessIntegralPixelTypesByItk(mitkIntImage3D);
    AccessIntegralPixelTypesByItk_1(mitkIntImage2D, EImageType, Int2D);
    AccessIntegralPixelTypesByItk_1(mitkIntImage3D, EImageType, Int3D);
    AccessIntegralPixelTypesByItk_2(mitkIntImage2D, EImageType, Int2D, int, 2);
    AccessIntegralPixelTypesByItk_2(mitkIntImage3D, EImageType, Int3D, int, 2);
    AccessIntegralPixelTypesByItk_3(mitkIntImage2D, EImageType, Int2D, int, 2, int, 3);
    AccessIntegralPixelTypesByItk_3(mitkIntImage3D, EImageType, Int3D, int, 2, int, 3);

    AccessFloatingPixelTypesByItk(mitkFloatImage2D);
    AccessFloatingPixelTypesByItk(mitkFloatImage3D);
    AccessFloatingPixelTypesByItk_1(mitkFloatImage2D, EImageType, Float2D);
    AccessFloatingPixelTypesByItk_1(mitkFloatImage3D, EImageType, Float3D);
    AccessFloatingPixelTypesByItk_2(mitkFloatImage2D, EImageType, Float2D, int, 2);
    AccessFloatingPixelTypesByItk_2(mitkFloatImage3D, EImageType, Float3D, int, 2);
    AccessFloatingPixelTypesByItk_3(mitkFloatImage2D, EImageType, Float2D, int, 2, int, 3);
    AccessFloatingPixelTypesByItk_3(mitkFloatImage3D, EImageType, Float3D, int, 2, int, 3);

    AccessSpecificPixelTypesByItk(mitkIntImage2D, short, unsigned int, float, int, char);
    AccessSpecificPixelTypesByItk(mitkIntImage3D, short, unsigned int, float, int, char);
    AccessSpecificPixelTypesByItk_1(mitkIntImage2D, EImageType, Int2D, short, unsigned int, float, int, char);
    AccessSpecificPixelTypesByItk_1(mitkFloatImage3D, EImageType, Float3D, short, unsigned int, float, int, char);
    AccessSpecificPixelTypesByItk_2(mitkIntImage2D, EImageType, Int2D, int, 2, short, unsigned int, float, int, char);
    AccessSpecificPixelTypesByItk_2(mitkFloatImage3D, EImageType, Float3D, int, 2, short, unsigned int, float, int, char);
    AccessSpecificPixelTypesByItk_3(mitkIntImage2D, EImageType, Int2D, int, 2, int, 3, short, unsigned int, float, int, char);
    AccessSpecificPixelTypesByItk_3(mitkFloatImage3D, EImageType, Float3D, int, 2, int, 3, short, unsigned int, float, int, char);

    MITK_TEST_FOR_EXCEPTION_BEGIN(const mitk::AccessByItkException&)
    AccessIntegralPixelTypesByItk(mitkFloatImage3D);
    MITK_TEST_FOR_EXCEPTION_END(const mitk::AccessByItkException&)
    MITK_TEST_FOR_EXCEPTION_BEGIN(const mitk::AccessByItkException&)
    AccessIntegralPixelTypesByItk_1(mitkFloatImage3D, EImageType, Float3D);
    MITK_TEST_FOR_EXCEPTION_END(const mitk::AccessByItkException&)
    MITK_TEST_FOR_EXCEPTION_BEGIN(const mitk::AccessByItkException&)
    AccessIntegralPixelTypesByItk_2(mitkFloatImage2D, EImageType, Float2D, int, 2);
    MITK_TEST_FOR_EXCEPTION_END(const mitk::AccessByItkException&)
    MITK_TEST_FOR_EXCEPTION_BEGIN(const mitk::AccessByItkException&)
    AccessIntegralPixelTypesByItk_3(mitkFloatImage3D, EImageType, Float3D, int, 2, int, 3);
    MITK_TEST_FOR_EXCEPTION_END(const mitk::AccessByItkException&)

    MITK_TEST_FOR_EXCEPTION_BEGIN(const mitk::AccessByItkException&)
    AccessFloatingPixelTypesByItk(mitkIntImage3D);
    MITK_TEST_FOR_EXCEPTION_END(const mitk::AccessByItkException&)
    MITK_TEST_FOR_EXCEPTION_BEGIN(const mitk::AccessByItkException&)
    AccessFloatingPixelTypesByItk_1(mitkIntImage3D, EImageType, Int3D);
    MITK_TEST_FOR_EXCEPTION_END(const mitk::AccessByItkException&)
    MITK_TEST_FOR_EXCEPTION_BEGIN(const mitk::AccessByItkException&)
    AccessFloatingPixelTypesByItk_2(mitkIntImage2D, EImageType, Int2D, int, 2);
    MITK_TEST_FOR_EXCEPTION_END(const mitk::AccessByItkException&)
    MITK_TEST_FOR_EXCEPTION_BEGIN(const mitk::AccessByItkException&)
    AccessFloatingPixelTypesByItk_3(mitkIntImage3D, EImageType, Int3D, int, 2, int, 3);
    MITK_TEST_FOR_EXCEPTION_END(const mitk::AccessByItkException&)

    MITK_TEST_FOR_EXCEPTION_BEGIN(const mitk::AccessByItkException&)
    AccessSpecificPixelTypesByItk(mitkIntImage3D, float, double, short);
    MITK_TEST_FOR_EXCEPTION_END(const mitk::AccessByItkException&)
    MITK_TEST_FOR_EXCEPTION_BEGIN(const mitk::AccessByItkException&)
    AccessSpecificPixelTypesByItk_1(mitkIntImage3D, EImageType, Int3D, float, double, short);
    MITK_TEST_FOR_EXCEPTION_END(const mitk::AccessByItkException&)
    MITK_TEST_FOR_EXCEPTION_BEGIN(const mitk::AccessByItkException&)
    AccessSpecificPixelTypesByItk_2(mitkIntImage2D, EImageType, Int2D, int, 2, float, double, short);
    MITK_TEST_FOR_EXCEPTION_END(const mitk::AccessByItkException&)
    MITK_TEST_FOR_EXCEPTION_BEGIN(const mitk::AccessByItkException&)
    AccessSpecificPixelTypesByItk_3(mitkIntImage3D, EImageType, Int3D, int, 2, int, 3, float, double, short);
    MITK_TEST_FOR_EXCEPTION_END(const mitk::AccessByItkException&)
  }

  template<typename TPixel, unsigned int VDimension>
  void AccessItkImage(itk::Image<TPixel, VDimension>*,
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

  private:

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
};

int mitkAccessByItkTest(int /*argc*/, char* /*argv*/[])
{

  MITK_TEST_BEGIN("AccessByItk")

  AccessByItkTest accessTest;

  MITK_TEST_OUTPUT(<< "Testing AccessByItk macro")
  accessTest.testAccessByItk();

  MITK_TEST_OUTPUT(<< "Testing AccessDefaultPixelTypesByItk macro")
  accessTest.testAccessDefaultPixelTypesByItk();

  MITK_TEST_OUTPUT(<< "Testing AccessFixedDimensionByItk macro")
  accessTest.testAccessFixedDimensionByItk();

  MITK_TEST_OUTPUT(<< "Testing AccessFixedTypeByItk macro")
  accessTest.testAccessFixedTypeByItk();

  MITK_TEST_OUTPUT(<< "Testing AccessFixedPixelTypeByItk macro")
  accessTest.testAccessFixedPixelTypeByItk();

  MITK_TEST_OUTPUT(<< "Testing AccessSpecificPixelTypesByItk macro")
      accessTest.testAccessSpecificPixelTypesByItk();

  MITK_TEST_END()

  return 0;
}
