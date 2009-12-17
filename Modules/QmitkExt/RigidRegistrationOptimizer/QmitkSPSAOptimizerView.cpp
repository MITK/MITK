/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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

#include "QmitkSPSAOptimizerView.h"
#include "mitkImageAccessByItk.h"
#include <itkSPSAOptimizer.h>

QmitkSPSAOptimizerView::QmitkSPSAOptimizerView(QWidget* parent, Qt::WindowFlags f ) : QmitkRigidRegistrationOptimizerGUIBase (parent, f),
m_NumberTransformParameters(16)
{

}

QmitkSPSAOptimizerView::~QmitkSPSAOptimizerView()
{
}

itk::Object::Pointer QmitkSPSAOptimizerView::GetOptimizer()
{
  itk::SPSAOptimizer::Pointer OptimizerPointer = itk::SPSAOptimizer::New();
  OptimizerPointer->SetMaximize( m_Controls.m_Maximize->isChecked() );
  OptimizerPointer->Seta( m_Controls.m_aSPSA->text().toFloat() );
  OptimizerPointer->SetA( m_Controls.m_ASPSA->text().toFloat() );
  OptimizerPointer->SetAlpha( m_Controls.m_AlphaSPSA->text().toFloat() );
  OptimizerPointer->Setc( m_Controls.m_cSPSA->text().toFloat() );
  OptimizerPointer->SetGamma( m_Controls.m_GammaSPSA->text().toFloat() );
  OptimizerPointer->SetTolerance(m_Controls.m_ToleranceSPSA->text().toFloat());
  OptimizerPointer->SetStateOfConvergenceDecayRate(m_Controls.m_StateOfConvergenceDecayRateSPSA->text().toFloat());
  OptimizerPointer->SetMinimumNumberOfIterations(m_Controls.m_MinimumNumberOfIterationsSPSA->text().toInt());
  OptimizerPointer->SetNumberOfPerturbations(m_Controls.m_NumberOfPerturbationsSPSA->text().toInt());
  OptimizerPointer->SetMaximumNumberOfIterations(m_Controls.m_IterationsSPSA->text().toInt());
  return OptimizerPointer.GetPointer();
}

itk::Array<double> QmitkSPSAOptimizerView::GetOptimizerParameters()
{
  itk::Array<double> optimizerValues;
  optimizerValues.SetSize(3);
  optimizerValues.fill(0);
  optimizerValues[0] = m_Controls.m_Maximize->isChecked();
  optimizerValues[1] = m_Controls.m_aSPSA->text().toFloat();
  optimizerValues[2] = m_Controls.m_ASPSA->text().toFloat();
  optimizerValues[3] = m_Controls.m_AlphaSPSA->text().toFloat();
  optimizerValues[4] = m_Controls.m_cSPSA->text().toFloat();
  optimizerValues[5] = m_Controls.m_GammaSPSA->text().toFloat();
  optimizerValues[6] = m_Controls.m_ToleranceSPSA->text().toFloat();
  optimizerValues[7] = m_Controls.m_StateOfConvergenceDecayRateSPSA->text().toFloat();
  optimizerValues[8] = m_Controls.m_MinimumNumberOfIterationsSPSA->text().toInt();
  optimizerValues[9] = m_Controls.m_NumberOfPerturbationsSPSA->text().toInt();
  optimizerValues[10] = m_Controls.m_IterationsSPSA->text().toInt();
  return optimizerValues;
}

void QmitkSPSAOptimizerView::SetOptimizerParameters(itk::Array<double> optimizerValues)
{
  m_Controls.m_Maximize->setChecked(optimizerValues[0]);
  m_Controls.m_aSPSA->setText(QString::number(optimizerValues[1]));
  m_Controls.m_ASPSA->setText(QString::number(optimizerValues[2]));
  m_Controls.m_AlphaSPSA->setText(QString::number(optimizerValues[3]));
  m_Controls.m_cSPSA->setText(QString::number(optimizerValues[4]));
  m_Controls.m_GammaSPSA->setText(QString::number(optimizerValues[5]));
  m_Controls.m_ToleranceSPSA->setText(QString::number(optimizerValues[6]));
  m_Controls.m_StateOfConvergenceDecayRateSPSA->setText(QString::number(optimizerValues[7]));
  m_Controls.m_MinimumNumberOfIterationsSPSA->setText(QString::number(optimizerValues[8]));
  m_Controls.m_NumberOfPerturbationsSPSA->setText(QString::number(optimizerValues[9]));
  m_Controls.m_IterationsSPSA->setText(QString::number(optimizerValues[10]));
}

void QmitkSPSAOptimizerView::SetNumberOfTransformParameters(int transformParameters)
{
  m_NumberTransformParameters = transformParameters;
}

QString QmitkSPSAOptimizerView::GetName()
{
  return "SPSA";
}

void QmitkSPSAOptimizerView::SetupUI(QWidget* parent)
{
  m_Controls.setupUi(parent);
  QValidator* validatorLineEditInput = new QIntValidator(0, 20000000, this);
  m_Controls.m_MinimumNumberOfIterationsSPSA->setValidator(validatorLineEditInput);
  m_Controls.m_NumberOfPerturbationsSPSA->setValidator(validatorLineEditInput);
  m_Controls.m_IterationsSPSA->setValidator(validatorLineEditInput);
  QValidator* validatorLineEditInputFloat = new QDoubleValidator(0, 20000000, 8, this);
  m_Controls.m_aSPSA->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ASPSA->setValidator(validatorLineEditInputFloat);
  m_Controls.m_AlphaSPSA->setValidator(validatorLineEditInputFloat);
  m_Controls.m_cSPSA->setValidator(validatorLineEditInputFloat);
  m_Controls.m_GammaSPSA->setValidator(validatorLineEditInputFloat);
  m_Controls.m_ToleranceSPSA->setValidator(validatorLineEditInputFloat);
  m_Controls.m_StateOfConvergenceDecayRateSPSA->setValidator(validatorLineEditInputFloat);
}
