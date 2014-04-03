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


// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "QmitkSurfaceRegistrationView.h"

// Qt
#include <QMessageBox>

#include <mitkSurface.h>
#include <mitkPointSet.h>
#include <mitkNodePredicateDataType.h>
#include <mitkAnisotropicIterativeClosestPointRegistration.h>
#include <mitkCovarianceMatrixCalculator.h>
#include <mitkAnisotropicRegistrationCommon.h>
#include <vtkPolyData.h>

const std::string QmitkSurfaceRegistrationView::VIEW_ID = "org.mitk.views.a-icpregistration";

void QmitkSurfaceRegistrationView::SetFocus()
{
 // m_Controls.buttonPerformImageProcessing->setFocus();
}

void QmitkSurfaceRegistrationView::CreateQtPartControl( QWidget *parent )
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi( parent );

  connect ( m_Controls.m_EnableTreCalculation,SIGNAL(clicked()),this, SLOT(OnEnableTreCalculation()) );
  connect ( m_Controls.m_RegisterSurfaceButton, SIGNAL(clicked()), this, SLOT(OnRunRegistration()) );

  // init combo boxes
  m_Controls.m_FixedSurfaceComboBox->SetDataStorage(this->GetDataStorage());
  m_Controls.m_FixedSurfaceComboBox->SetPredicate(mitk::NodePredicateDataType::New("Surface"));

  m_Controls.m_MovingSurfaceComboBox->SetDataStorage(this->GetDataStorage());
  m_Controls.m_MovingSurfaceComboBox->SetPredicate(mitk::NodePredicateDataType::New("Surface"));

  m_Controls.m_MovingTargets->SetDataStorage(this->GetDataStorage());
  m_Controls.m_MovingTargets->SetPredicate(mitk::NodePredicateDataType::New("PointSet"));

  m_Controls.m_FixedTargets->SetDataStorage(this->GetDataStorage());
  m_Controls.m_FixedTargets->SetPredicate(mitk::NodePredicateDataType::New("PointSet"));


  // disable target selection
  m_Controls.m_TargetSelectFrame->setEnabled(false);
}


void QmitkSurfaceRegistrationView::OnRunRegistration()
{
  typedef itk::Matrix<double,3,3> Matrix3x3;
  typedef itk::Vector<double,3> TranslationVector;
  typedef std::vector<Matrix3x3> CovarianceMatrixList;

  // anisotropic registration
  mitk::AnisotropicIterativeClosestPointRegistration::Pointer aICP =
      mitk::AnisotropicIterativeClosestPointRegistration::New();

  // covariance matrix calculator
  mitk::CovarianceMatrixCalculator::Pointer matrixCalculator =
                                        mitk::CovarianceMatrixCalculator::New();


  mitk::Surface::Pointer movingSurface = dynamic_cast<mitk::Surface*>(
            m_Controls.m_MovingSurfaceComboBox->GetSelectedNode()->GetData() );

  mitk::Surface::Pointer fixedSurface = dynamic_cast<mitk::Surface*>(
            m_Controls.m_FixedSurfaceComboBox->GetSelectedNode()->GetData() );

  // sanity check
  if ( fixedSurface.IsNull() || movingSurface.IsNull() )
  {
    MITK_ERROR << "Input surface not set";
    return;
  }

  // compute the covariance matrices for the moving surface (X)
  matrixCalculator->SetInputSurface(movingSurface);
  matrixCalculator->ComputeCovarianceMatrices();
  CovarianceMatrixList sigmas_X = matrixCalculator->GetCovarianceMatrices();
  double meanVarX = matrixCalculator->GetMeanVariance();

  // compute the covariance matrices for the fixed surface (Y)
  matrixCalculator->SetInputSurface(fixedSurface);
  matrixCalculator->ComputeCovarianceMatrices();
  CovarianceMatrixList sigmas_Y = matrixCalculator->GetCovarianceMatrices();
  double meanVarY = matrixCalculator->GetMeanVariance();

  // the FRE normalization factor
  double normalizationFactor = sqrt( meanVarX + meanVarY);

  // set up parameters
  aICP->SetMovingSurface(movingSurface);
  aICP->SetFixedSurface(fixedSurface);
  aICP->SetCovarianceMatricesMovingSurface(sigmas_X);
  aICP->SetCovarianceMatricesFixedSurface(sigmas_Y);
  aICP->SetFRENormalizationFactor(normalizationFactor);

  // run the algorithm
  aICP->Update();

  Matrix3x3 rotation = aICP->GetRotation();
  TranslationVector translation = aICP->GetTranslation();

  MITK_INFO << "Rotation: \n" << rotation << "Translation: " << translation;
  MITK_INFO << "FRE: " << aICP->GetFRE();

  // visualization:
  // transform the fixed surface with the inverse transform
  // onto the moving surface.
  rotation = rotation.GetInverse();
  translation = (rotation * translation) * -1.0;

  mitk::AnisotropicRegistrationCommon::TransformPoints (
          fixedSurface->GetVtkPolyData()->GetPoints(),
          fixedSurface->GetVtkPolyData()->GetPoints(),
          rotation, translation
        );

  // set modified flag to update rendering
  fixedSurface->GetVtkPolyData()->Modified();

  //update view
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkSurfaceRegistrationView::OnEnableTreCalculation()
{
  if ( m_Controls.m_EnableTreCalculation->isChecked() )
    m_Controls.m_TargetSelectFrame->setEnabled(true);
  else
    m_Controls.m_TargetSelectFrame->setEnabled(false);
}
