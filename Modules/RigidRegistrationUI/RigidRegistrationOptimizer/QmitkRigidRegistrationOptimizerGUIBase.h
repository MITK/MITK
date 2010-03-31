/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision: 1.12 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#ifndef QmitkRigidRegistrationOptimizerGUIBaseH
#define QmitkRigidRegistrationOptimizerGUIBaseH

#include <QWidget>
#include "MitkRigidRegistrationUIExports.h"
#include <itkArray.h>
#include <itkObject.h>
#include "mitkImage.h"
#include <QString>

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

  virtual itk::Object::Pointer GetOptimizer() = 0;

  virtual itk::Array<double> GetOptimizerParameters() = 0;

  virtual void SetOptimizerParameters(itk::Array<double> optimizerValues) = 0;

  virtual void SetNumberOfTransformParameters(int transformParameters) = 0;

  virtual QString GetName() = 0;

  virtual void SetupUI(QWidget* parent) = 0;
    

protected:

};

#endif //QmitkRigidRegistrationOptimizerGUIBaseH
