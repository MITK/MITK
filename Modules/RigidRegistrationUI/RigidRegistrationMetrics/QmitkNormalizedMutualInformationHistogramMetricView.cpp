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

#include "QmitkNormalizedMutualInformationHistogramMetricView.h"
#include <itkNormalizedMutualInformationHistogramImageToImageMetric.h>
#include "mitkImageAccessByItk.h"

#include "QValidator"

QmitkNormalizedMutualInformationHistogramMetricView::QmitkNormalizedMutualInformationHistogramMetricView(QWidget* parent, Qt::WindowFlags f ) : QmitkRigidRegistrationMetricsGUIBase (parent, f)
{
}

QmitkNormalizedMutualInformationHistogramMetricView::~QmitkNormalizedMutualInformationHistogramMetricView()
{
}

mitk::MetricParameters::MetricType QmitkNormalizedMutualInformationHistogramMetricView::GetMetricType()
{
  return mitk::MetricParameters::NORMALIZEDMUTUALINFORMATIONHISTOGRAMIMAGETOIMAGEMETRIC;
}

itk::Object::Pointer QmitkNormalizedMutualInformationHistogramMetricView::GetMetric()
{
  if (m_MovingImage.IsNotNull())
  {
    AccessByItk(m_MovingImage, GetMetric2);
    return m_MetricObject;
  }
  return nullptr;
}

template < class TPixelType, unsigned int VImageDimension >
itk::Object::Pointer QmitkNormalizedMutualInformationHistogramMetricView::GetMetric2(itk::Image<TPixelType, VImageDimension>* /*itkImage1*/)
{
  typedef typename itk::Image< TPixelType, VImageDimension >  FixedImageType;
  typedef typename itk::Image< TPixelType, VImageDimension >  MovingImageType;
  typename itk::NormalizedMutualInformationHistogramImageToImageMetric<FixedImageType, MovingImageType>::Pointer MetricPointer = itk::NormalizedMutualInformationHistogramImageToImageMetric<FixedImageType, MovingImageType>::New();
  unsigned int nBins = m_Controls.m_NumberOfHistogramBinsNormalizedMutualInformationHistogram->text().toInt();
  typename itk::NormalizedMutualInformationHistogramImageToImageMetric<FixedImageType, MovingImageType>::HistogramType::SizeType histogramSize;
  histogramSize[0] = nBins;
  histogramSize[1] = nBins;
  MetricPointer->SetHistogramSize(histogramSize);
  MetricPointer->SetComputeGradient(m_Controls.m_ComputeGradient->isChecked());
  m_MetricObject = MetricPointer.GetPointer();
  return MetricPointer.GetPointer();
}

itk::Array<double> QmitkNormalizedMutualInformationHistogramMetricView::GetMetricParameters()
{
  itk::Array<double> metricValues;
  metricValues.SetSize(2);
  metricValues.fill(0);
  metricValues[0] = m_Controls.m_ComputeGradient->isChecked();
  metricValues[1] = m_Controls.m_NumberOfHistogramBinsNormalizedMutualInformationHistogram->text().toInt();
  return metricValues;
}

void QmitkNormalizedMutualInformationHistogramMetricView::SetMetricParameters(itk::Array<double> metricValues)
{
  m_Controls.m_ComputeGradient->setChecked(metricValues[0]);
  m_Controls.m_NumberOfHistogramBinsNormalizedMutualInformationHistogram->setText(QString::number(metricValues[1]));
}

QString QmitkNormalizedMutualInformationHistogramMetricView::GetName()
{
  return "NormalizedMutualInformationHistogram";
}

void QmitkNormalizedMutualInformationHistogramMetricView::SetupUI(QWidget* parent)
{
  m_Controls.setupUi(parent);
  QValidator* validatorLineEditInput = new QIntValidator(0, 20000000, this);
  m_Controls.m_NumberOfHistogramBinsNormalizedMutualInformationHistogram->setValidator(validatorLineEditInput);
}

bool QmitkNormalizedMutualInformationHistogramMetricView::Maximize()
{
  return true;
}
