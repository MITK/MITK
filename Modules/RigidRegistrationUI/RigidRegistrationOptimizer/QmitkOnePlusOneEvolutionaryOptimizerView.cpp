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

#include "QmitkOnePlusOneEvolutionaryOptimizerView.h"
#include "mitkImageAccessByItk.h"
#include <itkOnePlusOneEvolutionaryOptimizer.h>
#include <itkNormalVariateGenerator.h>

QmitkOnePlusOneEvolutionaryOptimizerView::QmitkOnePlusOneEvolutionaryOptimizerView(QWidget* parent, Qt::WindowFlags f ) : QmitkRigidRegistrationOptimizerGUIBase (parent, f),
m_NumberTransformParameters(16)
{

}

QmitkOnePlusOneEvolutionaryOptimizerView::~QmitkOnePlusOneEvolutionaryOptimizerView()
{
}

itk::Object::Pointer QmitkOnePlusOneEvolutionaryOptimizerView::GetOptimizer()
{
  itk::OnePlusOneEvolutionaryOptimizer::Pointer OptimizerPointer = itk::OnePlusOneEvolutionaryOptimizer::New();
  OptimizerPointer->SetMaximize( m_Controls.m_Maximize->isChecked() );
  itk::Statistics::NormalVariateGenerator::Pointer generator = itk::Statistics::NormalVariateGenerator::New();
  generator->Initialize(12345);
  OptimizerPointer->SetNormalVariateGenerator(generator); 
  OptimizerPointer->SetShrinkFactor(m_Controls.m_ShrinkFactorOnePlusOneEvolutionary->text().toFloat());
  OptimizerPointer->SetGrowthFactor(m_Controls.m_GrowthFactorOnePlusOneEvolutionary->text().toFloat());
  OptimizerPointer->SetEpsilon(m_Controls.m_EpsilonOnePlusOneEvolutionary->text().toFloat());
  OptimizerPointer->SetInitialRadius(m_Controls.m_InitialRadiusOnePlusOneEvolutionary->text().toFloat());
  OptimizerPointer->SetMaximumIteration(m_Controls.m_IterationsOnePlusOneEvolutionary->text().toInt());
  return OptimizerPointer.GetPointer();
}

itk::Array<double> QmitkOnePlusOneEvolutionaryOptimizerView::GetOptimizerParameters()
{
  itk::Array<double> optimizerValues;
  optimizerValues.SetSize(6);
  optimizerValues.fill(0);
  optimizerValues[0] = m_Controls.m_Maximize->isChecked();
  optimizerValues[1] = m_Controls.m_ShrinkFactorOnePlusOneEvolutionary->text().toFloat();
  optimizerValues[2] = m_Controls.m_GrowthFactorOnePlusOneEvolutionary->text().toFloat();
  optimizerValues[3] = m_Controls.m_EpsilonOnePlusOneEvolutionary->text().toFloat();
  optimizerValues[4] = m_Controls.m_InitialRadiusOnePlusOneEvolutionary->text().toFloat();
  optimizerValues[5] = m_Controls.m_IterationsOnePlusOneEvolutionary->text().toInt();
  return optimizerValues;
}

void QmitkOnePlusOneEvolutionaryOptimizerView::SetOptimizerParameters(itk::Array<double> optimizerValues)
{
  m_Controls.m_Maximize->setChecked(optimizerValues[0]);
  m_Controls.m_ShrinkFactorOnePlusOneEvolutionary->setText(QString::number(optimizerValues[1]));
  m_Controls.m_GrowthFactorOnePlusOneEvolutionary->setText(QString::number(optimizerValues[2]));
  m_Controls.m_EpsilonOnePlusOneEvolutionary->setText(QString::number(optimizerValues[3]));
  m_Controls.m_InitialRadiusOnePlusOneEvolutionary->setText(QString::number(optimizerValues[4]));
  m_Controls.m_IterationsOnePlusOneEvolutionary->setText(QString::number(optimizerValues[5]));
}

void QmitkOnePlusOneEvolutionaryOptimizerView::SetNumberOfTransformParameters(int transformParameters)
{
  m_NumberTransformParameters = transformParameters;
}

QString QmitkOnePlusOneEvolutionaryOptimizerView::GetName()
{
  return "OnePlusOneEvolutionary";
}

void QmitkOnePlusOneEvolutionaryOptimizerView::SetupUI(QWidget* parent)
{
  m_Controls.setupUi(parent);
  QValidator* validatorLineEditInput = new QIntValidator(0, 20000000, this);
  m_Controls.m_EpsilonOnePlusOneEvolutionary->setValidator(validatorLineEditInput);
  m_Controls.m_InitialRadiusOnePlusOneEvolutionary->setValidator(validatorLineEditInput);
  m_Controls.m_IterationsOnePlusOneEvolutionary->setValidator(validatorLineEditInput);
  QValidator* validatorLineEditInputFloat = new QDoubleValidator(0, 20000000, 8, this);
  m_Controls.m_ShrinkFactorOnePlusOneEvolutionary->setValidator(validatorLineEditInputFloat);
  m_Controls.m_GrowthFactorOnePlusOneEvolutionary->setValidator(validatorLineEditInputFloat);
  m_Controls.m_EpsilonOnePlusOneEvolutionary->setValidator(validatorLineEditInputFloat);
  m_Controls.m_InitialRadiusOnePlusOneEvolutionary->setValidator(validatorLineEditInputFloat);
  m_Controls.m_IterationsOnePlusOneEvolutionary->setValidator(validatorLineEditInputFloat);
}
