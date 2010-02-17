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

#include "QmitkMeanSquaresHistogramMetricView.h"
#include <itkMeanSquaresHistogramImageToImageMetric.h>
#include "mitkImageAccessByItk.h"

#include "QValidator"

QmitkMeanSquaresHistogramMetricView::QmitkMeanSquaresHistogramMetricView(QWidget* parent, Qt::WindowFlags f ) : QmitkRigidRegistrationMetricsGUIBase (parent, f)
{
}

QmitkMeanSquaresHistogramMetricView::~QmitkMeanSquaresHistogramMetricView()
{
}

itk::Object::Pointer QmitkMeanSquaresHistogramMetricView::GetMetric()
{
 if (m_MovingImage.IsNotNull())
 {
   AccessByItk(m_MovingImage, GetMetric2);
   return m_MetricObject;
 }
 return NULL;
}

template < class TPixelType, unsigned int VImageDimension >
itk::Object::Pointer QmitkMeanSquaresHistogramMetricView::GetMetric2(itk::Image<TPixelType, VImageDimension>* /*itkImage1*/)
{
 typedef typename itk::Image< TPixelType, VImageDimension >  FixedImageType;
 typedef typename itk::Image< TPixelType, VImageDimension >  MovingImageType;
 typename itk::MeanSquaresHistogramImageToImageMetric<FixedImageType, MovingImageType>::Pointer MetricPointer = itk::MeanSquaresHistogramImageToImageMetric<FixedImageType, MovingImageType>::New();
 unsigned int nBins = m_Controls.m_NumberOfHistogramBinsMeanSquaresHistogram->text().toInt();
 typename itk::MeanSquaresHistogramImageToImageMetric<FixedImageType, MovingImageType>::HistogramType::SizeType histogramSize;
 histogramSize[0] = nBins;
 histogramSize[1] = nBins;
 MetricPointer->SetHistogramSize(histogramSize);
 MetricPointer->SetComputeGradient(m_Controls.m_ComputeGradient->isChecked());
 m_MetricObject = MetricPointer.GetPointer();
 return MetricPointer.GetPointer();
}

itk::Array<double> QmitkMeanSquaresHistogramMetricView::GetMetricParameters()
{
 itk::Array<double> metricValues;
 metricValues.SetSize(2);
 metricValues.fill(0);
 metricValues[0] = m_Controls.m_ComputeGradient->isChecked();
 metricValues[1] = m_Controls.m_NumberOfHistogramBinsMeanSquaresHistogram->text().toInt();
 return metricValues;
}

void QmitkMeanSquaresHistogramMetricView::SetMetricParameters(itk::Array<double> metricValues)
{
 m_Controls.m_ComputeGradient->setChecked(metricValues[0]);
 m_Controls.m_NumberOfHistogramBinsMeanSquaresHistogram->setText(QString::number(metricValues[1]));
}

QString QmitkMeanSquaresHistogramMetricView::GetName()
{
 return "MeanSquaresHistogram";
}

void QmitkMeanSquaresHistogramMetricView::SetupUI(QWidget* parent)
{
  m_Controls.setupUi(parent);
  QValidator* validatorLineEditInput = new QIntValidator(0, 20000000, this);
  m_Controls.m_NumberOfHistogramBinsMeanSquaresHistogram->setValidator(validatorLineEditInput);
}
