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

#include "mitkTestingMacros.h"
#include <mitkFiberBundle.h>
#include <mitkBaseData.h>
#include <itksys/SystemTools.hxx>
#include <mitkTestingConfig.h>
#include <mitkIOUtil.h>
#include <itkFiberCurvatureFilter.h>

#include "mitkTestFixture.h"

class mitkFiberProcessingTestSuite : public mitk::TestFixture
{

    CPPUNIT_TEST_SUITE(mitkFiberProcessingTestSuite);
    MITK_TEST(Test1);
    MITK_TEST(Test2);
    MITK_TEST(Test3);
    MITK_TEST(Test4);
//    MITK_TEST(Test5); - disabled as a temporary fix for bug 19770, should be fixed properly
    MITK_TEST(Test6);
    MITK_TEST(Test7);
    MITK_TEST(Test8);
    MITK_TEST(Test9);
    MITK_TEST(Test10);
    MITK_TEST(Test11);
    MITK_TEST(Test12);
    MITK_TEST(Test13);
    MITK_TEST(Test14);
    MITK_TEST(Test15);
    MITK_TEST(Test16);
    MITK_TEST(Test17);
    CPPUNIT_TEST_SUITE_END();

    typedef itk::Image<unsigned char, 3> ItkUcharImgType;

private:

    /** Members used inside the different (sub-)tests. All members are initialized via setUp().*/
    mitk::FiberBundle::Pointer  original;
    ItkUcharImgType::Pointer    mask;

public:

    void setUp() override
    {
        original = NULL;
        original = dynamic_cast<mitk::FiberBundle*>(mitk::IOUtil::Load(GetTestDataFilePath("DiffusionImaging/FiberProcessing/original.fib")).front().GetPointer());

        mitk::Image::Pointer img = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadImage(GetTestDataFilePath("DiffusionImaging/FiberProcessing/MASK.nrrd")).GetPointer());
        mask = ItkUcharImgType::New();
        mitk::CastToItkImage(img, mask);
    }

    void tearDown() override
    {
        original = NULL;
    }

    void Test1()
    {
        MITK_INFO << "TEST 1: Remove by direction";

        mitk::FiberBundle::Pointer fib = original->GetDeepCopy();
        vnl_vector_fixed<double,3> dir;
        dir[0] = 0;
        dir[1] = 1;
        dir[2] = 0;
        fib->RemoveDir(dir,cos(5.0*M_PI/180));

        mitk::FiberBundle::Pointer ref = dynamic_cast<mitk::FiberBundle*>(mitk::IOUtil::Load(GetTestDataFilePath("DiffusionImaging/FiberProcessing/remove_direction.fib")).front().GetPointer());

        CPPUNIT_ASSERT_MESSAGE("Should be equal", ref->Equals(fib));
    }

    void Test2()
    {
        MITK_INFO << "TEST 2: Remove by length";

        mitk::FiberBundle::Pointer fib = original->GetDeepCopy();
        fib->RemoveShortFibers(30);
        fib->RemoveLongFibers(40);

        mitk::FiberBundle::Pointer ref = dynamic_cast<mitk::FiberBundle*>(mitk::IOUtil::Load(GetTestDataFilePath("DiffusionImaging/FiberProcessing/remove_length.fib")).front().GetPointer());

        CPPUNIT_ASSERT_MESSAGE("Should be equal", ref->Equals(fib));
    }

    void Test3()
    {
        MITK_INFO << "TEST 3: Remove by curvature 1";

        mitk::FiberBundle::Pointer fib = original->GetDeepCopy();
        auto filter = itk::FiberCurvatureFilter::New();
        filter->SetInputFiberBundle(fib);
        filter->SetAngularDeviation(30);
        filter->SetDistance(5);
        filter->SetRemoveFibers(false);
        filter->Update();
        mitk::FiberBundle::Pointer ref = dynamic_cast<mitk::FiberBundle*>(mitk::IOUtil::Load(GetTestDataFilePath("DiffusionImaging/FiberProcessing/remove_curvature1.fib")).front().GetPointer());
        CPPUNIT_ASSERT_MESSAGE("Should be equal", ref->Equals(filter->GetOutputFiberBundle()));
    }

    void Test4()
    {
        MITK_INFO << "TEST 4: Remove by curvature 2";

        mitk::FiberBundle::Pointer fib = original->GetDeepCopy();
        auto filter = itk::FiberCurvatureFilter::New();
        filter->SetInputFiberBundle(fib);
        filter->SetAngularDeviation(30);
        filter->SetDistance(5);
        filter->SetRemoveFibers(true);
        filter->Update();
        mitk::FiberBundle::Pointer ref = dynamic_cast<mitk::FiberBundle*>(mitk::IOUtil::Load(GetTestDataFilePath("DiffusionImaging/FiberProcessing/remove_curvature2.fib")).front().GetPointer());
        CPPUNIT_ASSERT_MESSAGE("Should be equal", ref->Equals(filter->GetOutputFiberBundle()));
    }

    void Test5()
    {
        MITK_INFO << "TEST 5: Remove outside mask";

        mitk::FiberBundle::Pointer fib = original->GetDeepCopy();
        fib = fib->RemoveFibersOutside(mask);
        mitk::FiberBundle::Pointer ref = dynamic_cast<mitk::FiberBundle*>(mitk::IOUtil::Load(GetTestDataFilePath("DiffusionImaging/FiberProcessing/remove_outside.fib")).front().GetPointer());
        CPPUNIT_ASSERT_MESSAGE("Should be equal", ref->Equals(fib));
    }

    void Test6()
    {
        MITK_INFO << "TEST 6: Remove inside mask";

        mitk::FiberBundle::Pointer fib = original->GetDeepCopy();
        fib = fib->RemoveFibersOutside(mask, true);
        mitk::FiberBundle::Pointer ref = dynamic_cast<mitk::FiberBundle*>(mitk::IOUtil::Load(GetTestDataFilePath("DiffusionImaging/FiberProcessing/remove_inside.fib")).front().GetPointer());
        CPPUNIT_ASSERT_MESSAGE("Should be equal", ref->Equals(fib));
    }

    void Test7()
    {
        MITK_INFO << "TEST 7: Modify resample";

        mitk::FiberBundle::Pointer fib = original->GetDeepCopy();
        fib->ResampleSpline(5);
        mitk::FiberBundle::Pointer ref = dynamic_cast<mitk::FiberBundle*>(mitk::IOUtil::Load(GetTestDataFilePath("DiffusionImaging/FiberProcessing/modify_resample.fib")).front().GetPointer());
        CPPUNIT_ASSERT_MESSAGE("Should be equal", ref->Equals(fib));
    }

    void Test8()
    {
        MITK_INFO << "TEST 8: Modify compress";

        mitk::FiberBundle::Pointer fib = original->GetDeepCopy();
        fib->Compress(0.1);
        mitk::FiberBundle::Pointer ref = dynamic_cast<mitk::FiberBundle*>(mitk::IOUtil::Load(GetTestDataFilePath("DiffusionImaging/FiberProcessing/modify_compress.fib")).front().GetPointer());
        CPPUNIT_ASSERT_MESSAGE("Should be equal", ref->Equals(fib));
    }

    void Test9()
    {
        MITK_INFO << "TEST 9: Modify sagittal mirror";

        mitk::FiberBundle::Pointer fib = original->GetDeepCopy();
        fib->MirrorFibers(0);
        mitk::FiberBundle::Pointer ref = dynamic_cast<mitk::FiberBundle*>(mitk::IOUtil::Load(GetTestDataFilePath("DiffusionImaging/FiberProcessing/modify_sagittal_mirror.fib")).front().GetPointer());
        CPPUNIT_ASSERT_MESSAGE("Should be equal", ref->Equals(fib));
    }

    void Test10()
    {
        MITK_INFO << "TEST 10: Modify coronal mirror";

        mitk::FiberBundle::Pointer fib = original->GetDeepCopy();
        fib->MirrorFibers(1);
        mitk::FiberBundle::Pointer ref = dynamic_cast<mitk::FiberBundle*>(mitk::IOUtil::Load(GetTestDataFilePath("DiffusionImaging/FiberProcessing/modify_coronal_mirror.fib")).front().GetPointer());
        CPPUNIT_ASSERT_MESSAGE("Should be equal", ref->Equals(fib));
    }

    void Test11()
    {
        MITK_INFO << "TEST 11: Modify axial mirror";

        mitk::FiberBundle::Pointer fib = original->GetDeepCopy();
        fib->MirrorFibers(2);
        mitk::FiberBundle::Pointer ref = dynamic_cast<mitk::FiberBundle*>(mitk::IOUtil::Load(GetTestDataFilePath("DiffusionImaging/FiberProcessing/modify_axial_mirror.fib")).front().GetPointer());
        CPPUNIT_ASSERT_MESSAGE("Should be equal", ref->Equals(fib));
    }

    void Test12()
    {
        MITK_INFO << "TEST 12: Weight and join";

        mitk::FiberBundle::Pointer fib = original->GetDeepCopy();
        fib->SetFiberWeights(0.1);

        mitk::FiberBundle::Pointer fib2 = original->GetDeepCopy();
        fib2->SetFiberWeight(3, 0.5);

        fib = fib->AddBundle(fib2);

        mitk::FiberBundle::Pointer ref = dynamic_cast<mitk::FiberBundle*>(mitk::IOUtil::Load(GetTestDataFilePath("DiffusionImaging/FiberProcessing/modify_weighted_joined.fib")).front().GetPointer());

        CPPUNIT_ASSERT_MESSAGE("Number of fibers", ref->GetNumFibers() == fib->GetNumFibers());

        for (int i=0; i<ref->GetNumFibers(); i++)
            CPPUNIT_ASSERT_MESSAGE("Fiber weights", ref->GetFiberWeight(i) == fib->GetFiberWeight(i));
    }

    void Test13()
    {
        MITK_INFO << "TEST 13: Subtract";

        mitk::FiberBundle::Pointer fib = original->GetDeepCopy();

        mitk::FiberBundle::Pointer fib2 = dynamic_cast<mitk::FiberBundle*>(mitk::IOUtil::Load(GetTestDataFilePath("DiffusionImaging/FiberProcessing/remove_length.fib")).front().GetPointer());
        fib = fib->SubtractBundle(fib2);

        mitk::FiberBundle::Pointer ref = dynamic_cast<mitk::FiberBundle*>(mitk::IOUtil::Load(GetTestDataFilePath("DiffusionImaging/FiberProcessing/subtracted.fib")).front().GetPointer());

        CPPUNIT_ASSERT_MESSAGE("Should be equal", ref->Equals(fib));
    }

    void Test14()
    {
        MITK_INFO << "TEST 14: rotate";

        mitk::FiberBundle::Pointer fib = original->GetDeepCopy();
        fib->TransformFibers(1,2,3,0,0,0);

        mitk::FiberBundle::Pointer ref = dynamic_cast<mitk::FiberBundle*>(mitk::IOUtil::Load(GetTestDataFilePath("DiffusionImaging/FiberProcessing/transform_rotate.fib")).front().GetPointer());

        CPPUNIT_ASSERT_MESSAGE("Should be equal", ref->Equals(fib));
    }

    void Test15()
    {
        MITK_INFO << "TEST 15: translate";

        mitk::FiberBundle::Pointer fib = original->GetDeepCopy();
        fib->TransformFibers(0,0,0,1,2,3);

        mitk::FiberBundle::Pointer ref = dynamic_cast<mitk::FiberBundle*>(mitk::IOUtil::Load(GetTestDataFilePath("DiffusionImaging/FiberProcessing/transform_translate.fib")).front().GetPointer());

        CPPUNIT_ASSERT_MESSAGE("Should be equal", ref->Equals(fib));
    }

    void Test16()
    {
        MITK_INFO << "TEST 16: scale 1";

        mitk::FiberBundle::Pointer fib = original->GetDeepCopy();
        fib->ScaleFibers(0.1, 0.2, 0.3);

        mitk::FiberBundle::Pointer ref = dynamic_cast<mitk::FiberBundle*>(mitk::IOUtil::Load(GetTestDataFilePath("DiffusionImaging/FiberProcessing/transform_scale1.fib")).front().GetPointer());

        CPPUNIT_ASSERT_MESSAGE("Should be equal", ref->Equals(fib));
    }

    void Test17()
    {
        MITK_INFO << "TEST 17: scale 2";

        mitk::FiberBundle::Pointer fib = original->GetDeepCopy();
        fib->ScaleFibers(0.1, 0.2, 0.3, false);

        mitk::FiberBundle::Pointer ref = dynamic_cast<mitk::FiberBundle*>(mitk::IOUtil::Load(GetTestDataFilePath("DiffusionImaging/FiberProcessing/transform_scale2.fib")).front().GetPointer());

        CPPUNIT_ASSERT_MESSAGE("Should be equal", ref->Equals(fib));
    }

};

MITK_TEST_SUITE_REGISTRATION(mitkFiberProcessing)
