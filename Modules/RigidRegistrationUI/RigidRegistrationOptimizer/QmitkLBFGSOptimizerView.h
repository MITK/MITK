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

#ifndef QmitkLBFGSOptimizerViewWidgetHIncluded
#define QmitkLBFGSOptimizerViewWidgetHIncluded

#include "ui_QmitkLBFGSOptimizerControls.h"
#include "MitkRigidRegistrationUIExports.h"
#include "QmitkRigidRegistrationOptimizerGUIBase.h"
#include <itkArray.h>
#include <itkObject.h>
#include <itkImage.h>

/*!
* \brief Widget for rigid registration
*
* Displays options for rigid registration.
*/
class MITKRIGIDREGISTRATIONUI_EXPORT QmitkLBFGSOptimizerView : public QmitkRigidRegistrationOptimizerGUIBase
{

public:

  QmitkLBFGSOptimizerView( QWidget* parent = nullptr, Qt::WindowFlags f = nullptr );
  ~QmitkLBFGSOptimizerView();

  virtual mitk::OptimizerParameters::OptimizerType GetOptimizerType() override;

  virtual itk::Object::Pointer GetOptimizer() override;

  virtual itk::Array<double> GetOptimizerParameters() override;

  virtual void SetOptimizerParameters(itk::Array<double> metricValues) override;

  virtual void SetNumberOfTransformParameters(int transformParameters) override;

  virtual QString GetName() override;

  virtual void SetupUI(QWidget* parent) override;

protected:

  Ui::QmitkLBFGSOptimizerControls m_Controls;

  int m_NumberTransformParameters;

};

#endif
