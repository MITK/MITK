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

#include "QmitkMattesMutualInformationMetricView.h"
#include <itkMattesMutualInformationImageToImageMetric.h>
#include "mitkImageAccessByItk.h"

QmitkMattesMutualInformationMetricView::QmitkMattesMutualInformationMetricView(QWidget* parent, Qt::WindowFlags f ) : QmitkRigidRegistrationMetricsGUIBase (parent, f)
{

}

QmitkMattesMutualInformationMetricView::~QmitkMattesMutualInformationMetricView()
{
}

itk::Object::Pointer QmitkMattesMutualInformationMetricView::GetMetric()
{
  if (m_MovingImage.IsNotNull())
  {
    AccessByItk(m_MovingImage, GetMetric2);
    return m_MetricObject;
  }
  return NULL;
}

template < class TPixelType, unsigned int VImageDimension >
itk::Object::Pointer QmitkMattesMutualInformationMetricView::GetMetric2(itk::Image<TPixelType, VImageDimension>* /*itkImage1*/)
{
  typedef typename itk::Image< TPixelType, VImageDimension >  FixedImageType;
  typedef typename itk::Image< TPixelType, VImageDimension >  MovingImageType;
  typename itk::MattesMutualInformationImageToImageMetric<FixedImageType, MovingImageType>::Pointer MetricPointer = itk::MattesMutualInformationImageToImageMetric<FixedImageType, MovingImageType>::New();
  bool useSampling = m_Controls.m_UseSamplingMattesMutualInformation->isChecked();
  if( useSampling )
  {
    // set the number of samples to use
    MetricPointer->SetNumberOfSpatialSamples( m_Controls.m_NumberOfSpatialSamplesMattesMutualInformation->text().toInt() );
  }
  else
  {
    MetricPointer->UseAllPixelsOn();
  }
  MetricPointer->SetNumberOfHistogramBins(m_Controls.m_NumberOfHistogramBinsMattesMutualInformation->text().toInt());
  MetricPointer->ReinitializeSeed( 76926294 );
  MetricPointer->SetComputeGradient(m_Controls.m_ComputeGradient->isChecked());
  m_MetricObject = MetricPointer.GetPointer();
  return MetricPointer.GetPointer();
}

itk::Array<double> QmitkMattesMutualInformationMetricView::GetMetricParameters()
{
  itk::Array<double> metricValues;
  metricValues.SetSize(4);
  metricValues.fill(0);
  metricValues[0] = m_Controls.m_ComputeGradient->isChecked();
  metricValues[1] = m_Controls.m_UseSamplingMattesMutualInformation->isChecked();
  metricValues[2] = m_Controls.m_NumberOfSpatialSamplesMattesMutualInformation->text().toInt();
  metricValues[3] = m_Controls.m_NumberOfHistogramBinsMattesMutualInformation->text().toInt();
  return metricValues;
}

void QmitkMattesMutualInformationMetricView::SetMetricParameters(itk::Array<double> metricValues)
{
  m_Controls.m_ComputeGradient->setChecked(metricValues[0]);
  m_Controls.m_UseSamplingMattesMutualInformation->setChecked(metricValues[1]);
  m_Controls.m_NumberOfSpatialSamplesMattesMutualInformation->setText(QString::number(metricValues[2]));
  m_Controls.m_NumberOfHistogramBinsMattesMutualInformation->setText(QString::number(metricValues[3]));
}

QString QmitkMattesMutualInformationMetricView::GetName()
{
  return "MattesMutualInformation";
}

void QmitkMattesMutualInformationMetricView::SetupUI(QWidget* parent)
{
  m_Controls.setupUi(parent);
  QValidator* validatorLineEditInput = new QIntValidator(0, 20000000, this);
  m_Controls.m_NumberOfSpatialSamplesMattesMutualInformation->setValidator(validatorLineEditInput);
  m_Controls.m_NumberOfHistogramBinsMattesMutualInformation->setValidator(validatorLineEditInput);
}
