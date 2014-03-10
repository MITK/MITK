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

#include <mitkToFTestingCommon.h>
#include <mitkTestingMacros.h>
#include <mitkSurface.h>
#include <mitkAICPRegistration.h>
#include <mitkCovarianceMatricesCalculator.h>
#include <mitkSurfaceToPointSetFilter.h>
#include <mitkRegistrationEvaluation.h>
#include <mitkPointSetReader.h>
#include <vtkTimerLog.h>
#include <mitkIOUtil.h>
#include <mitkAnisotropicRegistrationTestUtil.h>
#include <vtkCleanPolyData.h>
#include <mitkVector.h>


#include <mitkCovarianceMatrixCalculator.h>

typedef itk::Matrix<double,3,3> CovarianceMatrix;

static bool CompareCovarianceMatrices( const std::vector<CovarianceMatrix>& ref,
                                       const std::vector<CovarianceMatrix>& result )
{
  for ( unsigned int i = 0; i < ref.size(); ++i )
  {
    if ( ! mitk::MatrixEqualElementWise( ref[i], result[i],0.0001 ) )
    {
      MITK_ERROR <<"IDX: " << i << "\n"<<  ref[i] << " != " << result[i];
      return false;
    }
  }
  return true;
}

int mitkCovarianceMatrixCalculatorTest( int /*argc*/, char* args[])
{
  MITK_TEST_BEGIN("CovarianceMatrixCalculatorTest");

  std::string fixedSurfaceFile = args[1];
  MITK_INFO << fixedSurfaceFile;

  mitk::Surface::Pointer fixedSurface = mitk::IOUtil::LoadSurface(fixedSurfaceFile);
  //generate input point sets from the surfaces
  mitk::SurfaceToPointSetFilter::Pointer surfaceToPointSetFilter = mitk::SurfaceToPointSetFilter::New();
  surfaceToPointSetFilter->SetInput(fixedSurface);
  surfaceToPointSetFilter->Update();
  mitk::PointSet::Pointer fixedPointSet = surfaceToPointSetFilter->GetOutput();

  //calculate covariance matrices reference impl
  MITK_INFO << "covariance matrices calculation started";
  mitk::CovarianceMatricesCalculator::Pointer covarianceMatricesCalculator = mitk::CovarianceMatricesCalculator::New();
  covarianceMatricesCalculator->CalculateWeightMatricesWithVoronoiArea(fixedSurface,1.0,fixedPointSet,1.0,NULL,mitk::CovarianceMatricesCalculator::PCAWithoutVoronoi,false);
  std::vector< itk::Matrix<double,3,3> > covarianceMatrices = covarianceMatricesCalculator->GetOutput();

//  vtkSmartPointer<vtkCleanPolyData> cleaner = vtkSmartPointer<vtkCleanPolyData>::New();
//  // config
//  cleaner->PieceInvariantOff();
//  cleaner->ConvertLinesToPointsOff();
//  cleaner->ConvertPolysToLinesOff();
//  cleaner->ConvertStripsToPolysOff();
//  cleaner->PointMergingOn();
//
//  cleaner->SetInput(fixedSurface->GetVtkPolyData());
//  cleaner->Update();
//
//  fixedSurface->SetVtkPolyData(cleaner->GetOutput());

  // run the calculator
  mitk::CovarianceMatrixCalculator::Pointer matrixCalculator = mitk::CovarianceMatrixCalculator::New();
  matrixCalculator->SetInputSurface(fixedSurface);
  matrixCalculator->ComputeCovarianceMatrices();
  std::vector< itk::Matrix<double,3,3> > myMatrix = matrixCalculator->GetCovarianceMatrices();

  MITK_TEST_CONDITION_REQUIRED(CompareCovarianceMatrices(covarianceMatrices,myMatrix), "Testing Covariance matrix computation with the CM_PCA method");

  MITK_TEST_CONDITION_REQUIRED( mitk::Equal(matrixCalculator->GetMeanVariance(), covarianceMatricesCalculator->GetMeanVariance() ),"Testing the computed");

  MITK_TEST_END();
}
