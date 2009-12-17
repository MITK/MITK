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

#ifndef QmitkKullbackLeiblerCompareHistogramMetricViewWidgetHIncluded
#define QmitkKullbackLeiblerCompareHistogramMetricViewWidgetHIncluded

#include "ui_QmitkKullbackLeiblerCompareHistogramMetricControls.h"
#include "QmitkRigidRegistrationMetricsGUIBase.h"
#include <itkArray.h>
#include <itkObject.h>
#include <itkImage.h>

/*!
* \brief Widget for rigid registration
*
* Displays options for rigid registration.
*/
class QMITKEXT_EXPORT QmitkKullbackLeiblerCompareHistogramMetricView : public QmitkRigidRegistrationMetricsGUIBase
{

public:

  QmitkKullbackLeiblerCompareHistogramMetricView( QWidget* parent = 0, Qt::WindowFlags f = 0 );
  ~QmitkKullbackLeiblerCompareHistogramMetricView();


  virtual itk::Object::Pointer GetMetric();

  virtual itk::Array<double> GetMetricParameters();

  virtual void SetMetricParameters(itk::Array<double> metricValues);

  virtual QString GetName();

  virtual void SetupUI(QWidget* parent);

private:

  template < class TPixelType, unsigned int VImageDimension >
  itk::Object::Pointer GetMetric2(itk::Image<TPixelType, VImageDimension>* itkImage1);

protected:

  Ui::QmitkKullbackLeiblerCompareHistogramMetricControls m_Controls;

  itk::Object::Pointer m_MetricObject;

};

#endif
