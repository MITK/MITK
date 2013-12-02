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
#include <mitkFiberTrackingObjectFactory.h>
#include <mitkDiffusionCoreObjectFactory.h>
#include <mitkFiberBundleX.h>
#include <mitkPlanarFigure.h>
#include <mitkPlanarFigureComposite.h>

/**Documentation
 *  Test if fiber transfortaiom methods work correctly
 */
int mitkFiberExtractionTest(int argc, char* argv[])
{
    MITK_TEST_BEGIN("mitkFiberExtractionTest");

    MITK_TEST_CONDITION_REQUIRED(argc==6,"check for input data")

            try{
        RegisterFiberTrackingObjectFactory();

        mitk::FiberBundleX::Pointer groundTruthFibs = dynamic_cast<mitk::FiberBundleX*>(mitk::IOUtil::LoadDataNode(argv[1])->GetData());
        mitk::FiberBundleX::Pointer testFibs = dynamic_cast<mitk::FiberBundleX*>(mitk::IOUtil::LoadDataNode(argv[2])->GetData());
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

        groundTruthFibs = groundTruthFibs->ExtractFiberSubset(pfc2);

        MITK_TEST_CONDITION_REQUIRED(groundTruthFibs->Equals(testFibs),"check extraction")
    }
    catch(...) {
        return EXIT_FAILURE;
    }

    // always end with this!
    MITK_TEST_END();
}
