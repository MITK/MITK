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

#include <mitkNodePredicateDataType.h>
#include <mitkRenderingTestHelper.h>
#include <mitkSmartPointerProperty.h>
#include <mitkTestingMacros.h>
#include <vtkSmartPointer.h>
#include "mitkTestingMacros.h"
#include <itksys/SystemTools.hxx>
#include <mitkTestingConfig.h>
#include <mitkIOUtil.h>
#include <mitkTestFixture.h>
#include <vtkTesting.h>
#include <mitkPeakImage.h>
#include <mitkShImage.h>
#include <mitkPreferenceListReaderOptionsFunctor.h>
#include <mitkImagePixelReadAccessor.h>


class mitkPeakShImageReaderTestSuite : public mitk::TestFixture
{

  CPPUNIT_TEST_SUITE(mitkPeakShImageReaderTestSuite);
  MITK_TEST(PeakReader);
  MITK_TEST(ShReader);
  CPPUNIT_TEST_SUITE_END();

  typedef itk::Image<float, 3> ItkFloatImgType;

private:

  /** Members used inside the different (sub-)tests. All members are initialized via setUp().*/

public:

  void setUp() override
  {

  }

  void tearDown() override
  {

  }

  void PeakReader()
  {
    mitk::PreferenceListReaderOptionsFunctor functor = mitk::PreferenceListReaderOptionsFunctor({"Peak Image"}, {});
    auto inputData = mitk::IOUtil::Load<mitk::PeakImage>(GetTestDataFilePath("DiffusionImaging/peak_image_test.nii.gz"), &functor);
    std::string class_name = "PeakImage";
    bool ok = true;
    if (class_name.compare(inputData->GetNameOfClass())!=0)
      ok = false;
    MITK_TEST_CONDITION_REQUIRED(ok, "Check class name = PeakImage");

    mitk::Vector3D sp;
    sp[0] = 1.23;
    sp[1] = 2.001;
    sp[2] = 1.409;
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(sp, inputData->GetGeometry()->GetSpacing(), 0.0001, true), "Check spacing ok");

    mitk::Point3D o;
    o[0] = 0.615;
    o[1] = 1.0005;
    o[2] = 0.7045;
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(o, inputData->GetGeometry()->GetOrigin(), 0.0001, true), "Check origin ok");

    mitk::ImagePixelReadAccessor<mitk::PeakImage::PixelType, 4> readAccess(inputData);
    itk::Index<4> idx;
    idx[0] = 5;
    idx[1] = 4;
    idx[2] = 1;
    idx[3] = 0;
    MITK_TEST_CONDITION_REQUIRED(fabs(-0.997147f-readAccess.GetPixelByIndex(idx))<0.0001f, "Check pixel value ok");
  }

  void ShReader()
  {
    mitk::PreferenceListReaderOptionsFunctor functor = mitk::PreferenceListReaderOptionsFunctor({"SH Image"}, {});
    auto inputData = mitk::IOUtil::Load<mitk::ShImage>(GetTestDataFilePath("DiffusionImaging/sh_image_test.nii.gz"), &functor);

    std::string class_name = "ShImage";
    bool ok = true;
    if (class_name.compare(inputData->GetNameOfClass())!=0)
      ok = false;
    MITK_TEST_CONDITION_REQUIRED(ok, "Check class name = ShImage");

    mitk::Vector3D sp;
    sp[0] = 1.23;
    sp[1] = 2.001;
    sp[2] = 1.409;
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(sp, inputData->GetGeometry()->GetSpacing(), 0.0001, true), "Check spacing ok");

    mitk::Point3D o;
    o[0] = 0.615;
    o[1] = 1.0005;
    o[2] = 0.7045;
    MITK_TEST_CONDITION_REQUIRED(mitk::Equal(o, inputData->GetGeometry()->GetOrigin(), 0.0001, true), "Check origin ok");

    mitk::ImagePixelReadAccessor<itk::Vector<float, 15>, 3> readAccess(inputData);
    itk::Index<3> idx;
    idx[0] = 5;
    idx[1] = 4;
    idx[2] = 1;
    itk::Vector<float, 15> pixel = readAccess.GetPixelByIndex(idx);
    MITK_TEST_CONDITION_REQUIRED(fabs(0.282095f-pixel[0])<0.000001f, "Check SH coeff value 0 ok");
    MITK_TEST_CONDITION_REQUIRED(fabs(-0.00900571f-pixel[2])<0.000001f, "Check SH coeff value 2 ok");
    MITK_TEST_CONDITION_REQUIRED(fabs(0.0513185f-pixel[6])<0.000001f, "Check SH coeff value 6 ok");
  }

};

MITK_TEST_SUITE_REGISTRATION(mitkPeakShImageReader)
