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

#include "QmitkMeanSquaresMetricView.h"
#include <itkMeanSquaresImageToImageMetric.h>
#include "mitkImageAccessByItk.h"

QmitkMeanSquaresMetricView::QmitkMeanSquaresMetricView(QWidget* parent, Qt::WindowFlags f ) : QmitkRigidRegistrationMetricsGUIBase (parent, f)
{

}

QmitkMeanSquaresMetricView::~QmitkMeanSquaresMetricView()
{
}

mitk::MetricParameters::MetricType QmitkMeanSquaresMetricView::GetMetricType()
{
  return mitk::MetricParameters::MEANSQUARESIMAGETOIMAGEMETRIC;
}

itk::Object::Pointer QmitkMeanSquaresMetricView::GetMetric()
{
  if (m_MovingImage.IsNotNull())
  {
    AccessByItk(m_MovingImage, GetMetric2);
    return m_MetricObject;
  }
  return nullptr;
}

template < class TPixelType, unsigned int VImageDimension >
itk::Object::Pointer QmitkMeanSquaresMetricView::GetMetric2(itk::Image<TPixelType, VImageDimension>* /*itkImage1*/)
{
  typedef typename itk::Image< TPixelType, VImageDimension >  FixedImageType;
  typedef typename itk::Image< TPixelType, VImageDimension >  MovingImageType;
  typename itk::MeanSquaresImageToImageMetric<FixedImageType, MovingImageType>::Pointer MetricPointer = itk::MeanSquaresImageToImageMetric<FixedImageType, MovingImageType>::New();
  MetricPointer->SetComputeGradient(m_Controls.m_ComputeGradient->isChecked());
  m_MetricObject = MetricPointer.GetPointer();
  return MetricPointer.GetPointer();
}

itk::Array<double> QmitkMeanSquaresMetricView::GetMetricParameters()
{
  itk::Array<double> metricValues;
  metricValues.SetSize(1);
  metricValues.fill(0);
  metricValues[0] = m_Controls.m_ComputeGradient->isChecked();
  return metricValues;
}

void QmitkMeanSquaresMetricView::SetMetricParameters(itk::Array<double> metricValues)
{
  m_Controls.m_ComputeGradient->setChecked(metricValues[0]);
}

QString QmitkMeanSquaresMetricView::GetName()
{
  return "MeanSquares";
}

void QmitkMeanSquaresMetricView::SetupUI(QWidget* parent)
{
  m_Controls.setupUi(parent);
}

bool QmitkMeanSquaresMetricView::Maximize()
{
  return false;
}
