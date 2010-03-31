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

#ifndef QmitkMutualInformationHistogramMetricViewWidgetHIncluded
#define QmitkMutualInformationHistogramMetricViewWidgetHIncluded

#include "ui_QmitkMutualInformationHistogramMetricControls.h"
#include "MitkRigidRegistrationUIExports.h"
#include "QmitkRigidRegistrationMetricsGUIBase.h"
#include <itkArray.h>
#include <itkObject.h>
#include <itkImage.h>

/*!
* \brief Widget for rigid registration
*
* Displays options for rigid registration.
*/
class MITK_RIGIDREGISTRATION_UI_EXPORT QmitkMutualInformationHistogramMetricView : public QmitkRigidRegistrationMetricsGUIBase
{

public:

  QmitkMutualInformationHistogramMetricView( QWidget* parent = 0, Qt::WindowFlags f = 0 );
  ~QmitkMutualInformationHistogramMetricView();


  virtual itk::Object::Pointer GetMetric();

  virtual itk::Array<double> GetMetricParameters();

  virtual void SetMetricParameters(itk::Array<double> metricValues);

  virtual QString GetName();

  virtual void SetupUI(QWidget* parent);

private:

  template < class TPixelType, unsigned int VImageDimension >
  itk::Object::Pointer GetMetric2(itk::Image<TPixelType, VImageDimension>* itkImage1);

protected:

  Ui::QmitkMutualInformationHistogramMetricControls m_Controls;

  itk::Object::Pointer m_MetricObject;

};

#endif
