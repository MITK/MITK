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

#include "QmitkGradientDescentOptimizerView.h"
#include "mitkImageAccessByItk.h"
#include <itkGradientDescentOptimizer.h>

QmitkGradientDescentOptimizerView::QmitkGradientDescentOptimizerView(QWidget* parent, Qt::WindowFlags f ) : QmitkRigidRegistrationOptimizerGUIBase (parent, f),
m_NumberTransformParameters(16)
{

}

QmitkGradientDescentOptimizerView::~QmitkGradientDescentOptimizerView()
{
}

itk::Object::Pointer QmitkGradientDescentOptimizerView::GetOptimizer()
{
  itk::GradientDescentOptimizer::Pointer OptimizerPointer = itk::GradientDescentOptimizer::New();
  OptimizerPointer->SetMaximize( m_Controls.m_Maximize->isChecked());
  OptimizerPointer->SetLearningRate(m_Controls.m_LearningRateGradientDescent->text().toFloat());
  OptimizerPointer->SetNumberOfIterations( m_Controls.m_IterationsGradientDescent->text().toInt() );
  return OptimizerPointer.GetPointer();
}

itk::Array<double> QmitkGradientDescentOptimizerView::GetOptimizerParameters()
{
  itk::Array<double> optimizerValues;
  optimizerValues.SetSize(3);
  optimizerValues.fill(0);
  optimizerValues[0] = m_Controls.m_Maximize->isChecked();
  optimizerValues[1] = m_Controls.m_LearningRateGradientDescent->text().toFloat();
  optimizerValues[2] = m_Controls.m_IterationsGradientDescent->text().toInt();
  return optimizerValues;
}

void QmitkGradientDescentOptimizerView::SetOptimizerParameters(itk::Array<double> optimizerValues)
{
  m_Controls.m_Maximize->setChecked(optimizerValues[0]);
  m_Controls.m_LearningRateGradientDescent->setText(QString::number(optimizerValues[1]));
  m_Controls.m_IterationsGradientDescent->setText(QString::number(optimizerValues[2]));
}

void QmitkGradientDescentOptimizerView::SetNumberOfTransformParameters(int transformParameters)
{
  m_NumberTransformParameters = transformParameters;
}

QString QmitkGradientDescentOptimizerView::GetName()
{
  return "GradientDescent";
}

void QmitkGradientDescentOptimizerView::SetupUI(QWidget* parent)
{
  m_Controls.setupUi(parent);
  QValidator* validatorLineEditInput = new QIntValidator(0, 20000000, this);
  m_Controls.m_IterationsGradientDescent->setValidator(validatorLineEditInput);
  QValidator* validatorLineEditInputFloat = new QDoubleValidator(0, 20000000, 8, this);
  m_Controls.m_LearningRateGradientDescent->setValidator(validatorLineEditInputFloat);
}
