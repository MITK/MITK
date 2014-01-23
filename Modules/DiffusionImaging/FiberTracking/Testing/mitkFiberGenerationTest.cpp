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
#include <mitkPlanarEllipse.h>
#include <itkFibersFromPlanarFiguresFilter.h>

/**Documentation
 *  Test if fiber transfortaiom methods work correctly
 */
int mitkFiberGenerationTest(int argc, char* argv[])
{
    MITK_TEST_BEGIN("mitkFiberGenerationTest");

    MITK_TEST_CONDITION_REQUIRED(argc==6,"check for input data")

            try{

        mitk::PlanarEllipse::Pointer pf1 = dynamic_cast<mitk::PlanarEllipse*>(mitk::IOUtil::LoadDataNode(argv[1])->GetData());
        mitk::PlanarEllipse::Pointer pf2 = dynamic_cast<mitk::PlanarEllipse*>(mitk::IOUtil::LoadDataNode(argv[2])->GetData());
        mitk::PlanarEllipse::Pointer pf3 = dynamic_cast<mitk::PlanarEllipse*>(mitk::IOUtil::LoadDataNode(argv[3])->GetData());
        mitk::FiberBundleX::Pointer uniform = dynamic_cast<mitk::FiberBundleX*>(mitk::IOUtil::LoadDataNode(argv[4])->GetData());
        mitk::FiberBundleX::Pointer gaussian = dynamic_cast<mitk::FiberBundleX*>(mitk::IOUtil::LoadDataNode(argv[5])->GetData());


        vector< mitk::PlanarEllipse::Pointer > fid; fid.push_back(pf1); fid.push_back(pf2); fid.push_back(pf3);
        vector< unsigned int > flip; flip.push_back(0); flip.push_back(0); flip.push_back(0);
        vector< vector< mitk::PlanarEllipse::Pointer > > fiducials;
        vector< vector< unsigned int > > fliplist;
        fiducials.push_back(fid); fliplist.push_back(flip);

        // check uniform fiber distribution
        {
            itk::FibersFromPlanarFiguresFilter::Pointer filter = itk::FibersFromPlanarFiguresFilter::New();
            filter->SetFiducials(fiducials);
            filter->SetFlipList(fliplist);
            filter->SetFiberDistribution(itk::FibersFromPlanarFiguresFilter::DISTRIBUTE_UNIFORM);
            filter->SetDensity(50);
            filter->SetTension(0);
            filter->SetContinuity(0);
            filter->SetBias(0);
            filter->SetFiberSampling(1);
            filter->Update();
            vector< mitk::FiberBundleX::Pointer > fiberBundles = filter->GetFiberBundles();
            MITK_TEST_CONDITION_REQUIRED(uniform->Equals(fiberBundles.at(0)),"check uniform bundle")
        }

        // check gaussian fiber distribution
        {
            itk::FibersFromPlanarFiguresFilter::Pointer filter = itk::FibersFromPlanarFiguresFilter::New();
            filter->SetFiducials(fiducials);
            filter->SetFlipList(fliplist);
            filter->SetFiberDistribution(itk::FibersFromPlanarFiguresFilter::DISTRIBUTE_GAUSSIAN);
            filter->SetVariance(0.1);
            filter->SetDensity(50);
            filter->SetTension(0);
            filter->SetContinuity(0);
            filter->SetBias(0);
            filter->SetFiberSampling(1);
            filter->Update();
            vector< mitk::FiberBundleX::Pointer > fiberBundles = filter->GetFiberBundles();
            MITK_TEST_CONDITION_REQUIRED(gaussian->Equals(fiberBundles.at(0)),"check gaussian bundle")
        }
    }
    catch(...) {
        return EXIT_FAILURE;
    }

    // always end with this!
    MITK_TEST_END();
}
