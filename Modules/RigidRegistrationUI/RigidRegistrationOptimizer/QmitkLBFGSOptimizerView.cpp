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

#include "QmitkLBFGSOptimizerView.h"
#include "mitkImageAccessByItk.h"
#include <itkLBFGSOptimizer.h>

QmitkLBFGSOptimizerView::QmitkLBFGSOptimizerView(QWidget* parent, Qt::WindowFlags f ) : QmitkRigidRegistrationOptimizerGUIBase (parent, f),
m_NumberTransformParameters(16)
{

}

QmitkLBFGSOptimizerView::~QmitkLBFGSOptimizerView()
{
}

itk::Object::Pointer QmitkLBFGSOptimizerView::GetOptimizer()
{
  itk::LBFGSOptimizer::Pointer OptimizerPointer = itk::LBFGSOptimizer::New();
  OptimizerPointer->SetMaximize( m_Controls.m_Maximize->isChecked() );
  OptimizerPointer->SetGradientConvergenceTolerance( m_Controls.m_GradientMagnitudeToleranceLBFGS->text().toFloat() );
  OptimizerPointer->SetLineSearchAccuracy( m_Controls.m_LineSearchAccuracyLBFGS->text().toFloat() );
  OptimizerPointer->SetDefaultStepLength( m_Controls.m_DefaultStepLengthLBFGS->text().toFloat() );
  OptimizerPointer->SetMaximumNumberOfFunctionEvaluations( m_Controls.m_MaximumEvaluationsLBFGS->text().toInt() );
  OptimizerPointer->SetTrace(m_Controls.m_TraceOnLBFGS->isChecked());
  return OptimizerPointer.GetPointer();
}

itk::Array<double> QmitkLBFGSOptimizerView::GetOptimizerParameters()
{
  itk::Array<double> optimizerValues;
  optimizerValues.SetSize(6);
  optimizerValues.fill(0);
  optimizerValues[0] = m_Controls.m_Maximize->isChecked();
  optimizerValues[1] = m_Controls.m_GradientMagnitudeToleranceLBFGS->text().toFloat();
  optimizerValues[2] = m_Controls.m_LineSearchAccuracyLBFGS->text().toFloat();
  optimizerValues[3] = m_Controls.m_DefaultStepLengthLBFGS->text().toFloat();
  optimizerValues[4] = m_Controls.m_MaximumEvaluationsLBFGS->text().toInt();
  optimizerValues[5] = m_Controls.m_TraceOnLBFGS->isChecked();
  return optimizerValues;
}

void QmitkLBFGSOptimizerView::SetOptimizerParameters(itk::Array<double> optimizerValues)
{
  m_Controls.m_Maximize->setChecked(optimizerValues[0]);
  m_Controls.m_GradientMagnitudeToleranceLBFGS->setText(QString::number(optimizerValues[1]));
  m_Controls.m_LineSearchAccuracyLBFGS->setText(QString::number(optimizerValues[2]));
  m_Controls.m_DefaultStepLengthLBFGS->setText(QString::number(optimizerValues[3]));
  m_Controls.m_MaximumEvaluationsLBFGS->setText(QString::number(optimizerValues[4]));
  m_Controls.m_TraceOnLBFGS->setChecked(optimizerValues[5]);
}

void QmitkLBFGSOptimizerView::SetNumberOfTransformParameters(int transformParameters)
{
  m_NumberTransformParameters = transformParameters;
}

QString QmitkLBFGSOptimizerView::GetName()
{
  return "LBFGS";
}

void QmitkLBFGSOptimizerView::SetupUI(QWidget* parent)
{
  m_Controls.setupUi(parent);
  QValidator* validatorLineEditInput = new QIntValidator(0, 20000000, this);
  m_Controls.m_MaximumEvaluationsLBFGS->setValidator(validatorLineEditInput);
  QValidator* validatorLineEditInputFloat = new QDoubleValidator(0, 20000000, 8, this);
  m_Controls.m_GradientMagnitudeToleranceLBFGS->setValidator(validatorLineEditInputFloat);
  m_Controls.m_LineSearchAccuracyLBFGS->setValidator(validatorLineEditInputFloat);
  m_Controls.m_DefaultStepLengthLBFGS->setValidator(validatorLineEditInputFloat);
}
