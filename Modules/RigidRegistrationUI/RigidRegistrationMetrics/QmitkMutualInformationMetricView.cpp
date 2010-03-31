/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
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

#include "QmitkMutualInformationMetricView.h"
#include <itkMutualInformationImageToImageMetric.h>
#include "mitkImageAccessByItk.h"

QmitkMutualInformationMetricView::QmitkMutualInformationMetricView(QWidget* parent, Qt::WindowFlags f ) : QmitkRigidRegistrationMetricsGUIBase (parent, f)
{

}

QmitkMutualInformationMetricView::~QmitkMutualInformationMetricView()
{
}

itk::Object::Pointer QmitkMutualInformationMetricView::GetMetric()
{
  if (m_MovingImage.IsNotNull())
  {
    AccessByItk(m_MovingImage, GetMetric2);
    return m_MetricObject;
  }
  return NULL;
}

template < class TPixelType, unsigned int VImageDimension >
itk::Object::Pointer QmitkMutualInformationMetricView::GetMetric2(itk::Image<TPixelType, VImageDimension>* /*itkImage1*/)
{
  typedef typename itk::Image< TPixelType, VImageDimension >  FixedImageType;
  typedef typename itk::Image< TPixelType, VImageDimension >  MovingImageType;
  typename itk::MutualInformationImageToImageMetric<FixedImageType, MovingImageType>::Pointer MetricPointer = itk::MutualInformationImageToImageMetric<FixedImageType, MovingImageType>::New();
  MetricPointer->SetNumberOfSpatialSamples(m_Controls.m_NumberOfSpatialSamplesMutualInformation->text().toInt());
  MetricPointer->SetFixedImageStandardDeviation(m_Controls.m_FixedImageStandardDeviationMutualInformation->text().toFloat());
  MetricPointer->SetMovingImageStandardDeviation(m_Controls.m_MovingImageStandardDeviationMutualInformation->text().toFloat());
  MetricPointer->SetComputeGradient(m_Controls.m_ComputeGradient->isChecked());
  m_MetricObject = MetricPointer.GetPointer();
  return MetricPointer.GetPointer();
}

itk::Array<double> QmitkMutualInformationMetricView::GetMetricParameters()
{
  itk::Array<double> metricValues;
  metricValues.SetSize(4);
  metricValues.fill(0);
  metricValues[0] = m_Controls.m_ComputeGradient->isChecked();
  metricValues[1] = m_Controls.m_NumberOfSpatialSamplesMutualInformation->text().toInt();
  metricValues[2] = m_Controls.m_FixedImageStandardDeviationMutualInformation->text().toFloat();
  metricValues[3] = m_Controls.m_MovingImageStandardDeviationMutualInformation->text().toFloat();
  return metricValues;
}

void QmitkMutualInformationMetricView::SetMetricParameters(itk::Array<double> metricValues)
{
  m_Controls.m_ComputeGradient->setChecked(metricValues[0]);
  m_Controls.m_NumberOfSpatialSamplesMutualInformation->setText(QString::number(metricValues[1]));
  m_Controls.m_FixedImageStandardDeviationMutualInformation->setText(QString::number(metricValues[2]));
  m_Controls.m_MovingImageStandardDeviationMutualInformation->setText(QString::number(metricValues[3]));
}

QString QmitkMutualInformationMetricView::GetName()
{
  return "MutualInformation";
}

void QmitkMutualInformationMetricView::SetupUI(QWidget* parent)
{
  m_Controls.setupUi(parent);
  QValidator* validatorLineEditInput = new QIntValidator(0, 20000000, this);
  m_Controls.m_NumberOfSpatialSamplesMutualInformation->setValidator(validatorLineEditInput);
  QValidator* validatorLineEditInputFloat = new QDoubleValidator(0, 20000000, 8, this);
  m_Controls.m_FixedImageStandardDeviationMutualInformation->setValidator(validatorLineEditInputFloat);
  m_Controls.m_MovingImageStandardDeviationMutualInformation->setValidator(validatorLineEditInputFloat);
}
