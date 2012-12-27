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

#include "QmitkGradientDifferenceMetricView.h"
#include <itkGradientDifferenceImageToImageMetric.h>
#include "mitkImageAccessByItk.h"

#include "QValidator"

QmitkGradientDifferenceMetricView::QmitkGradientDifferenceMetricView(QWidget* parent, Qt::WindowFlags f ) : QmitkRigidRegistrationMetricsGUIBase (parent, f)
{
}

QmitkGradientDifferenceMetricView::~QmitkGradientDifferenceMetricView()
{
}

mitk::MetricParameters::MetricType QmitkGradientDifferenceMetricView::GetMetricType()
{
  return mitk::MetricParameters::GRADIENTDIFFERENCEIMAGETOIMAGEMETRIC;
}

itk::Object::Pointer QmitkGradientDifferenceMetricView::GetMetric()
{
  if (m_MovingImage.IsNotNull())
  {
    AccessByItk(m_MovingImage, GetMetric2);
    return m_MetricObject;
  }
  return NULL;
}

template < class TPixelType, unsigned int VImageDimension >
itk::Object::Pointer QmitkGradientDifferenceMetricView::GetMetric2(itk::Image<TPixelType, VImageDimension>* /*itkImage1*/)
{
  typedef typename itk::Image< TPixelType, VImageDimension >  FixedImageType;
  typedef typename itk::Image< TPixelType, VImageDimension >  MovingImageType;
  typename itk::GradientDifferenceImageToImageMetric<FixedImageType, MovingImageType>::Pointer MetricPointer = itk::GradientDifferenceImageToImageMetric<FixedImageType, MovingImageType>::New();
  MetricPointer->SetComputeGradient(m_Controls.m_ComputeGradient->isChecked());
  m_MetricObject = MetricPointer.GetPointer();
  return MetricPointer.GetPointer();
}

itk::Array<double> QmitkGradientDifferenceMetricView::GetMetricParameters()
{
  itk::Array<double> metricValues;
  metricValues.SetSize(1);
  metricValues.fill(0);
  metricValues[0] = m_Controls.m_ComputeGradient->isChecked();
  return metricValues;
}

void QmitkGradientDifferenceMetricView::SetMetricParameters(itk::Array<double> metricValues)
{
  m_Controls.m_ComputeGradient->setChecked(metricValues[0]);
}

QString QmitkGradientDifferenceMetricView::GetName()
{
  return "GradientDifference";
}

void QmitkGradientDifferenceMetricView::SetupUI(QWidget* parent)
{
  m_Controls.setupUi(parent);
}

bool QmitkGradientDifferenceMetricView::Maximize()
{
  return false;
}
