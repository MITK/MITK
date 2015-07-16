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
#include <mitkFiberBundle.h>
#include <mitkPlanarFigure.h>
#include <mitkPlanarFigureComposite.h>
#include <mitkImageCast.h>
#include <mitkStandaloneDataStorage.h>
#include <vtkDebugLeaks.h>

/**Documentation
 *  Test if fiber transfortaiom methods work correctly
 */
int mitkFiberExtractionTest(int argc, char* argv[])
{
    MITK_TEST_BEGIN("mitkFiberExtractionTest");

    /// \todo Fix VTK memory leaks. Bug 18097.
    vtkDebugLeaks::SetExitError(0);

    MITK_INFO << "argc: " << argc;
    MITK_TEST_CONDITION_REQUIRED(argc==13,"check for input data");

            try{
        mitk::FiberBundle::Pointer groundTruthFibs = dynamic_cast<mitk::FiberBundle*>( mitk::IOUtil::Load(argv[1]).front().GetPointer() );
        mitk::FiberBundle::Pointer testFibs = dynamic_cast<mitk::FiberBundle*>( mitk::IOUtil::Load(argv[2]).front().GetPointer() );

        // test planar figure based extraction
        mitk::DataNode::Pointer pf1 = mitk::IOUtil::LoadDataNode(argv[3]);
        mitk::DataNode::Pointer pf2 = mitk::IOUtil::LoadDataNode(argv[4]);
        mitk::DataNode::Pointer pf3 = mitk::IOUtil::LoadDataNode(argv[5]);

        mitk::StandaloneDataStorage::Pointer storage = mitk::StandaloneDataStorage::New();

        mitk::PlanarFigureComposite::Pointer pfc2 = mitk::PlanarFigureComposite::New();
        pfc2->setOperationType(mitk::PlanarFigureComposite::OR);
        mitk::DataNode::Pointer pfcNode2 = mitk::DataNode::New();
        pfcNode2->SetData(pfc2);
        mitk::DataStorage::SetOfObjects::Pointer set2 = mitk::DataStorage::SetOfObjects::New();
        set2->push_back(pfcNode2);

        mitk::PlanarFigureComposite::Pointer pfc1 = mitk::PlanarFigureComposite::New();
        pfc1->setOperationType(mitk::PlanarFigureComposite::AND);
        mitk::DataNode::Pointer pfcNode1 = mitk::DataNode::New();
        pfcNode1->SetData(pfc1);
        mitk::DataStorage::SetOfObjects::Pointer set1 = mitk::DataStorage::SetOfObjects::New();
        set1->push_back(pfcNode1);

        storage->Add(pfcNode2);
        storage->Add(pf1, set2);
        storage->Add(pfcNode1, set2);
        storage->Add(pf2, set1);
        storage->Add(pf3, set1);

        MITK_INFO << "TEST1";
        mitk::FiberBundle::Pointer extractedFibs = groundTruthFibs->ExtractFiberSubset(pfcNode2, storage);
        MITK_INFO << "TEST2";
        MITK_TEST_CONDITION_REQUIRED(extractedFibs->Equals(testFibs),"check planar figure extraction");

        MITK_INFO << "TEST3";
        // test subtraction and addition
        mitk::FiberBundle::Pointer notExtractedFibs = groundTruthFibs->SubtractBundle(extractedFibs);

        MITK_INFO << argv[11];
        testFibs = dynamic_cast<mitk::FiberBundle*>(mitk::IOUtil::Load(argv[11]).front().GetPointer());
        MITK_TEST_CONDITION_REQUIRED(notExtractedFibs->Equals(testFibs),"check bundle subtraction");

        mitk::FiberBundle::Pointer joinded = extractedFibs->AddBundle(notExtractedFibs);
        testFibs = dynamic_cast<mitk::FiberBundle*>(mitk::IOUtil::Load(argv[12]).front().GetPointer());
        MITK_TEST_CONDITION_REQUIRED(joinded->Equals(testFibs),"check bundle addition");

        // test binary image based extraction
        mitk::Image::Pointer mitkRoiImage = dynamic_cast<mitk::Image*>(mitk::IOUtil::Load(argv[6]).front().GetPointer());
        typedef itk::Image< unsigned char, 3 >    itkUCharImageType;
        itkUCharImageType::Pointer itkRoiImage = itkUCharImageType::New();
        mitk::CastToItkImage(mitkRoiImage, itkRoiImage);

//        mitk::FiberBundle::Pointer inside = groundTruthFibs->RemoveFibersOutside(itkRoiImage, false);
//        mitk::IOUtil::SaveBaseData(inside, mitk::IOUtil::GetTempPath()+"inside.fib");
//        mitk::FiberBundle::Pointer outside = groundTruthFibs->RemoveFibersOutside(itkRoiImage, true);
//        mitk::IOUtil::SaveBaseData(outside, mitk::IOUtil::GetTempPath()+"outside.fib");

        mitk::FiberBundle::Pointer passing = groundTruthFibs->ExtractFiberSubset(itkRoiImage, true);
//        mitk::IOUtil::SaveBaseData(passing, mitk::IOUtil::GetTempPath()+"passing.fib");
        mitk::FiberBundle::Pointer ending = groundTruthFibs->ExtractFiberSubset(itkRoiImage, false);
//        mitk::IOUtil::SaveBaseData(ending, mitk::IOUtil::GetTempPath()+"ending.fib");

//        testFibs = dynamic_cast<mitk::FiberBundle*>(mitk::IOUtil::LoadDataNode(argv[7])->GetData());
//        MITK_TEST_CONDITION_REQUIRED(inside->Equals(testFibs),"check inside mask extraction")

//        testFibs = dynamic_cast<mitk::FiberBundle*>(mitk::IOUtil::LoadDataNode(argv[8])->GetData());
//        MITK_TEST_CONDITION_REQUIRED(outside->Equals(testFibs),"check outside mask extraction")

        testFibs = dynamic_cast<mitk::FiberBundle*>(mitk::IOUtil::Load(argv[9]).front().GetPointer());
        MITK_TEST_CONDITION_REQUIRED(passing->Equals(testFibs),"check passing mask extraction");

        testFibs = dynamic_cast<mitk::FiberBundle*>(mitk::IOUtil::Load(argv[10]).front().GetPointer());
        MITK_TEST_CONDITION_REQUIRED(ending->Equals(testFibs),"check ending in mask extraction");
    }
    catch(...) {
        return EXIT_FAILURE;
    }

    // always end with this!
    MITK_TEST_END();
}
