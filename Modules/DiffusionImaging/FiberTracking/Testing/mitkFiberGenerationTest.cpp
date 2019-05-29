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
#include <mitkPlanarEllipse.h>
#include <itkFibersFromPlanarFiguresFilter.h>
#include <omp.h>

/**Documentation
 *  Test if fiber transfortaiom methods work correctly
 */
int mitkFiberGenerationTest(int argc, char* argv[])
{
    omp_set_num_threads(1);
    MITK_TEST_BEGIN("mitkFiberGenerationTest");

    MITK_TEST_CONDITION_REQUIRED(argc==6,"check for input data")

            try{

        mitk::PlanarEllipse::Pointer pf1 = mitk::IOUtil::Load<mitk::PlanarEllipse>(argv[1]);
        mitk::PlanarEllipse::Pointer pf2 = mitk::IOUtil::Load<mitk::PlanarEllipse>(argv[2]);
        mitk::PlanarEllipse::Pointer pf3 = mitk::IOUtil::Load<mitk::PlanarEllipse>(argv[3]);
        mitk::FiberBundle::Pointer uniform = mitk::IOUtil::Load<mitk::FiberBundle>(argv[4]);
        mitk::FiberBundle::Pointer gaussian = mitk::IOUtil::Load<mitk::FiberBundle>(argv[5]);

        FiberGenerationParameters parameters;
        std::vector< mitk::PlanarEllipse::Pointer > fid; fid.push_back(pf1); fid.push_back(pf2); fid.push_back(pf3);
        std::vector< unsigned int > flip; flip.push_back(0); flip.push_back(0); flip.push_back(0);
        parameters.m_Fiducials.push_back(fid); parameters.m_FlipList.push_back(flip);
        parameters.m_Density = 50;
        parameters.m_Tension = 0;
        parameters.m_Continuity = 0;
        parameters.m_Bias = 0;
        parameters.m_Sampling = 1;
        parameters.m_Variance = 0.1;

        // check uniform fiber distribution
        {
            itk::FibersFromPlanarFiguresFilter::Pointer filter = itk::FibersFromPlanarFiguresFilter::New();
            parameters.m_Distribution = FiberGenerationParameters::DISTRIBUTE_UNIFORM;
            filter->SetParameters(parameters);
            filter->SetFixSeed(true);
            filter->Update();
            std::vector< mitk::FiberBundle::Pointer > fiberBundles = filter->GetFiberBundles();
            MITK_TEST_CONDITION_REQUIRED(uniform->Equals(fiberBundles.at(0)),"check uniform bundle")
        }

        // check gaussian fiber distribution
        {
            itk::FibersFromPlanarFiguresFilter::Pointer filter = itk::FibersFromPlanarFiguresFilter::New();
            parameters.m_Distribution = FiberGenerationParameters::DISTRIBUTE_GAUSSIAN;
            filter->SetParameters(parameters);
            filter->SetFixSeed(true);
            filter->Update();
            std::vector< mitk::FiberBundle::Pointer > fiberBundles = filter->GetFiberBundles();
            MITK_TEST_CONDITION_REQUIRED(gaussian->Equals(fiberBundles.at(0)),"check gaussian bundle")
        }
    }
    catch(...) {
        return EXIT_FAILURE;
    }

    // always end with this!
    MITK_TEST_END();
}
