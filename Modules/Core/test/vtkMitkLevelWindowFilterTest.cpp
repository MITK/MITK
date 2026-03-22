/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

// This test covers vtkMitkLevelWindowFilter with RGB inputs, specifically the
// vtkApplyLookupTableOnRGBA code path. Prior to a bug fix, the output
// vtkImageIterator was templated on the *input* scalar type T instead of being
// fixed to unsigned char. Since RequestInformation unconditionally declares the
// output as VTK_UNSIGNED_CHAR with 4 components, using vtkImageIterator<float>
// for the output buffer caused incorrect memory writes (4-byte float values
// written into 1-byte slots) and produced corrupted output for any RGB input
// type other than unsigned char. The fix changes the output iterator to always
// use vtkImageIterator<unsigned char>, matching the declared output type.

#include <mitkTestFixture.h>
#include <mitkTestingMacros.h>

#include <vtkMitkLevelWindowFilter.h>

#include <vtkImageData.h>
#include <vtkLookupTable.h>
#include <vtkSmartPointer.h>

class vtkMitkLevelWindowFilterTestSuite : public mitk::TestFixture
{
  CPPUNIT_TEST_SUITE(vtkMitkLevelWindowFilterTestSuite);
  MITK_TEST(TestRgbUcharOutputType);
  MITK_TEST(TestRgbFloatOutputType);
  MITK_TEST(TestRgbFloatValuesCorrect);
  MITK_TEST(TestClippingZeroesOutsidePixels);
  MITK_TEST(TestScalarUcharOutputType);
  CPPUNIT_TEST_SUITE_END();

private:
  vtkSmartPointer<vtkLookupTable> m_Lut;

  // Creates a filter with the given input and clipping bounds [xMin,xMax) x [yMin,yMax).
  vtkSmartPointer<vtkMitkLevelWindowFilter> CreateFilter(
    vtkImageData *input, double xMin, double xMax, double yMin, double yMax) const
  {
    auto filter = vtkSmartPointer<vtkMitkLevelWindowFilter>::New();
    filter->SetInputData(input);
    filter->SetLookupTable(m_Lut);
    double bounds[4] = {xMin, xMax, yMin, yMax};
    filter->SetClippingBounds(bounds);
    filter->SetMinOpacity(0.0);
    filter->SetMaxOpacity(255.0);
    return filter;
  }

public:
  void setUp() override
  {
    // Identity LUT: full [0, 255] range, fully opaque. The range must be set
    // before Build() so GetTableRange() returns the correct values when
    // vtkApplyLookupTableOnRGBA computes the level-window scale and bias.
    m_Lut = vtkSmartPointer<vtkLookupTable>::New();
    m_Lut->SetRange(0.0, 255.0);
    m_Lut->SetAlphaRange(1.0, 1.0);
    m_Lut->Build();
  }

  void tearDown() override { m_Lut = nullptr; }

  // RGB unsigned-char input — baseline: the vtkApplyLookupTableOnRGBA path
  // worked correctly for uchar before the fix (T == unsigned char by coincidence).
  // Verify that the output scalar type and component count are correct.
  void TestRgbUcharOutputType()
  {
    auto input = vtkSmartPointer<vtkImageData>::New();
    input->SetDimensions(1, 1, 1);
    input->AllocateScalars(VTK_UNSIGNED_CHAR, 3);
    auto *pixel = static_cast<unsigned char *>(input->GetScalarPointer(0, 0, 0));
    pixel[0] = 200;
    pixel[1] = 200;
    pixel[2] = 200;

    auto filter = CreateFilter(input, 0.0, 1.0, 0.0, 1.0);
    filter->Update();

    vtkImageData *output = filter->GetOutput();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("RGB uchar input: output scalar type must be VTK_UNSIGNED_CHAR",
                                 static_cast<int>(VTK_UNSIGNED_CHAR),
                                 output->GetScalarType());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("RGB uchar input: output must have 4 components (RGBA)",
                                 4,
                                 output->GetNumberOfScalarComponents());
  }

  // RGB float input — before the fix, vtkApplyLookupTableOnRGBA<float> used
  // vtkImageIterator<float> for the output buffer, which is allocated as
  // unsigned char. This caused the iterator to advance 4 bytes per write
  // instead of 1, corrupting the buffer. Verify the output type is correct.
  void TestRgbFloatOutputType()
  {
    auto input = vtkSmartPointer<vtkImageData>::New();
    input->SetDimensions(1, 1, 1);
    input->AllocateScalars(VTK_FLOAT, 3);
    auto *pixel = static_cast<float *>(input->GetScalarPointer(0, 0, 0));
    pixel[0] = 200.0f;
    pixel[1] = 200.0f;
    pixel[2] = 200.0f;

    auto filter = CreateFilter(input, 0.0, 1.0, 0.0, 1.0);
    filter->Update();

    vtkImageData *output = filter->GetOutput();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("RGB float input: output scalar type must be VTK_UNSIGNED_CHAR",
                                 static_cast<int>(VTK_UNSIGNED_CHAR),
                                 output->GetScalarType());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("RGB float input: output must have 4 components (RGBA)",
                                 4,
                                 output->GetNumberOfScalarComponents());
  }

  // RGB float input — verify that the output *values* are correct after the fix.
  // A neutral gray (R=G=B) has S=0 in HSI space, so the HSI round-trip is
  // mathematically exact. With the old broken output iterator, the raw 4-byte
  // float representation of ~200.0 was written into the 1-byte-per-component
  // output slots, producing completely wrong values.
  void TestRgbFloatValuesCorrect()
  {
    auto input = vtkSmartPointer<vtkImageData>::New();
    input->SetDimensions(1, 1, 1);
    input->AllocateScalars(VTK_FLOAT, 3);
    auto *pixel = static_cast<float *>(input->GetScalarPointer(0, 0, 0));
    pixel[0] = 200.0f;
    pixel[1] = 200.0f;
    pixel[2] = 200.0f;

    auto filter = CreateFilter(input, 0.0, 1.0, 0.0, 1.0);
    filter->Update();

    const auto *out =
      static_cast<unsigned char *>(filter->GetOutput()->GetScalarPointer(0, 0, 0));

    // Allow a tolerance of ±2 for floating-point rounding in the HSI conversion.
    const int r = out[0], g = out[1], b = out[2];
    CPPUNIT_ASSERT_MESSAGE("RGB float input: red channel must be ~200 after identity level window",
                           r >= 198 && r <= 202);
    CPPUNIT_ASSERT_MESSAGE("RGB float input: green channel must be ~200 after identity level window",
                           g >= 198 && g <= 202);
    CPPUNIT_ASSERT_MESSAGE("RGB float input: blue channel must be ~200 after identity level window",
                           b >= 198 && b <= 202);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("RGB float input: alpha must be 255 (no alpha channel in source)",
                                 static_cast<int>(255),
                                 static_cast<int>(out[3]));
  }

  // Pixels outside the clipping rectangle must be rendered as transparent black (0,0,0,0).
  // This test uses a 2x2 RGB image (all white) with clipping bounds [0,1) x [0,1),
  // so only pixel (0,0) is inside; the other three must be zeroed.
  void TestClippingZeroesOutsidePixels()
  {
    auto input = vtkSmartPointer<vtkImageData>::New();
    input->SetDimensions(2, 2, 1);
    input->AllocateScalars(VTK_UNSIGNED_CHAR, 3);
    for (int y = 0; y < 2; ++y)
    {
      for (int x = 0; x < 2; ++x)
      {
        auto *p = static_cast<unsigned char *>(input->GetScalarPointer(x, y, 0));
        p[0] = 255;
        p[1] = 255;
        p[2] = 255;
      }
    }

    auto filter = CreateFilter(input, 0.0, 1.0, 0.0, 1.0);
    filter->Update();

    vtkImageData *output = filter->GetOutput();

    const auto *inside = static_cast<unsigned char *>(output->GetScalarPointer(0, 0, 0));
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Clipping: pixel (0,0) inside bounds must have alpha 255",
                                 static_cast<int>(255),
                                 static_cast<int>(inside[3]));

    // x=1 does not satisfy x < 1.0, so pixel (1,0) must be all zeros.
    const auto *outsideX = static_cast<unsigned char *>(output->GetScalarPointer(1, 0, 0));
    CPPUNIT_ASSERT_MESSAGE("Clipping: pixel (1,0) outside x bounds must be transparent black",
                           outsideX[0] == 0 && outsideX[1] == 0 && outsideX[2] == 0 && outsideX[3] == 0);

    // y=1 does not satisfy y < 1.0, so pixel (0,1) must be all zeros.
    const auto *outsideY = static_cast<unsigned char *>(output->GetScalarPointer(0, 1, 0));
    CPPUNIT_ASSERT_MESSAGE("Clipping: pixel (0,1) outside y bounds must be transparent black",
                           outsideY[0] == 0 && outsideY[1] == 0 && outsideY[2] == 0 && outsideY[3] == 0);
  }

  // Single-component input takes a completely separate code path
  // (vtkApplyLookupTableOnScalars / vtkApplyLookupTableOnScalarsFast).
  // The fix to vtkApplyLookupTableOnRGBA must not affect it.
  void TestScalarUcharOutputType()
  {
    auto input = vtkSmartPointer<vtkImageData>::New();
    input->SetDimensions(1, 1, 1);
    input->AllocateScalars(VTK_UNSIGNED_CHAR, 1);
    *static_cast<unsigned char *>(input->GetScalarPointer(0, 0, 0)) = 128;

    auto filter = CreateFilter(input, 0.0, 1.0, 0.0, 1.0);
    filter->Update();

    vtkImageData *output = filter->GetOutput();
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Scalar uchar input: output scalar type must be VTK_UNSIGNED_CHAR",
                                 static_cast<int>(VTK_UNSIGNED_CHAR),
                                 output->GetScalarType());
    CPPUNIT_ASSERT_EQUAL_MESSAGE("Scalar uchar input: output must have 4 components (RGBA)",
                                 4,
                                 output->GetNumberOfScalarComponents());
  }
};

MITK_TEST_SUITE_REGISTRATION(vtkMitkLevelWindowFilter)
