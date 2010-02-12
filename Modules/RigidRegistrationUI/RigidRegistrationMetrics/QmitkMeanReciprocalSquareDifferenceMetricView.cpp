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

#include "QmitkMeanReciprocalSquareDifferenceMetricView.h"
#include <itkMeanReciprocalSquareDifferenceImageToImageMetric.h>
#include "mitkImageAccessByItk.h"

#include "QValidator"

QmitkMeanReciprocalSquareDifferenceMetricView::QmitkMeanReciprocalSquareDifferenceMetricView(QWidget* parent, Qt::WindowFlags f ) : QmitkRigidRegistrationMetricsGUIBase (parent, f)
{
}

QmitkMeanReciprocalSquareDifferenceMetricView::~QmitkMeanReciprocalSquareDifferenceMetricView()
{
}

itk::Object::Pointer QmitkMeanReciprocalSquareDifferenceMetricView::GetMetric()
{
  if (m_MovingImage.IsNotNull())
  {
    AccessByItk(m_MovingImage, GetMetric2);
    return m_MetricObject;
  }
  return NULL;
}

template < class TPixelType, unsigned int VImageDimension >
itk::Object::Pointer QmitkMeanReciprocalSquareDifferenceMetricView::GetMetric2(itk::Image<TPixelType, VImageDimension>* itkImage1)
{
  typedef typename itk::Image< TPixelType, VImageDimension >  FixedImageType;
  typedef typename itk::Image< TPixelType, VImageDimension >  MovingImageType;
  typename itk::MeanReciprocalSquareDifferenceImageToImageMetric<FixedImageType, MovingImageType>::Pointer MetricPointer = itk::MeanReciprocalSquareDifferenceImageToImageMetric<FixedImageType, MovingImageType>::New();
  //------------------------------------------------------------
  // The lambda value is the intensity difference that should
  // make the metric drop by 50%
  //------------------------------------------------------------
  MetricPointer->SetLambda( m_Controls.m_LambdaMeanReciprocalSquareDifference->text().toInt() );
  MetricPointer->SetComputeGradient(m_Controls.m_ComputeGradient->isChecked());
  m_MetricObject = MetricPointer.GetPointer();
  return MetricPointer.GetPointer();
}

itk::Array<double> QmitkMeanReciprocalSquareDifferenceMetricView::GetMetricParameters()
{
  itk::Array<double> metricValues;
  metricValues.SetSize(2);
  metricValues.fill(0);
  metricValues[0] = m_Controls.m_ComputeGradient->isChecked();
  metricValues[1] = m_Controls.m_LambdaMeanReciprocalSquareDifference->text().toInt();
  return metricValues;
}

void QmitkMeanReciprocalSquareDifferenceMetricView::SetMetricParameters(itk::Array<double> metricValues)
{
  m_Controls.m_ComputeGradient->setChecked(metricValues[0]);
  m_Controls.m_LambdaMeanReciprocalSquareDifference->setText(QString::number(metricValues[1]));
}

QString QmitkMeanReciprocalSquareDifferenceMetricView::GetName()
{
  return "MeanReciprocalSquareDifference";
}

void QmitkMeanReciprocalSquareDifferenceMetricView::SetupUI(QWidget* parent)
{
  m_Controls.setupUi(parent);
  QValidator* validatorLineEditInput = new QIntValidator(0, 20000000, this);
  m_Controls.m_LambdaMeanReciprocalSquareDifference->setValidator(validatorLineEditInput);
}
