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

#include "QmitkQuaternionRigidTransformGradientDescentOptimizerView.h"
#include "mitkImageAccessByItk.h"
#include <itkQuaternionRigidTransformGradientDescentOptimizer.h>

QmitkQuaternionRigidTransformGradientDescentOptimizerView::QmitkQuaternionRigidTransformGradientDescentOptimizerView(QWidget* parent, Qt::WindowFlags f ) : QmitkRigidRegistrationOptimizerGUIBase (parent, f),
m_NumberTransformParameters(16)
{

}

QmitkQuaternionRigidTransformGradientDescentOptimizerView::~QmitkQuaternionRigidTransformGradientDescentOptimizerView()
{
}

mitk::OptimizerParameters::OptimizerType QmitkQuaternionRigidTransformGradientDescentOptimizerView::GetOptimizerType()
{
  return mitk::OptimizerParameters::QUATERNIONRIGIDTRANSFORMGRADIENTDESCENTOPTIMIZER;
}

itk::Object::Pointer QmitkQuaternionRigidTransformGradientDescentOptimizerView::GetOptimizer()
{
  itk::QuaternionRigidTransformGradientDescentOptimizer::Pointer OptimizerPointer = itk::QuaternionRigidTransformGradientDescentOptimizer::New();
  OptimizerPointer->SetMaximize( m_Controls.m_Maximize->isChecked());
  OptimizerPointer->SetLearningRate(m_Controls.m_LearningRateQuaternionRigidTransformGradientDescent->text().toFloat());
  OptimizerPointer->SetNumberOfIterations( m_Controls.m_IterationsQuaternionRigidTransformGradientDescent->text().toInt() );
  return OptimizerPointer.GetPointer();
}

itk::Array<double> QmitkQuaternionRigidTransformGradientDescentOptimizerView::GetOptimizerParameters()
{
  itk::Array<double> optimizerValues;
  optimizerValues.SetSize(3);
  optimizerValues.fill(0);
  optimizerValues[0] = m_Controls.m_Maximize->isChecked();
  optimizerValues[1] = m_Controls.m_LearningRateQuaternionRigidTransformGradientDescent->text().toFloat();
  optimizerValues[2] = m_Controls.m_IterationsQuaternionRigidTransformGradientDescent->text().toInt();
  return optimizerValues;
}

void QmitkQuaternionRigidTransformGradientDescentOptimizerView::SetOptimizerParameters(itk::Array<double> optimizerValues)
{
  m_Controls.m_Maximize->setChecked(optimizerValues[0]);
  m_Controls.m_LearningRateQuaternionRigidTransformGradientDescent->setText(QString::number(optimizerValues[1]));
  m_Controls.m_IterationsQuaternionRigidTransformGradientDescent->setText(QString::number(optimizerValues[2]));
}

void QmitkQuaternionRigidTransformGradientDescentOptimizerView::SetNumberOfTransformParameters(int transformParameters)
{
  m_NumberTransformParameters = transformParameters;
}

QString QmitkQuaternionRigidTransformGradientDescentOptimizerView::GetName()
{
  return "QuaternionRigidTransformGradientDescent";
}

void QmitkQuaternionRigidTransformGradientDescentOptimizerView::SetupUI(QWidget* parent)
{
  m_Controls.setupUi(parent);
  QValidator* validatorLineEditInput = new QIntValidator(0, 20000000, this);
  m_Controls.m_IterationsQuaternionRigidTransformGradientDescent->setValidator(validatorLineEditInput);
  QValidator* validatorLineEditInputFloat = new QDoubleValidator(0, 20000000, 8, this);
  m_Controls.m_LearningRateQuaternionRigidTransformGradientDescent->setValidator(validatorLineEditInputFloat);
}
