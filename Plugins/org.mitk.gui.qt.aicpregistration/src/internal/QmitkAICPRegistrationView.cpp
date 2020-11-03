/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/


// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "QmitkAICPRegistrationView.h"

// Qt
#include <QMessageBox>
#include <QThread>

// MITK
#include <mitkSurface.h>
#include <mitkPointSet.h>
#include <mitkNodePredicateDataType.h>
#include <mitkAnisotropicIterativeClosestPointRegistration.h>
#include <mitkCovarianceMatrixCalculator.h>
#include <mitkAnisotropicRegistrationCommon.h>

// vtk
#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkCleanPolyData.h>

const std::string QmitkAICPRegistrationView::VIEW_ID = "org.mitk.views.aicpregistration";

 /**
   * @brief Pimpl holding the datastructures used by the
   *        QmitkAICPRegistrationView.
   */
class AICPRegistrationViewData
{

public:
  QThread* m_RegistrationThread;
  UIWorker* m_Worker;
  double m_Threshold;
  double m_MaxIterations;
  double m_TrimmFactor;
  double m_SearchRadius;

  // anisotropic registration
  mitk::AnisotropicIterativeClosestPointRegistration::Pointer m_AICP;

  // covariance matrix calculator
  mitk::CovarianceMatrixCalculator::Pointer m_MatrixCalculator;

  vtkSmartPointer<vtkCleanPolyData> m_CleanPolyData;

  mitk::Surface::Pointer m_MovingSurface;

  mitk::Surface::Pointer m_FixedSurface;

  // c tor
  AICPRegistrationViewData()
   : m_RegistrationThread(new QThread()),
     m_Worker(new UIWorker()),
     m_Threshold(0.00001),
     m_MaxIterations(1000),
     m_TrimmFactor(0.0),
     m_SearchRadius(30.0),
     m_AICP(mitk::AnisotropicIterativeClosestPointRegistration::New()),
     m_MatrixCalculator(mitk::CovarianceMatrixCalculator::New()),
     m_CleanPolyData(vtkSmartPointer<vtkCleanPolyData>::New()),
     m_MovingSurface(nullptr),
     m_FixedSurface(nullptr)
  {
  }

  // cleanup
  ~AICPRegistrationViewData()
  {
    if ( m_RegistrationThread )
      delete m_RegistrationThread;
    if ( m_Worker )
      delete m_Worker;

    m_AICP = nullptr;
    m_MatrixCalculator = nullptr;
    m_CleanPolyData = nullptr;
    m_MovingSurface = nullptr;
    m_FixedSurface = nullptr;
  }
};

QmitkAICPRegistrationView::QmitkAICPRegistrationView()
{
  d = new AICPRegistrationViewData();
}

QmitkAICPRegistrationView::~QmitkAICPRegistrationView()
{
  if ( d )
    delete d;
}

void QmitkAICPRegistrationView::SetFocus(){}

void QmitkAICPRegistrationView::CreateQtPartControl( QWidget *parent )
{
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls.setupUi( parent );

  // connect signals and slots
  connect ( m_Controls.m_EnableTreCalculation,SIGNAL(clicked()),this, SLOT(OnEnableTreCalculation()) );
  connect ( m_Controls.m_RegisterSurfaceButton, SIGNAL(clicked()), this, SLOT(OnStartRegistration()) );
  connect ( m_Controls.m_EnableTrimming, SIGNAL(clicked()), this, SLOT(OnEnableTrimming()) );
  connect ( d->m_Worker, SIGNAL( RegistrationFinished()), this, SLOT( OnRegistrationFinished()) );
  connect(d->m_RegistrationThread,SIGNAL(started()), d->m_Worker,SLOT(RegistrationThreadFunc()) );

  // move the u worker to the thread
  d->m_Worker->moveToThread(d->m_RegistrationThread);

  // setup tooltips
  m_Controls.m_MovingSurfaceComboBox->setToolTip("Set the moving surface of the A-ICP algorithm");
  m_Controls.m_FixedSurfaceComboBox->setToolTip("Set the fixed surface of the A-ICP algorithm");
  m_Controls.m_EnableTreCalculation->setToolTip("Enable the trimmed version of the algorithm.");
  m_Controls.m_TrimmFactorSpinbox->setToolTip("Set the trimmfactor. The algorithm will use a percentage of the Moving pointset for the registration. Valid number are between 0 and 1.");
  m_Controls.m_ThresholdSpinbox->setToolTip("Set the threshold to wich the algorithm will converge.");
  m_Controls.m_MaxIterationsSpinbox->setToolTip("The maximum number of iterations used by the algorithm.");
  m_Controls.m_SearchRadius->setToolTip("Set the search radius in mm for the calculation of the correspondences.");
  m_Controls.m_RegisterSurfaceButton->setToolTip("Start the registration.");
  m_Controls.m_EnableTrimming->setToolTip("Enables the trimmed version of the algorithm.");
  m_Controls.m_TrimmFactorSpinbox->setToolTip("Set teh overlapping part of the surface in %. The valid range is between 0 and 1.");
  m_Controls.m_MovingTargets->setToolTip("Select the targets for the moving surface.");
  m_Controls.m_FixedTargets->setToolTip("Select the targets for the fixed surface.");

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


bool QmitkAICPRegistrationView::CheckInput()
{
  QMessageBox msg;
  msg.setIcon(QMessageBox::Critical);

  if ( m_Controls.m_MovingSurfaceComboBox->GetSelectedNode().IsNull() ||
        m_Controls.m_FixedSurfaceComboBox->GetSelectedNode().IsNull() )
  {
    const char* message = "No Surfaces selected.";
    MITK_ERROR << message;
    msg.setText(message);
    msg.exec();
    return false;
  }

  if ( m_Controls.m_EnableTreCalculation->isChecked() )
  {
    if ( m_Controls.m_FixedTargets->GetSelectedNode().IsNull() ||
           m_Controls.m_MovingTargets->GetSelectedNode().IsNull() )
    {
      const char* message = "TRE calculation is enabled, but no target points are selected.";
      msg.setText(message);
      msg.exec();
      return false;
    }
  }
  return true;
}

void QmitkAICPRegistrationView::OnStartRegistration()
{
  d->m_Threshold = m_Controls.m_ThresholdSpinbox->value();
  d->m_MaxIterations = m_Controls.m_MaxIterationsSpinbox->value();
  d->m_SearchRadius = m_Controls.m_SearchRadius->value();
  d->m_TrimmFactor = 0.0;

  if ( m_Controls.m_EnableTrimming->isChecked() )
  {
    d->m_TrimmFactor = m_Controls.m_TrimmFactorSpinbox->value();
  }

  if (! CheckInput() )
    return;

  d->m_MovingSurface = dynamic_cast<mitk::Surface*>(
            m_Controls.m_MovingSurfaceComboBox->GetSelectedNode()->GetData() );

  d->m_FixedSurface = dynamic_cast<mitk::Surface*>(
            m_Controls.m_FixedSurfaceComboBox->GetSelectedNode()->GetData() );

  // sanity check
  if ( d->m_FixedSurface.IsNull() || d->m_MovingSurface.IsNull() )
  {
    const char* message = "Input surfaces are nullptr.";
    QMessageBox msg;
    msg.setIcon(QMessageBox::Critical);
    msg.setText(message);
    MITK_ERROR << message;
    return;
  }

  // enable trimming
  if ( m_Controls.m_EnableTrimming->isChecked() )
  {
    d->m_TrimmFactor = m_Controls.m_TrimmFactorSpinbox->value();
  }

  // set data into the UI thread
  d->m_Worker->SetRegistrationData(d);

  // start thread
  d->m_RegistrationThread->start();

  // disable registration button
  m_Controls.m_RegisterSurfaceButton->setEnabled(false);

  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkAICPRegistrationView::OnEnableTreCalculation()
{
  if ( m_Controls.m_EnableTreCalculation->isChecked() )
    m_Controls.m_TargetSelectFrame->setEnabled(true);
  else
    m_Controls.m_TargetSelectFrame->setEnabled(false);
}

void QmitkAICPRegistrationView::OnEnableTrimming()
{
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

void QmitkAICPRegistrationView::OnRegistrationFinished()
{
  typedef itk::Matrix<double,3,3> Matrix3x3;
  typedef itk::Vector<double,3> TranslationVector;

  double tre = -1.0;
  Matrix3x3 rotation = d->m_AICP->GetRotation();
  TranslationVector translation = d->m_AICP->GetTranslation();

  // exit the thread
  d->m_RegistrationThread->quit();

  MITK_INFO << "Rotation: \n" << rotation << "Translation: " << translation;
  MITK_INFO << "FRE: " << d->m_AICP->GetFRE();

  // compute TRE
  if ( m_Controls.m_EnableTreCalculation->isChecked() )
  {
    mitk::PointSet* movingTargets = dynamic_cast<mitk::PointSet*> (
                     m_Controls.m_MovingTargets->GetSelectedNode()->GetData() );

    mitk::PointSet* fixedTargets = dynamic_cast<mitk::PointSet*> (
                      m_Controls.m_FixedTargets->GetSelectedNode()->GetData() );

    // sanity check
    if ( movingTargets && fixedTargets )
    {
      // swap the moving and the fixed point set, since we use the inverse
      // transform
      tre = mitk::AnisotropicRegistrationCommon::ComputeTargetRegistrationError(
                                                                  movingTargets,
                                                                  fixedTargets,
                                                                  rotation,
                                                                  translation
                                                                );
      MITK_INFO << "TRE: " << tre;

      // transform the fixed point set
      for ( int i = 0; i < movingTargets->GetSize(); ++i )
      {
        mitk::Point3D p = movingTargets->GetPoint(i);

        p = rotation * p + translation;

        movingTargets->SetPoint(i,p);
      }
    }
  }
  // display result in textbox ( the inverse transform )
  QString text("");
  std::ostringstream oss;

  oss << "<b>Iterations:</b> "<< d->m_AICP->GetNumberOfIterations()
      << "<br><b>FRE:</b> " << d->m_AICP->GetFRE()
      << "<br><b>TRE:</b> ";

   if ( tre != -1.0)
    oss << tre;
   else
    oss << "N/A";

  oss << "<br><br><b>Rotation:</b><br>";

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
          d->m_MovingSurface->GetVtkPolyData()->GetPoints(),
          d->m_MovingSurface->GetVtkPolyData()->GetPoints(),
          rotation,
          translation
        );

  // set modified flag to update rendering
  d->m_MovingSurface->GetVtkPolyData()->Modified();

  // reanable registration button
  m_Controls.m_RegisterSurfaceButton->setEnabled(true);

  //update view
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void UIWorker::SetRegistrationData(AICPRegistrationViewData *data)
{
  this->d = data;
}

void UIWorker::RegistrationThreadFunc()
{
  typedef itk::Matrix<double,3,3> Matrix3x3;
  typedef std::vector<Matrix3x3> CovarianceMatrixList;

  // moving surface
  mitk::Surface::Pointer X = mitk::Surface::New();
  // helper
  vtkPolyData* data_X = vtkPolyData::New();
  // fixed surface
  mitk::Surface::Pointer Y = mitk::Surface::New();
  // helper
  vtkPolyData* data_Y = vtkPolyData::New();

  // clean the poly data to prevent manifold edges and duplicated vertices
  d->m_CleanPolyData->SetInputData(d->m_MovingSurface->GetVtkPolyData());
  d->m_CleanPolyData->Update();
  // copy the polys
  data_X->DeepCopy(d->m_CleanPolyData->GetOutput());
  X->SetVtkPolyData(data_X);

  d->m_CleanPolyData->SetInputData(d->m_FixedSurface->GetVtkPolyData());
  d->m_CleanPolyData->Update();
  data_Y->DeepCopy(d->m_CleanPolyData->GetOutput());
  Y->SetVtkPolyData(data_Y);

  // compute the covariance matrices for the moving surface (X)
  d->m_MatrixCalculator->SetInputSurface(X);
  d->m_MatrixCalculator->ComputeCovarianceMatrices();
  CovarianceMatrixList sigmas_X = d->m_MatrixCalculator->GetCovarianceMatrices();
  const double meanVarX = d->m_MatrixCalculator->GetMeanVariance();

  // compute the covariance matrices for the fixed surface (Y)
  d->m_MatrixCalculator->SetInputSurface(Y);
  d->m_MatrixCalculator->ComputeCovarianceMatrices();
  CovarianceMatrixList sigmas_Y = d->m_MatrixCalculator->GetCovarianceMatrices();
  const double meanVarY = d->m_MatrixCalculator->GetMeanVariance();

  // the FRE normalization factor
  const double normalizationFactor = sqrt( meanVarX + meanVarY);

  // set up parameters
  d->m_AICP->SetMovingSurface(X);
  d->m_AICP->SetFixedSurface(Y);
  d->m_AICP->SetCovarianceMatricesMovingSurface(sigmas_X);
  d->m_AICP->SetCovarianceMatricesFixedSurface(sigmas_Y);
  d->m_AICP->SetFRENormalizationFactor(normalizationFactor);
  d->m_AICP->SetMaxIterations(d->m_MaxIterations);
  d->m_AICP->SetSearchRadius(d->m_SearchRadius);
  d->m_AICP->SetThreshold(d->m_Threshold);
  d->m_AICP->SetTrimmFactor(d->m_TrimmFactor);

  // run the algorithm
  d->m_AICP->Update();

  data_X->Delete();
  data_Y->Delete();

  emit RegistrationFinished();
}
