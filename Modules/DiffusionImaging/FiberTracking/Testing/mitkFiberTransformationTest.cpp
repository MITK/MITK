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

/**Documentation
 *  Test if fiber transfortaiom methods work correctly
 */
int mitkFiberTransformationTest(int argc, char* argv[])
{
    MITK_TEST_BEGIN("mitkFiberTransformationTest");

    MITK_TEST_CONDITION_REQUIRED(argc==3,"check for input data")

            try{
        mitk::FiberBundleX::Pointer groundTruthFibs = dynamic_cast<mitk::FiberBundleX*>(mitk::IOUtil::LoadDataNode(argv[1])->GetData());
        mitk::FiberBundleX::Pointer transformedFibs = dynamic_cast<mitk::FiberBundleX*>(mitk::IOUtil::LoadDataNode(argv[2])->GetData());

        groundTruthFibs->RotateAroundAxis(90, 45, 10);
        groundTruthFibs->TranslateFibers(2, 3, 5);
        groundTruthFibs->ScaleFibers(1, 0.1, 1.3);
        groundTruthFibs->RemoveLongFibers(150);
        groundTruthFibs->RemoveShortFibers(20);
        groundTruthFibs->ResampleSpline(1.0);
        groundTruthFibs->ApplyCurvatureThreshold(3.0, true);
        groundTruthFibs->MirrorFibers(0);
        groundTruthFibs->MirrorFibers(1);
        groundTruthFibs->MirrorFibers(2);

        MITK_TEST_CONDITION_REQUIRED(groundTruthFibs->Equals(transformedFibs),"check transformation")
    }
    catch(...) {
        return EXIT_FAILURE;
    }

    // always end with this!
    MITK_TEST_END();
}
