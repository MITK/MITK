/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date$
Version:   $Revision: -1 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkNormalizedCorrelationMetricView.h"
#include <itkNormalizedCorrelationImageToImageMetric.h>
#include "mitkImageAccessByItk.h"

#include "qvalidator.h"

QmitkNormalizedCorrelationMetricView::QmitkNormalizedCorrelationMetricView(QWidget* parent, Qt::WindowFlags f ) : QmitkRigidRegistrationMetricsGUIBase (parent, f)
{
}

QmitkNormalizedCorrelationMetricView::~QmitkNormalizedCorrelationMetricView()
{
}

itk::Object::Pointer QmitkNormalizedCorrelationMetricView::GetMetric()
{
  if (m_MovingImage.IsNotNull())
  {
    AccessByItk(m_MovingImage, GetMetric2);
    return m_MetricObject;
  }
  return NULL;
}

template < class TPixelType, unsigned int VImageDimension >
itk::Object::Pointer QmitkNormalizedCorrelationMetricView::GetMetric2(itk::Image<TPixelType, VImageDimension>* itkImage1)
{
  typedef typename itk::Image< TPixelType, VImageDimension >  FixedImageType;
  typedef typename itk::Image< TPixelType, VImageDimension >  MovingImageType;
  typename itk::NormalizedCorrelationImageToImageMetric<FixedImageType, MovingImageType>::Pointer MetricPointer = itk::NormalizedCorrelationImageToImageMetric<FixedImageType, MovingImageType>::New();
  MetricPointer->SetComputeGradient(m_Controls.m_ComputeGradient->isChecked());
  m_MetricObject = MetricPointer.GetPointer();
  return MetricPointer.GetPointer();
}

itk::Array<double> QmitkNormalizedCorrelationMetricView::GetMetricParameters()
{
  itk::Array<double> metricValues;
  metricValues.SetSize(1);
  metricValues.fill(0);
  metricValues[0] = m_Controls.m_ComputeGradient->isChecked();
  return metricValues;
}

void QmitkNormalizedCorrelationMetricView::SetMetricParameters(itk::Array<double> metricValues)
{
  m_Controls.m_ComputeGradient->setChecked(metricValues[0]);
}

QString QmitkNormalizedCorrelationMetricView::GetName()
{
  return "NormalizedCorrelation";
}

void QmitkNormalizedCorrelationMetricView::SetupUI(QWidget* parent)
{
  m_Controls.setupUi(parent);
}
