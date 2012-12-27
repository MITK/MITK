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

#include "QmitkNormalizedCorrelationMetricView.h"
#include <itkNormalizedCorrelationImageToImageMetric.h>
#include "mitkImageAccessByItk.h"

#include "QValidator"

QmitkNormalizedCorrelationMetricView::QmitkNormalizedCorrelationMetricView(QWidget* parent, Qt::WindowFlags f ) : QmitkRigidRegistrationMetricsGUIBase (parent, f)
{
}

QmitkNormalizedCorrelationMetricView::~QmitkNormalizedCorrelationMetricView()
{
}

mitk::MetricParameters::MetricType QmitkNormalizedCorrelationMetricView::GetMetricType()
{
  return mitk::MetricParameters::NORMALIZEDCORRELATIONIMAGETOIMAGEMETRIC;
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
itk::Object::Pointer QmitkNormalizedCorrelationMetricView::GetMetric2(itk::Image<TPixelType, VImageDimension>* /*itkImage1*/)
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

bool QmitkNormalizedCorrelationMetricView::Maximize()
{
  return true;
}
