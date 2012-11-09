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

#ifndef QmitkRigidRegistrationOptimizerGUIBaseH
#define QmitkRigidRegistrationOptimizerGUIBaseH

#include <QWidget>
#include "MitkRigidRegistrationUIExports.h"
#include <itkArray.h>
#include <itkObject.h>
#include "mitkImage.h"
#include <QString>
#include "mitkOptimizerParameters.h"

/*!
* \brief Widget for rigid registration
*
* Displays options for rigid registration.
*/
class MITK_RIGIDREGISTRATION_UI_EXPORT QmitkRigidRegistrationOptimizerGUIBase : public QWidget
{

public:

  QmitkRigidRegistrationOptimizerGUIBase(QWidget* parent = 0, Qt::WindowFlags f = 0);
  ~QmitkRigidRegistrationOptimizerGUIBase();

  virtual mitk::OptimizerParameters::OptimizerType GetOptimizerType() = 0;

  virtual itk::Object::Pointer GetOptimizer() = 0;

  virtual itk::Array<double> GetOptimizerParameters() = 0;

  virtual void SetOptimizerParameters(itk::Array<double> optimizerValues) = 0;

  virtual void SetNumberOfTransformParameters(int transformParameters) = 0;

  virtual QString GetName() = 0;

  virtual void SetupUI(QWidget* parent) = 0;


protected:

};

#endif //QmitkRigidRegistrationOptimizerGUIBaseH
