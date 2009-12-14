/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile: mitkPropertyManager.cpp,v $
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

#ifndef QmitkRigidRegistrationMetricsGUIBaseH
#define QmitkRigidRegistrationMetricsGUIBaseH

#include <qwidget>
#include <itkArray.h>
#include <itkObject.h>
#include "mitkImage.h"
#include <qstring>

/*!
* \brief Widget for rigid registration
*
* Displays options for rigid registration.
*/
class QMITKEXT_EXPORT QmitkRigidRegistrationMetricsGUIBase : public QWidget
{

public:

  QmitkRigidRegistrationMetricsGUIBase(QWidget* parent = 0, Qt::WindowFlags f = 0);
  ~QmitkRigidRegistrationMetricsGUIBase();

  virtual itk::Object::Pointer GetMetric() = 0;

  virtual itk::Array<double> GetMetricParameters() = 0;

  virtual void SetMetricParameters(itk::Array<double> metricValues) = 0;

  void SetMovingImage(mitk::Image::Pointer movingImage);

  virtual QString GetName() = 0;

  virtual void SetupUI(QWidget* parent) = 0;
    

protected:

  mitk::Image::Pointer m_MovingImage;

};

#endif //QmitkRigidRegistrationMetricsGUIBaseH
