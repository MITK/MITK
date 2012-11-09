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

#ifndef QmitkRigidRegistrationMetricsGUIBaseH
#define QmitkRigidRegistrationMetricsGUIBaseH

#include <QWidget>
#include "MitkRigidRegistrationUIExports.h"
#include <itkArray.h>
#include <itkObject.h>
#include "mitkImage.h"
#include <QString>
#include "mitkMetricParameters.h"

/*!
* \brief Widget for rigid registration
*
* Displays options for rigid registration.
*/
class MITK_RIGIDREGISTRATION_UI_EXPORT QmitkRigidRegistrationMetricsGUIBase : public QWidget
{

public:

  QmitkRigidRegistrationMetricsGUIBase(QWidget* parent = 0, Qt::WindowFlags f = 0);
  ~QmitkRigidRegistrationMetricsGUIBase();

  virtual mitk::MetricParameters::MetricType GetMetricType() = 0;

  virtual itk::Object::Pointer GetMetric() = 0;

  virtual itk::Array<double> GetMetricParameters() = 0;

  virtual void SetMetricParameters(itk::Array<double> metricValues) = 0;

  void SetMovingImage(mitk::Image::Pointer movingImage);

  virtual QString GetName() = 0;

  virtual void SetupUI(QWidget* parent) = 0;

  virtual bool Maximize() = 0;

protected:

  mitk::Image::Pointer m_MovingImage;

};

#endif //QmitkRigidRegistrationMetricsGUIBaseH
