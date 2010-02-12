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

#include "QmitkMutualInformationHistogramMetricView.h"
#include <itkMutualInformationHistogramImageToImageMetric.h>
#include "mitkImageAccessByItk.h"

#include "QValidator"

QmitkMutualInformationHistogramMetricView::QmitkMutualInformationHistogramMetricView(QWidget* parent, Qt::WindowFlags f ) : QmitkRigidRegistrationMetricsGUIBase (parent, f)
{
}

QmitkMutualInformationHistogramMetricView::~QmitkMutualInformationHistogramMetricView()
{
}

itk::Object::Pointer QmitkMutualInformationHistogramMetricView::GetMetric()
{
  if (m_MovingImage.IsNotNull())
  {
    AccessByItk(m_MovingImage, GetMetric2);
    return m_MetricObject;
  }
  return NULL;
}

template < class TPixelType, unsigned int VImageDimension >
itk::Object::Pointer QmitkMutualInformationHistogramMetricView::GetMetric2(itk::Image<TPixelType, VImageDimension>* itkImage1)
{
  typedef typename itk::Image< TPixelType, VImageDimension >  FixedImageType;
  typedef typename itk::Image< TPixelType, VImageDimension >  MovingImageType;
  typename itk::MutualInformationHistogramImageToImageMetric<FixedImageType, MovingImageType>::Pointer MetricPointer = itk::MutualInformationHistogramImageToImageMetric<FixedImageType, MovingImageType>::New();
  unsigned int nBins = m_Controls.m_NumberOfHistogramBinsMutualInformationHistogram->text().toInt();
  typename itk::MutualInformationHistogramImageToImageMetric<FixedImageType, MovingImageType>::HistogramType::SizeType histogramSize;
  histogramSize[0] = nBins;
  histogramSize[1] = nBins;
  MetricPointer->SetHistogramSize(histogramSize);
  MetricPointer->SetComputeGradient(m_Controls.m_ComputeGradient->isChecked());
  m_MetricObject = MetricPointer.GetPointer();
  return MetricPointer.GetPointer();
}

itk::Array<double> QmitkMutualInformationHistogramMetricView::GetMetricParameters()
{
  itk::Array<double> metricValues;
  metricValues.SetSize(2);
  metricValues.fill(0);
  metricValues[0] = m_Controls.m_ComputeGradient->isChecked();
  metricValues[1] = m_Controls.m_NumberOfHistogramBinsMutualInformationHistogram->text().toInt();
  return metricValues;
}

void QmitkMutualInformationHistogramMetricView::SetMetricParameters(itk::Array<double> metricValues)
{
  m_Controls.m_ComputeGradient->setChecked(metricValues[0]);
  m_Controls.m_NumberOfHistogramBinsMutualInformationHistogram->setText(QString::number(metricValues[1]));
}

QString QmitkMutualInformationHistogramMetricView::GetName()
{
  return "MutualInformationHistogram";
}

void QmitkMutualInformationHistogramMetricView::SetupUI(QWidget* parent)
{
  m_Controls.setupUi(parent);
  QValidator* validatorLineEditInput = new QIntValidator(0, 20000000, this);
  m_Controls.m_NumberOfHistogramBinsMutualInformationHistogram->setValidator(validatorLineEditInput);
}
