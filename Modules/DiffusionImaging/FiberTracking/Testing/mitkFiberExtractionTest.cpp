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

#include <mitkTestingMacros.h>
#include <mitkIOUtil.h>
#include <mitkFiberBundleX.h>
#include <mitkPlanarFigure.h>
#include <mitkPlanarFigureComposite.h>
#include <mitkImageCast.h>

/**Documentation
 *  Test if fiber transfortaiom methods work correctly
 */
int mitkFiberExtractionTest(int argc, char* argv[])
{
    MITK_TEST_BEGIN("mitkFiberExtractionTest");

    MITK_INFO << "argc: " << argc;
    MITK_TEST_CONDITION_REQUIRED(argc==13,"check for input data")

            try{
        mitk::FiberBundleX::Pointer groundTruthFibs = dynamic_cast<mitk::FiberBundleX*>(mitk::IOUtil::LoadDataNode(argv[1])->GetData());
        mitk::FiberBundleX::Pointer testFibs = dynamic_cast<mitk::FiberBundleX*>(mitk::IOUtil::LoadDataNode(argv[2])->GetData());

        // test planar figure based extraction
        mitk::PlanarFigure::Pointer pf1 = dynamic_cast<mitk::PlanarFigure*>(mitk::IOUtil::LoadDataNode(argv[3])->GetData());
        mitk::PlanarFigure::Pointer pf2 = dynamic_cast<mitk::PlanarFigure*>(mitk::IOUtil::LoadDataNode(argv[4])->GetData());
        mitk::PlanarFigure::Pointer pf3 = dynamic_cast<mitk::PlanarFigure*>(mitk::IOUtil::LoadDataNode(argv[5])->GetData());
        mitk::PlanarFigureComposite::Pointer pfc1 = mitk::PlanarFigureComposite::New();
        pfc1->setOperationType(mitk::PFCOMPOSITION_AND_OPERATION);
        pfc1->addPlanarFigure(pf2);
        pfc1->addPlanarFigure(pf3);
        mitk::PlanarFigureComposite::Pointer pfc2 = mitk::PlanarFigureComposite::New();
        pfc2->setOperationType(mitk::PFCOMPOSITION_OR_OPERATION);
        pfc2->addPlanarFigure(pf1);
        pfc2->addPlanarFigure(dynamic_cast<mitk::PlanarFigure*>(pfc1.GetPointer()));
        mitk::FiberBundleX::Pointer extractedFibs = groundTruthFibs->ExtractFiberSubset(pfc2);
        MITK_TEST_CONDITION_REQUIRED(extractedFibs->Equals(testFibs),"check planar figure extraction")

        // test subtraction and addition
        mitk::FiberBundleX::Pointer notExtractedFibs = groundTruthFibs->SubtractBundle(extractedFibs);

        MITK_INFO << argv[11];
        testFibs = dynamic_cast<mitk::FiberBundleX*>(mitk::IOUtil::LoadDataNode(argv[11])->GetData());
        MITK_TEST_CONDITION_REQUIRED(notExtractedFibs->Equals(testFibs),"check bundle subtraction")

        mitk::FiberBundleX::Pointer joinded = extractedFibs->AddBundle(notExtractedFibs);
        testFibs = dynamic_cast<mitk::FiberBundleX*>(mitk::IOUtil::LoadDataNode(argv[12])->GetData());
        MITK_TEST_CONDITION_REQUIRED(joinded->Equals(testFibs),"check bundle addition")

        // test binary image based extraction
        mitk::Image::Pointer mitkRoiImage = dynamic_cast<mitk::Image*>(mitk::IOUtil::LoadDataNode(argv[6])->GetData());
        typedef itk::Image< unsigned char, 3 >    itkUCharImageType;
        itkUCharImageType::Pointer itkRoiImage = itkUCharImageType::New();
        mitk::CastToItkImage(mitkRoiImage, itkRoiImage);

        mitk::FiberBundleX::Pointer inside = groundTruthFibs->RemoveFibersOutside(itkRoiImage, false);
        mitk::FiberBundleX::Pointer outside = groundTruthFibs->RemoveFibersOutside(itkRoiImage, true);
        mitk::FiberBundleX::Pointer passing = groundTruthFibs->ExtractFiberSubset(itkRoiImage, true);
        mitk::FiberBundleX::Pointer ending = groundTruthFibs->ExtractFiberSubset(itkRoiImage, false);

        testFibs = dynamic_cast<mitk::FiberBundleX*>(mitk::IOUtil::LoadDataNode(argv[7])->GetData());
        MITK_TEST_CONDITION_REQUIRED(inside->Equals(testFibs),"check inside mask extraction")

        testFibs = dynamic_cast<mitk::FiberBundleX*>(mitk::IOUtil::LoadDataNode(argv[8])->GetData());
        MITK_TEST_CONDITION_REQUIRED(outside->Equals(testFibs),"check outside mask extraction")

        testFibs = dynamic_cast<mitk::FiberBundleX*>(mitk::IOUtil::LoadDataNode(argv[9])->GetData());
        MITK_TEST_CONDITION_REQUIRED(passing->Equals(testFibs),"check passing mask extraction")

        testFibs = dynamic_cast<mitk::FiberBundleX*>(mitk::IOUtil::LoadDataNode(argv[10])->GetData());
        MITK_TEST_CONDITION_REQUIRED(ending->Equals(testFibs),"check ending in mask extraction")
    }
    catch(...) {
        return EXIT_FAILURE;
    }

    // always end with this!
    MITK_TEST_END();
}
