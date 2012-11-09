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

#ifndef QmitkVersorRigid3DOptimizerViewWidgetHIncluded
#define QmitkVersorRigid3DOptimizerViewWidgetHIncluded

#include "ui_QmitkVersorRigid3DTransformOptimizerControls.h"
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
class MITK_RIGIDREGISTRATION_UI_EXPORT QmitkVersorRigid3DTransformOptimizerView : public QmitkRigidRegistrationOptimizerGUIBase
{

public:

  QmitkVersorRigid3DTransformOptimizerView( QWidget* parent = 0, Qt::WindowFlags f = 0 );
  ~QmitkVersorRigid3DTransformOptimizerView();

  virtual mitk::OptimizerParameters::OptimizerType GetOptimizerType();

  virtual itk::Object::Pointer GetOptimizer();

  virtual itk::Array<double> GetOptimizerParameters();

  virtual void SetOptimizerParameters(itk::Array<double> metricValues);

  virtual void SetNumberOfTransformParameters(int transformParameters);

  virtual QString GetName();

  virtual void SetupUI(QWidget* parent);

protected:

  Ui::QmitkVersorRigid3DTransformOptimizerControls m_Controls;

  int m_NumberTransformParameters;

};

#endif
