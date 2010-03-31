/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: -1 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkAmoebaOptimizerView.h"
#include "mitkImageAccessByItk.h"
#include <itkAmoebaOptimizer.h>

QmitkAmoebaOptimizerView::QmitkAmoebaOptimizerView(QWidget* parent, Qt::WindowFlags f ) : QmitkRigidRegistrationOptimizerGUIBase (parent, f),
m_NumberTransformParameters(15)
{
}

QmitkAmoebaOptimizerView::~QmitkAmoebaOptimizerView()
{
}

itk::Object::Pointer QmitkAmoebaOptimizerView::GetOptimizer()
{
  itk::AmoebaOptimizer::Pointer OptimizerPointer = itk::AmoebaOptimizer::New();
  OptimizerPointer->SetMaximize( m_Controls.m_Maximize->isChecked() );
  OptimizerPointer->SetParametersConvergenceTolerance(m_Controls.m_ParametersConvergenceToleranceAmoeba->text().toFloat());
  OptimizerPointer->SetFunctionConvergenceTolerance(m_Controls.m_FunctionConvergenceToleranceAmoeba->text().toFloat());
  typedef itk::SingleValuedNonLinearOptimizer OptimizerType;
  OptimizerType::ParametersType simplexDelta( m_NumberTransformParameters );
  itk::Array<double> simplexDeltaAmoeba = this->GetOptimizerParameters();
  for (int i = 0; i < m_NumberTransformParameters; i++)
  {
    simplexDelta[i] = simplexDeltaAmoeba[i+1];
  }
  OptimizerPointer->AutomaticInitialSimplexOff();
  OptimizerPointer->SetInitialSimplexDelta( simplexDelta );
  OptimizerPointer->SetMaximumNumberOfIterations( m_Controls.m_IterationsAmoeba->text().toInt() );
  return OptimizerPointer.GetPointer();
}

itk::Array<double> QmitkAmoebaOptimizerView::GetOptimizerParameters()
{
  itk::Array<double> optimizerValues;
  optimizerValues.SetSize(19);
  optimizerValues.fill(0);
  optimizerValues[0] = m_Controls.m_Maximize->isChecked();
  optimizerValues[1] = m_Controls.m_SimplexDeltaAmoeba1->text().toDouble();
  optimizerValues[2] = m_Controls.m_SimplexDeltaAmoeba2->text().toDouble();
  optimizerValues[3] = m_Controls.m_SimplexDeltaAmoeba3->text().toDouble();
  optimizerValues[4] = m_Controls.m_SimplexDeltaAmoeba4->text().toDouble();
  optimizerValues[5] = m_Controls.m_SimplexDeltaAmoeba5->text().toDouble();
  optimizerValues[6] = m_Controls.m_SimplexDeltaAmoeba6->text().toDouble();
  optimizerValues[7] = m_Controls.m_SimplexDeltaAmoeba7->text().toDouble();
  optimizerValues[8] = m_Controls.m_SimplexDeltaAmoeba8->text().toDouble();
  optimizerValues[9] = m_Controls.m_SimplexDeltaAmoeba9->text().toDouble();
  optimizerValues[10] = m_Controls.m_SimplexDeltaAmoeba10->text().toDouble();
  optimizerValues[11] = m_Controls.m_SimplexDeltaAmoeba11->text().toDouble();
  optimizerValues[12] = m_Controls.m_SimplexDeltaAmoeba12->text().toDouble();
  optimizerValues[13] = m_Controls.m_SimplexDeltaAmoeba13->text().toDouble();
  optimizerValues[14] = m_Controls.m_SimplexDeltaAmoeba14->text().toDouble();
  optimizerValues[15] = m_Controls.m_SimplexDeltaAmoeba15->text().toDouble();
  optimizerValues[16] = m_Controls.m_ParametersConvergenceToleranceAmoeba->text().toFloat();
  optimizerValues[17] = m_Controls.m_FunctionConvergenceToleranceAmoeba->text().toFloat();
  optimizerValues[18] = m_Controls.m_IterationsAmoeba->text().toInt();
  return optimizerValues;
}

void QmitkAmoebaOptimizerView::SetOptimizerParameters(itk::Array<double> optimizerValues)
{
  m_Controls.m_Maximize->setChecked(optimizerValues[0]);
  m_Controls.m_SimplexDeltaAmoeba1->setText(QString::number(optimizerValues[1]));
  m_Controls.m_SimplexDeltaAmoeba2->setText(QString::number(optimizerValues[2]));
  m_Controls.m_SimplexDeltaAmoeba3->setText(QString::number(optimizerValues[3]));
  m_Controls.m_SimplexDeltaAmoeba4->setText(QString::number(optimizerValues[4]));
  m_Controls.m_SimplexDeltaAmoeba5->setText(QString::number(optimizerValues[5]));
  m_Controls.m_SimplexDeltaAmoeba6->setText(QString::number(optimizerValues[6]));
  m_Controls.m_SimplexDeltaAmoeba7->setText(QString::number(optimizerValues[7]));
  m_Controls.m_SimplexDeltaAmoeba8->setText(QString::number(optimizerValues[8]));
  m_Controls.m_SimplexDeltaAmoeba9->setText(QString::number(optimizerValues[9]));
  m_Controls.m_SimplexDeltaAmoeba10->setText(QString::number(optimizerValues[10]));
  m_Controls.m_SimplexDeltaAmoeba11->setText(QString::number(optimizerValues[11]));
  m_Controls.m_SimplexDeltaAmoeba12->setText(QString::number(optimizerValues[12]));
  m_Controls.m_SimplexDeltaAmoeba13->setText(QString::number(optimizerValues[13]));
  m_Controls.m_SimplexDeltaAmoeba14->setText(QString::number(optimizerValues[14]));
  m_Controls.m_SimplexDeltaAmoeba15->setText(QString::number(optimizerValues[15]));
  m_Controls.m_ParametersConvergenceToleranceAmoeba->setText(QString::number(optimizerValues[16]));
  m_Controls.m_FunctionConvergenceToleranceAmoeba->setText(QString::number(optimizerValues[17]));
  m_Controls.m_IterationsAmoeba->setText(QString::number(optimizerValues[18]));
}

void QmitkAmoebaOptimizerView::SetNumberOfTransformParameters(int transformParameters)
{
  m_NumberTransformParameters = transformParameters;
  this->ShowSimplexDelta();
}

QString QmitkAmoebaOptimizerView::GetName()
{
  return "Amoeba";
}

void QmitkAmoebaOptimizerView::SetupUI(QWidget* parent)
{
  m_Controls.setupUi(parent);
  QValidator* validatorLineEditInput = new QIntValidator(0, 20000000, this);
  m_Controls.m_IterationsAmoeba->setValidator(validatorLineEditInput);
  QValidator* validatorLineEditInputFloat = new QDoubleValidator(0, 20000000, 8, this);
  m_Controls.m_SimplexDeltaAmoeba1->setValidator(validatorLineEditInputFloat);
  m_Controls.m_SimplexDeltaAmoeba2->setValidator(validatorLineEditInputFloat);
  m_Controls.m_SimplexDeltaAmoeba3->setValidator(validatorLineEditInputFloat);
  m_Controls.m_SimplexDeltaAmoeba4->setValidator(validatorLineEditInputFloat);
  m_Controls.m_SimplexDeltaAmoeba5->setValidator(validatorLineEditInputFloat);
  m_Controls.m_SimplexDeltaAmoeba6->setValidator(validatorLineEditInputFloat);
  m_Controls.m_SimplexDeltaAmoeba7->setValidator(validatorLineEditInputFloat);
  m_Controls.m_SimplexDeltaAmoeba8->setValidator(validatorLineEditInputFloat);
  m_Controls.m_SimplexDeltaAmoeba9->setValidator(validatorLineEditInputFloat);
  m_Controls.m_SimplexDeltaAmoeba10->setValidator(validatorLineEditInputFloat);
  m_Controls.m_SimplexDeltaAmoeba11->setValidator(validatorLineEditInputFloat);
  m_Controls.m_SimplexDeltaAmoeba12->setValidator(validatorLineEditInputFloat);
  m_Controls.m_SimplexDeltaAmoeba13->setValidator(validatorLineEditInputFloat);
  m_Controls.m_SimplexDeltaAmoeba14->setValidator(validatorLineEditInputFloat);
  m_Controls.m_SimplexDeltaAmoeba15->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ParametersConvergenceToleranceAmoeba->setValidator(validatorLineEditInputFloat);
  m_Controls.m_FunctionConvergenceToleranceAmoeba->setValidator(validatorLineEditInputFloat);
  simplexDeltaLineEdits.clear();
  simplexDeltaLineEdits.push_back(m_Controls.m_SimplexDeltaAmoeba1);
  simplexDeltaLineEdits.push_back(m_Controls.m_SimplexDeltaAmoeba2);
  simplexDeltaLineEdits.push_back(m_Controls.m_SimplexDeltaAmoeba3);
  simplexDeltaLineEdits.push_back(m_Controls.m_SimplexDeltaAmoeba4);
  simplexDeltaLineEdits.push_back(m_Controls.m_SimplexDeltaAmoeba5);
  simplexDeltaLineEdits.push_back(m_Controls.m_SimplexDeltaAmoeba6);
  simplexDeltaLineEdits.push_back(m_Controls.m_SimplexDeltaAmoeba7);
  simplexDeltaLineEdits.push_back(m_Controls.m_SimplexDeltaAmoeba8);
  simplexDeltaLineEdits.push_back(m_Controls.m_SimplexDeltaAmoeba9);
  simplexDeltaLineEdits.push_back(m_Controls.m_SimplexDeltaAmoeba10);
  simplexDeltaLineEdits.push_back(m_Controls.m_SimplexDeltaAmoeba11);
  simplexDeltaLineEdits.push_back(m_Controls.m_SimplexDeltaAmoeba12);
  simplexDeltaLineEdits.push_back(m_Controls.m_SimplexDeltaAmoeba13);
  simplexDeltaLineEdits.push_back(m_Controls.m_SimplexDeltaAmoeba14);
  simplexDeltaLineEdits.push_back(m_Controls.m_SimplexDeltaAmoeba15);
  simplexDeltaLabels.clear();
  simplexDeltaLabels.push_back(m_Controls.m_SimplexDeltaAmoebaLabel1);
  simplexDeltaLabels.push_back(m_Controls.m_SimplexDeltaAmoebaLabel2);
  simplexDeltaLabels.push_back(m_Controls.m_SimplexDeltaAmoebaLabel3);
  simplexDeltaLabels.push_back(m_Controls.m_SimplexDeltaAmoebaLabel4);
  simplexDeltaLabels.push_back(m_Controls.m_SimplexDeltaAmoebaLabel5);
  simplexDeltaLabels.push_back(m_Controls.m_SimplexDeltaAmoebaLabel6);
  simplexDeltaLabels.push_back(m_Controls.m_SimplexDeltaAmoebaLabel7);
  simplexDeltaLabels.push_back(m_Controls.m_SimplexDeltaAmoebaLabel8);
  simplexDeltaLabels.push_back(m_Controls.m_SimplexDeltaAmoebaLabel9);
  simplexDeltaLabels.push_back(m_Controls.m_SimplexDeltaAmoebaLabel10);
  simplexDeltaLabels.push_back(m_Controls.m_SimplexDeltaAmoebaLabel11);
  simplexDeltaLabels.push_back(m_Controls.m_SimplexDeltaAmoebaLabel12);
  simplexDeltaLabels.push_back(m_Controls.m_SimplexDeltaAmoebaLabel13);
  simplexDeltaLabels.push_back(m_Controls.m_SimplexDeltaAmoebaLabel14);
  simplexDeltaLabels.push_back(m_Controls.m_SimplexDeltaAmoebaLabel15);
}

void QmitkAmoebaOptimizerView::ShowSimplexDelta()
{
  std::list<QLineEdit*>::iterator it_edit = simplexDeltaLineEdits.begin();
  for (it_edit = simplexDeltaLineEdits.begin(); it_edit != simplexDeltaLineEdits.end(); it_edit++)
  {
    (*it_edit)->hide();
  }
  it_edit = simplexDeltaLineEdits.begin();
  for (int i = 0; i < m_NumberTransformParameters; i++)
  {
    (*it_edit)->show();
    it_edit++;
  }
  std::list<QLabel*>::iterator it_label = simplexDeltaLabels.begin();
  for (it_label = simplexDeltaLabels.begin(); it_label != simplexDeltaLabels.end(); it_label++)
  {
    (*it_label)->hide();
  }
  it_label = simplexDeltaLabels.begin();
  for (int i = 0; i < m_NumberTransformParameters; i++)
  {
    (*it_label)->show();
    it_label++;
  }
  this->updateGeometry();
}
