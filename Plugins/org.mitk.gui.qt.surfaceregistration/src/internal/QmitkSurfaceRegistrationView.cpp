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
#include <QThread>

#include <mitkSurface.h>
#include <mitkPointSet.h>
#include <mitkNodePredicateDataType.h>
#include <mitkAnisotropicIterativeClosestPointRegistration.h>
#include <mitkCovarianceMatrixCalculator.h>
#include <mitkAnisotropicRegistrationCommon.h>
#include <vtkPolyData.h>

const std::string QmitkSurfaceRegistrationView::VIEW_ID = "org.mitk.views.a-icpregistration";

class SurfaceRegistrationViewData
{
public:
  QThread* m_RegistrationThread;
  double m_Threshold;
  double m_MaxIterations;
  double m_TrimmFactor;

  // anisotropic registration
  mitk::AnisotropicIterativeClosestPointRegistration::Pointer m_AICP;

  // covariance matrix calculator
  mitk::CovarianceMatrixCalculator::Pointer m_MatrixCalculator;

  mitk::Surface::Pointer m_MovingSurface;

  mitk::Surface::Pointer m_FixedSurface;

  SurfaceRegistrationViewData()
   : m_RegistrationThread(new QThread),
     m_Threshold(0.00001),
     m_MaxIterations(1000),
     m_TrimmFactor(0.0),
     m_AICP(mitk::AnisotropicIterativeClosestPointRegistration::New()),
     m_MatrixCalculator(mitk::CovarianceMatrixCalculator::New()),
     m_MovingSurface(NULL),
     m_FixedSurface(NULL)
  {
  }

  ~SurfaceRegistrationViewData()
  {
    if ( m_RegistrationThread )
      delete m_RegistrationThread;

    m_AICP = NULL;
    m_MatrixCalculator = NULL;
  }
};

QmitkSurfaceRegistrationView::QmitkSurfaceRegistrationView()
{
  d = new SurfaceRegistrationViewData();
}

QmitkSurfaceRegistrationView::~QmitkSurfaceRegistrationView()
{
  if ( d )
    delete d;
}

void QmitkSurfaceRegistrationView::SetFocus()
{
}

void QmitkSurfaceRegistrationView::CreateQtPartControl( QWidget *parent )
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi( parent );

  connect ( m_Controls.m_EnableTreCalculation,SIGNAL(clicked()),this, SLOT(OnEnableTreCalculation()) );
  connect ( m_Controls.m_RegisterSurfaceButton, SIGNAL(clicked()), this, SLOT(OnRunRegistration()) );
  connect ( m_Controls.m_EnableTrimming, SIGNAL(clicked()), this, SLOT(OnEnableTrimming()) );

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

  // disable trimming options
  m_Controls.m_TrimmFactorLabel->setEnabled(false);
  m_Controls.m_TrimmFactorSpinbox->setEnabled(false);
}

void QmitkSurfaceRegistrationView::OnRunRegistration()
{
  typedef itk::Matrix<double,3,3> Matrix3x3;
  typedef itk::Vector<double,3> TranslationVector;
  typedef std::vector<Matrix3x3> CovarianceMatrixList;

  d->m_Threshold = m_Controls.m_ThresholdSpinbox->value();
  d->m_MaxIterations = m_Controls.m_MaxIterationsSpinbox->value();
  d->m_TrimmFactor = 0.0;

  if ( m_Controls.m_EnableTrimming->isChecked() )
  {
    d->m_TrimmFactor = m_Controls.m_TrimmFactorSpinbox->value();
  }



  d->m_MovingSurface = dynamic_cast<mitk::Surface*>(
            m_Controls.m_MovingSurfaceComboBox->GetSelectedNode()->GetData() );

  d->m_FixedSurface = dynamic_cast<mitk::Surface*>(
            m_Controls.m_FixedSurfaceComboBox->GetSelectedNode()->GetData() );

  // sanity check
  if ( d->m_FixedSurface.IsNull() || d->m_MovingSurface.IsNull() )
  {
    MITK_ERROR << "Input surface not set";
    return;
  }

  // enable trimming
  if ( m_Controls.m_EnableTrimming->isChecked() )
  {
    d->m_TrimmFactor = m_Controls.m_TrimmFactorSpinbox->value();
  }

  // compute the covariance matrices for the moving surface (X)
  d->m_MatrixCalculator->SetInputSurface(d->m_MovingSurface);
  d->m_MatrixCalculator->ComputeCovarianceMatrices();
  CovarianceMatrixList sigmas_X = d->m_MatrixCalculator->GetCovarianceMatrices();
  double meanVarX = d->m_MatrixCalculator->GetMeanVariance();

  // compute the covariance matrices for the fixed surface (Y)
  d->m_MatrixCalculator->SetInputSurface(d->m_FixedSurface);
  d->m_MatrixCalculator->ComputeCovarianceMatrices();
  CovarianceMatrixList sigmas_Y = d->m_MatrixCalculator->GetCovarianceMatrices();
  double meanVarY = d->m_MatrixCalculator->GetMeanVariance();

  // the FRE normalization factor
  double normalizationFactor = sqrt( meanVarX + meanVarY);

  // set up parameters
  d->m_AICP->SetMovingSurface(d->m_MovingSurface);
  d->m_AICP->SetFixedSurface(d->m_FixedSurface);
  d->m_AICP->SetCovarianceMatricesMovingSurface(sigmas_X);
  d->m_AICP->SetCovarianceMatricesFixedSurface(sigmas_Y);
  d->m_AICP->SetFRENormalizationFactor(normalizationFactor);
  d->m_AICP->SetMaxIterations(d->m_MaxIterations);
  d->m_AICP->SetThreshold(d->m_Threshold);
  d->m_AICP->SetTrimmFactor(d->m_TrimmFactor);

  // run the algorithm
  d->m_AICP->Update();

  Matrix3x3 rotation = d->m_AICP->GetRotation();
  TranslationVector translation = d->m_AICP->GetTranslation();

  double tre = -1.0;
  // compute TRE
  if ( m_Controls.m_EnableTreCalculation->isChecked() )
  {
    mitk::PointSet::Pointer movingTargets = dynamic_cast<mitk::PointSet*> (
          m_Controls.m_MovingTargets->GetSelectedNode()->GetData() );

    mitk::PointSet::Pointer fixedTargets = dynamic_cast<mitk::PointSet*> (
          m_Controls.m_FixedTargets->GetSelectedNode()->GetData() );

    // sanity check
    if ( movingTargets.IsNotNull() && fixedTargets.IsNotNull() )
    {
      tre = mitk::AnisotropicRegistrationCommon::ComputeTargetRegistrationError (
                                                                       movingTargets,
                                                                       fixedTargets,
                                                                       rotation,
                                                                       translation
                                                                     );
      MITK_INFO << "TRE: " << tre;
    }
  }

  // Visualization:
  // transform the fixed surface with the inverse transform
  // onto the moving surface.
  rotation = rotation.GetInverse();
  translation = (rotation * translation) * -1.0;

  MITK_INFO << "Rotation: \n" << rotation << "Translation: " << translation;
  MITK_INFO << "FRE: " << d->m_AICP->GetFRE();

  // display result in textbox ( the inverse transform )
  QString text("");
  std::ostringstream oss;

  oss << "<b>Iterations:</b> "<< d->m_AICP->GetNumberOfIterations()
      << "<br><b>FRE:</b> " << d->m_AICP->GetFRE()
      << "<br><b>TRE:</b> " << tre
      << "<br><br><b>Rotation:</b><br>";

  for ( int i = 0; i < 3; ++i ) {
    for ( int j = 0; j < 3; ++j )
      oss << rotation[i][j] << " ";
    oss << "<br>";
  }

  oss << "<br><b>Translation:</b><br>" << translation << "<br>";

  std::string s(oss.str());
  text.append(s.c_str());

  m_Controls.m_TextEdit->clear();
  m_Controls.m_TextEdit->append(text);

  mitk::AnisotropicRegistrationCommon::TransformPoints (
          d->m_FixedSurface->GetVtkPolyData()->GetPoints(),
          d->m_FixedSurface->GetVtkPolyData()->GetPoints(),
          rotation, translation
        );

  // set modified flag to update rendering
  d->m_FixedSurface->GetVtkPolyData()->Modified();

  //update view
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();

  // reanable registration button
  m_Controls.m_RegisterSurfaceButton->setEnabled(true);
}

void QmitkSurfaceRegistrationView::OnEnableTreCalculation()
{
  if ( m_Controls.m_EnableTreCalculation->isChecked() )
    m_Controls.m_TargetSelectFrame->setEnabled(true);
  else
    m_Controls.m_TargetSelectFrame->setEnabled(false);
}

void QmitkSurfaceRegistrationView::OnEnableTrimming()
{
  MITK_INFO << "enable trimming";
  if ( m_Controls.m_EnableTrimming->isChecked() )
  {
    // disable trimming options
    m_Controls.m_TrimmFactorLabel->setEnabled(true);
    m_Controls.m_TrimmFactorSpinbox->setEnabled(true);
  } else {
    // disable trimming options
    m_Controls.m_TrimmFactorLabel->setEnabled(false);
    m_Controls.m_TrimmFactorSpinbox->setEnabled(false);
  }
}
