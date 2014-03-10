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
#include <mitkSurface.h>
#include <vtkTimerLog.h>
#include <mitkIOUtil.h>
#include <vtkCleanPolyData.h>

#include "mitkAnisotropicIterativeClosestPointRegistration.h"
#include "mitkCovarianceMatrixCalculator.h"

/**
 * Test to verify the results of the AICP registration achieved for publications.
 * The test can be used for both, the standard AICP and the trimmed variant.

 * Results for the standard variant are published in:
   Maier-Hein et. al. "Convergent Iterative Closest-Point Algorithm to Accomodate
   Anisotropic and Inhomogenous Localization Error".
 * Results for the trimmed variant are published in:
   Kilgus et. al. "Registration of Partially Overlapping Surfaces for Range Image based
   Augmented Reality on Mobile Devices".
 */

int mitkAnisotropicIterativeClosestPointRegistrationTest( int argc, char* args[])
{
    MITK_TEST_BEGIN("mitkAnisotropicIterativeClosestPointRegistrationTest");

    MITK_TEST_CONDITION_REQUIRED( argc >= 8, "Testing if all arguments are set.");
    //load input surfaces
    std::string fixedSurfaceFile = args[1];
    std::string movingSurfaceFile = args[2];

    std::string fixedTargetsFile = args[3];
    std::string movingTargetsFile = args[4];

    double trimmedPart = atof(args[5]);
    double freLiterature = atof(args[6]);
    double treLiterature = atof(args[7]);

    mitk::PointSet::Pointer movingTargets = mitk::IOUtil::LoadPointSet(movingTargetsFile);
    mitk::PointSet::Pointer fixedTargets = mitk::IOUtil::LoadPointSet(fixedTargetsFile);

    mitk::Surface::Pointer fixedSurface = mitk::IOUtil::LoadSurface(fixedSurfaceFile);
    mitk::Surface::Pointer movingSurface = mitk::IOUtil::LoadSurface(movingSurfaceFile);

    // compute cov matrices for the moving (.e.g. ToF Surface)
    mitk::CovarianceMatrixCalculator::Pointer matrixCalculator = mitk::CovarianceMatrixCalculator::New();
    matrixCalculator->SetInputSurface(movingSurface);
    matrixCalculator->ComputeCovarianceMatrices();
    std::vector< itk::Matrix<double,3,3> > covarianceMatricesMoving = matrixCalculator->GetCovarianceMatrices();
    double FRENormalizationsigmaMoving = matrixCalculator->GetMeanVariance();
    MITK_INFO << "1: " << covarianceMatricesMoving.size() << " , 2: " << movingSurface->GetVtkPolyData()->GetNumberOfPoints();
    MITK_TEST_CONDITION_REQUIRED( (int)covarianceMatricesMoving.size() == movingSurface->GetVtkPolyData()->GetNumberOfPoints(), "Testing if a covariance matrix was generated for every point." );

    // compute the cov matrices for the fixed surface ( ct surface)
    matrixCalculator->SetInputSurface(fixedSurface);
    matrixCalculator->ComputeCovarianceMatrices();
    std::vector< itk::Matrix<double,3,3> > covarianceMatricesFixed= matrixCalculator->GetCovarianceMatrices();
    double FRENormalizationsigmaFixed = matrixCalculator->GetMeanVariance();
    MITK_INFO << "1: " << covarianceMatricesFixed.size() << " , 2: " << fixedSurface->GetVtkPolyData()->GetNumberOfPoints();
    MITK_TEST_CONDITION_REQUIRED( (int)covarianceMatricesFixed.size() == fixedSurface->GetVtkPolyData()->GetNumberOfPoints(), "Testing if a covariance matrix was generated for every point." );
    double FRENormalizationFactor = sqrt(FRENormalizationsigmaMoving+FRENormalizationsigmaFixed);

    // run the algorithm
    mitk::AnisotropicIterativeClosestPointRegistration::Pointer aICP =
        mitk::AnisotropicIterativeClosestPointRegistration::New();

  aICP->SetMovingSurface(movingSurface);
  aICP->SetFixedSurface(fixedSurface);
  aICP->SetCovarianceMatricesMovingSurface(covarianceMatricesMoving);
  aICP->SetCovarianceMatricesFixedSurface(covarianceMatricesFixed);
  aICP->SetFRENormalizationFactor(FRENormalizationFactor);

  //  aICP->SetMovingPointSet(movingPointSet);
  //  aICP->SetFixedPointSet(fixedPointSet);
  //  aICP->SetCovarianceMatrices(covarianceMatricesMoving,covarianceMatricesFixed);
  //  aICP->SetMovingSurface(movingSurface);
  //  aICP->SetFixedSurface(fixedSurface);
  //  aICP->SetFRENormalizationFactor(FRENormalizationFactor);
  //  aICP->SetLimitIterationsAnisotropicPointRegister(200.0);
  //  aICP->SetMaxNumberOfIterations(200);
  aICP->SetThreshold(0.0000000010);
  //  aICP->SetCandiateRadius(30.0);

  if(trimmedPart != 0.0)
  {
     aICP->SetTrimmFactor(1.0 - trimmedPart);
     MITK_INFO << "Algorithm is using trimmed variant.";
  }

  MITK_INFO << "Before do register";
  aICP->Update();

  MITK_INFO<<"After do register";
  MITK_INFO << "FRE:"  << aICP->GetFRE() << ", ref: " << freLiterature;
  //  //value 3.15857 obtained from: \e130-projekte\NeedleNavigation\AnisotropicICP\EvaluationTPAMI\Experimente_Maerz2011/Ergebnisse.xlsx
  MITK_TEST_CONDITION( mitk::Equal(aICP->GetFRE(), freLiterature,0.001), "Testing if FRE equals results from literature.");

  double tre = -1;
 // tre = mitk::AnisotropicRegistrationTestUtil::ComputeTRE( movingTargets, fixedTargets, aICP->GetRotation(), aICP->GetTranslation());
 // MITK_INFO << "tre: " << tre << ", ref :" << treLiterature;
 // //  //value 0.0784612 obtained from: \e130-projekte\NeedleNavigation\AnisotropicICP\EvaluationTPAMI\Experimente_Maerz2011/Ergebnisse.xlsx
 // MITK_TEST_CONDITION( mitk::Equal(tre, treLiterature,0.001), "Testing if TRE equals the results from literature.");

  MITK_TEST_END();
}
