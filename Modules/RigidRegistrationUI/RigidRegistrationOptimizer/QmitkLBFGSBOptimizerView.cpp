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

#include "QmitkLBFGSBOptimizerView.h"
#include "mitkImageAccessByItk.h"
#include <itkLBFGSBOptimizer.h>

#include <QIntValidator>
#include <QDoubleValidator>

QmitkLBFGSBOptimizerView::QmitkLBFGSBOptimizerView(QWidget* parent, Qt::WindowFlags f ) : QmitkRigidRegistrationOptimizerGUIBase (parent, f),
m_NumberTransformParameters(16)
{

}

QmitkLBFGSBOptimizerView::~QmitkLBFGSBOptimizerView()
{
}

mitk::OptimizerParameters::OptimizerType QmitkLBFGSBOptimizerView::GetOptimizerType()
{
  return mitk::OptimizerParameters::LBFGSBOPTIMIZER;
}


itk::Object::Pointer QmitkLBFGSBOptimizerView::GetOptimizer()
{
  itk::LBFGSBOptimizer::Pointer OptimizerPointer = itk::LBFGSBOptimizer::New();
  // Set up boundary conditions
  itk::LBFGSBOptimizer::BoundValueType lower(12);
  itk::LBFGSBOptimizer::BoundValueType upper(12);
  itk::LBFGSBOptimizer::BoundSelectionType select(12);

  lower.Fill( -1 );
  upper.Fill( 10 );
  select.Fill( 2 );

  OptimizerPointer->SetLowerBound( lower );
  OptimizerPointer->SetUpperBound( upper );
  OptimizerPointer->SetBoundSelection( select );
  OptimizerPointer->SetCostFunctionConvergenceFactor(1e+1);
  OptimizerPointer->SetMaximumNumberOfCorrections(5);
  OptimizerPointer->SetProjectedGradientTolerance(1e-5);
  OptimizerPointer->SetMaximumNumberOfEvaluations(500);
  OptimizerPointer->SetMaximumNumberOfIterations( 200 );
  return OptimizerPointer.GetPointer();
}

itk::Array<double> QmitkLBFGSBOptimizerView::GetOptimizerParameters()
{
  itk::Array<double> optimizerValues;
  optimizerValues.SetSize(1);
  optimizerValues.fill(0);
  optimizerValues[0] = m_Controls.m_Maximize->isChecked();
  return optimizerValues;
}

void QmitkLBFGSBOptimizerView::SetOptimizerParameters(itk::Array<double> optimizerValues)
{
  m_Controls.m_Maximize->setChecked(optimizerValues[0]);
}

void QmitkLBFGSBOptimizerView::SetNumberOfTransformParameters(int transformParameters)
{
  m_NumberTransformParameters = transformParameters;
}

QString QmitkLBFGSBOptimizerView::GetName()
{
  return "LBFGSB";
}

void QmitkLBFGSBOptimizerView::SetupUI(QWidget* parent)
{
  m_Controls.setupUi(parent);
  /*QValidator* validatorLineEditInput = */ new QIntValidator(0, 20000000, this);
  /*QValidator* validatorLineEditInputFloat = */ new QDoubleValidator(0, 20000000, 8, this);
}
