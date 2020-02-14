/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkSurfaceUtilities.h"

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// MITK
#include <mitkDataNode.h>
#include <mitkNodePredicateDataType.h>
#include <mitkSurface.h>

#include "mitkSurfaceModifier.h"
#include "mitkTargetPointsCalculator.h"

const std::string QmitkSurfaceUtilities::VIEW_ID = "org.mitk.views.qmitksurfaceutilies";

QmitkSurfaceUtilities::QmitkSurfaceUtilities() : QmitkAbstractView()
{
}

QmitkSurfaceUtilities::~QmitkSurfaceUtilities()
{
}

void QmitkSurfaceUtilities::CreateQtPartControl(QWidget *parent)
{
  // build up qt view
  // create GUI widgets from the Qt Designer's .ui file
  m_Controls = Ui::QmitkSurfaceUtilitiesControls();
  m_Controls.setupUi(parent);

  connect(m_Controls.m_moveToOrigin, SIGNAL(clicked()), this, SLOT(OnMoveToOriginClicked()));
  connect(m_Controls.m_applyTransform, SIGNAL(clicked()), this, SLOT(OnApplyTransformClicked()));
  connect(m_Controls.m_computeMatrix, SIGNAL(clicked()), this, SLOT(OnEulerToMatrixClicked()));
  connect(m_Controls.m_ComputeCoG, SIGNAL(clicked()), this, SLOT(OnComputeCoG()));
  connect((QObject *)(m_Controls.m_generateTargetPoints), SIGNAL(clicked()), this, SLOT(OnGenerateTargetPoints()));
  connect(m_Controls.m_Perturbe, SIGNAL(clicked()), this, SLOT(OnPerturbeSurface()));
  connect(m_Controls.m_addOutliers, SIGNAL(clicked()), this, SLOT(OnAddOutliers()));

  m_Controls.m_applySurfaceBox->SetDataStorage(this->GetDataStorage());
  m_Controls.m_applySurfaceBox->SetAutoSelectNewItems(true);
  m_Controls.m_applySurfaceBox->SetPredicate(mitk::NodePredicateDataType::New("Surface"));
}

void QmitkSurfaceUtilities::SetFocus()
{
}

void QmitkSurfaceUtilities::OnPerturbeSurface()
{
  //########## check if everything is ready... ##################
  mitk::DataNode *node = m_Controls.m_applySurfaceBox->GetSelectedNode();
  if (!node)
  {
    // Nothing selected. Inform the user and return
    MITK_WARN << "ERROR: Please select a surface before!";
    return;
  }
  //#############################################################

  mitk::SurfaceModifier::Pointer myModifier = mitk::SurfaceModifier::New();
  myModifier->PerturbeSurface(dynamic_cast<mitk::Surface *>(m_Controls.m_applySurfaceBox->GetSelectedNode()->GetData()),
                              m_Controls.m_varianceX->value(),
                              m_Controls.m_varianceY->value(),
                              m_Controls.m_varianceZ->value());
}

void QmitkSurfaceUtilities::OnAddOutliers()
{
  //########## check if everything is ready... ##################
  mitk::DataNode *node = m_Controls.m_applySurfaceBox->GetSelectedNode();
  if (!node)
  {
    // Nothing selected. Inform the user and return
    MITK_WARN << "ERROR: Please select a surface before!";
    return;
  }
  //#############################################################

  double outlierChance = (double)m_Controls.m_outlierChance->value() / 100.0;
  mitk::SurfaceModifier::Pointer myModifier = mitk::SurfaceModifier::New();
  myModifier->AddOutlierToSurface(
    dynamic_cast<mitk::Surface *>(m_Controls.m_applySurfaceBox->GetSelectedNode()->GetData()),
    m_Controls.m_outlierVarianceX->value(),
    m_Controls.m_outlierVarianceY->value(),
    m_Controls.m_outlierVarianceZ->value(),
    outlierChance);
}

void QmitkSurfaceUtilities::OnGenerateTargetPoints()
{
  //########## check if everything is ready... ##################
  mitk::DataNode *node = m_Controls.m_applySurfaceBox->GetSelectedNode();
  if (!node)
  {
    // Nothing selected. Inform the user and return
    MITK_WARN << "ERROR: Please select a surface before!";
    return;
  }
  //#############################################################

  mitk::Surface::Pointer surface = dynamic_cast<mitk::Surface *>(node->GetData());

  mitk::TargetPointsCalculator::Pointer myTargetPointsCalculator = mitk::TargetPointsCalculator::New();
  myTargetPointsCalculator->SetInterPointDistance(m_Controls.m_InterPointDistance->value());
  myTargetPointsCalculator->SetInput(surface);
  MITK_INFO << "Calculating Target Points (this may take a while) ..." << std::endl;
  myTargetPointsCalculator->DoCalculate();
  mitk::PointSet::Pointer targetPoints = myTargetPointsCalculator->GetOutput();
  MITK_INFO << "      ... done." << std::endl;

  mitk::DataNode::Pointer targetPointsImageNode = mitk::DataNode::New();
  targetPointsImageNode->SetName("Target Points");
  targetPointsImageNode->SetData(targetPoints);
  this->GetDataStorage()->Add(targetPointsImageNode);
}

void QmitkSurfaceUtilities::OnApplyTransformClicked()
{
  // get surface from UI
  mitk::Surface::Pointer surface;
  if (m_Controls.m_applySurfaceBox->GetSelectedNode().IsNull())
  {
    MITK_WARN << "No valid surface selected!";
    return;
  }
  surface = dynamic_cast<mitk::Surface *>(m_Controls.m_applySurfaceBox->GetSelectedNode()->GetData());

  itk::Matrix<double, 3, 3> TransformationR;
  itk::Vector<double, 3> TransformationT;
  TransformationR = ReadInFromGUI_RotationMatrix();
  TransformationT = ReadInFromGUI_TranslationVector();

  // check if rotation matrix is empty
  if (this->ReadInFromGUI_RotationMatrix().GetVnlMatrix().is_zero())
  {
    MITK_WARN << "Error, rotation matrix is zero!";
    return;
  }

  // transform surface
  itk::Matrix<double, 3, 3> actualTransformationR;
  itk::Vector<double, 3> actualTransformationT;
  mitk::SurfaceModifier::Pointer myModifier = mitk::SurfaceModifier::New();
  myModifier->TransformSurfaceCoGCoordinates(
    surface, TransformationR, TransformationT, actualTransformationR, actualTransformationT);

  // update view
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkSurfaceUtilities::OnEulerToMatrixClicked()
{
  // read angles from GUI:
  double alpha = m_Controls.m_eulerangle0->text().toDouble();
  double beta = m_Controls.m_eulerangle1->text().toDouble();
  double gamma = m_Controls.m_eulerangle2->text().toDouble();

  // convert angles to matrix:
  itk::Matrix<double, 3, 3> matrix = ConvertEulerAnglesToRotationMatrix(alpha, beta, gamma);

  // write matrix to GUI
  this->WriteToGUI_RotationMatrix(matrix);
}

void QmitkSurfaceUtilities::OnMoveToOriginClicked()
{
  // get surface from UI
  mitk::Surface::Pointer surface;
  if (m_Controls.m_applySurfaceBox->GetSelectedNode().IsNull())
  {
    MITK_WARN << "No surface selected!";
    return;
  }
  surface = dynamic_cast<mitk::Surface *>(m_Controls.m_applySurfaceBox->GetSelectedNode()->GetData());

  // move surface to center
  mitk::SurfaceModifier::Pointer myModifier = mitk::SurfaceModifier::New();
  itk::Matrix<double, 3, 3> TransformToCenterR;
  itk::Vector<double, 3> TransformToCenterT;
  myModifier->MoveSurfaceToCenter(surface, TransformToCenterR, TransformToCenterT);

  // update view
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();
}

void QmitkSurfaceUtilities::OnComputeCoG()
{
  // check if input is valid
  if (this->m_Controls.m_applySurfaceBox->GetSelectedNode().IsNull())
  {
    MITK_WARN << "Error, no surface selected.";
    return;
  }

  // get selected surface and name of surface
  mitk::Surface::Pointer selectedSurface =
    dynamic_cast<mitk::Surface *>(this->m_Controls.m_applySurfaceBox->GetSelectedNode()->GetData());
  std::string nameOfSelectedSurface = this->m_Controls.m_applySurfaceBox->GetSelectedNode()->GetName();

  // calculate target point
  mitk::TargetPointsCalculator::Pointer myTargetPointsCalculator = mitk::TargetPointsCalculator::New();
  myTargetPointsCalculator->SetTargetPointCalculationMethod(
    mitk::TargetPointsCalculator::OneTargetPointInCenterOfGravity);
  myTargetPointsCalculator->SetInput(selectedSurface);
  myTargetPointsCalculator->DoCalculate();
  mitk::PointSet::Pointer targetPoints = myTargetPointsCalculator->GetOutput();

  // create a new data node with targets
  mitk::DataNode::Pointer newNode = mitk::DataNode::New();
  newNode->SetName(nameOfSelectedSurface + "_CenterOfGravity");
  newNode->SetData(targetPoints);

  // add the new node to the data storage
  this->GetDataStorage()->Add(newNode);
  this->GetDataStorage()->Modified();
}

itk::Matrix<double, 3, 3> QmitkSurfaceUtilities::ReadInFromGUI_RotationMatrix()
{
  itk::Matrix<double, 3, 3> transR;
  transR[0][0] = m_Controls.m_rotmat00->text().toDouble();
  transR[1][0] = m_Controls.m_rotmat10->text().toDouble();
  transR[2][0] = m_Controls.m_rotmat20->text().toDouble();
  transR[0][1] = m_Controls.m_rotmat01->text().toDouble();
  transR[1][1] = m_Controls.m_rotmat11->text().toDouble();
  transR[2][1] = m_Controls.m_rotmat21->text().toDouble();
  transR[0][2] = m_Controls.m_rotmat02->text().toDouble();
  transR[1][2] = m_Controls.m_rotmat12->text().toDouble();
  transR[2][2] = m_Controls.m_rotmat22->text().toDouble();
  return transR;
}

itk::Vector<double, 3> QmitkSurfaceUtilities::ReadInFromGUI_TranslationVector()
{
  itk::Vector<double, 3> transT;
  transT[0] = m_Controls.m_tvec0->text().toDouble();
  transT[1] = m_Controls.m_tvec1->text().toDouble();
  transT[2] = m_Controls.m_tvec2->text().toDouble();
  return transT;
}

void QmitkSurfaceUtilities::WriteToGUI_RotationMatrix(itk::Matrix<double, 3, 3> r)
{
  m_Controls.m_rotmat00->setText(QString::number(r[0][0]));
  m_Controls.m_rotmat01->setText(QString::number(r[0][1]));
  m_Controls.m_rotmat02->setText(QString::number(r[0][2]));
  m_Controls.m_rotmat10->setText(QString::number(r[1][0]));
  m_Controls.m_rotmat11->setText(QString::number(r[1][1]));
  m_Controls.m_rotmat12->setText(QString::number(r[1][2]));
  m_Controls.m_rotmat20->setText(QString::number(r[2][0]));
  m_Controls.m_rotmat21->setText(QString::number(r[2][1]));
  m_Controls.m_rotmat22->setText(QString::number(r[2][2]));
}

void QmitkSurfaceUtilities::WriteToGUI_TranslationVector(itk::Vector<double, 3> t)
{
  m_Controls.m_tvec0->setText(QString::number(t[0]));
  m_Controls.m_tvec1->setText(QString::number(t[1]));
  m_Controls.m_tvec2->setText(QString::number(t[2]));
}

itk::Matrix<double, 3, 3> QmitkSurfaceUtilities::ConvertEulerAnglesToRotationMatrix(double alpha,
                                                                                    double beta,
                                                                                    double gamma)
{
  double PI = 3.141592653589793;
  alpha = alpha * PI / 180;
  beta = beta * PI / 180;
  gamma = gamma * PI / 180;

  // convert angles to matrix:
  itk::Matrix<double, 3, 3> matrix;

  /* x-Konvention (Z, X, Z)
  matrix[0][0] = cos(alpha) * cos(gamma) - sin(alpha) * cos(beta) * sin(gamma);
  matrix[0][1] = -cos(alpha) * sin(gamma)- sin(alpha) * cos(beta) * cos(gamma);
  matrix[0][2] = sin(alpha) * sin(beta);

  matrix[1][0] = sin(alpha) * cos(gamma) + cos(alpha) * cos(beta) * sin(gamma);
  matrix[1][1] = cos(alpha) * cos(beta) * cos(gamma) - sin(alpha) * sin(gamma);
  matrix[1][2] = -cos(alpha) * sin(beta);

  matrix[2][0] = sin(beta) * sin(gamma);
  matrix[2][1] = sin(beta) * cos(gamma);
  matrix[2][2] = cos(beta); */

  // Luftfahrtnorm (DIN 9300) (Yaw-Pitch-Roll, Z, Y, X)
  matrix[0][0] = cos(beta) * cos(alpha);
  matrix[0][1] = cos(beta) * sin(alpha);
  matrix[0][2] = -sin(beta);

  matrix[1][0] = sin(gamma) * sin(beta) * cos(alpha) - cos(gamma) * sin(alpha);
  matrix[1][1] = sin(gamma) * sin(beta) * sin(alpha) + cos(gamma) * cos(alpha);
  matrix[1][2] = sin(gamma) * cos(beta);

  matrix[2][0] = cos(gamma) * sin(beta) * cos(alpha) + sin(gamma) * sin(alpha);
  matrix[2][1] = cos(gamma) * sin(beta) * sin(alpha) - sin(gamma) * cos(alpha);
  matrix[2][2] = cos(gamma) * cos(beta);

  return matrix;
}
