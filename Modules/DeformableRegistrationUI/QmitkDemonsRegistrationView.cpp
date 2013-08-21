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

#include <qvalidator.h>
#include <mitkImageCast.h>
#include <stdio.h>
#include <stdlib.h>
#include <mitkLevelWindowProperty.h>
#include <mitkRenderingManager.h>
#include "itkRegularStepGradientDescentOptimizer.h"
#include <qfiledialog.h>
#include <qmessagebox.h>
#include "QmitkDemonsRegistrationView.h"
#include "ui_QmitkDemonsRegistrationViewControls.h"
#include "mitkITKImageImport.h"
#include "mitkProgressBar.h"

QmitkDemonsRegistrationView::QmitkDemonsRegistrationView(QWidget* parent, Qt::WindowFlags f ) : QWidget( parent, f ),
m_FixedNode(NULL), m_MovingNode(NULL), m_ResultImage(NULL), m_ResultDeformationField(NULL)
{
  m_Controls.setupUi(parent);

  QValidator* validatorHistogramLevels = new QIntValidator(1, 20000000, this);
  m_Controls.m_NumberOfHistogramLevels->setValidator(validatorHistogramLevels);

  QValidator* validatorMatchPoints = new QIntValidator(1, 20000000, this);
  m_Controls.m_NumberOfMatchPoints->setValidator(validatorMatchPoints);

  QValidator* validatorIterations = new QIntValidator(1, 20000000, this);
  m_Controls.m_Iterations->setValidator(validatorIterations);

  QValidator* validatorStandardDeviation = new QDoubleValidator(0, 20000000, 2, this);
  m_Controls.m_StandardDeviation->setValidator(validatorStandardDeviation);
}

QmitkDemonsRegistrationView::~QmitkDemonsRegistrationView()
{
}


int QmitkDemonsRegistrationView::GetNumberOfIterations()
{
  return atoi(m_Controls.m_Iterations->text().toLatin1());
}

float QmitkDemonsRegistrationView::GetStandardDeviation()
{
  return atof(m_Controls.m_StandardDeviation->text().toLatin1());
}

mitk::Image::Pointer QmitkDemonsRegistrationView::GetResultImage()
{
  return m_ResultImage;
}

mitk::Image::Pointer QmitkDemonsRegistrationView::GetResultDeformationfield()
{
  return m_ResultDeformationField;
}

void QmitkDemonsRegistrationView::CalculateTransformation()
{
  if (m_FixedNode.IsNotNull() && m_MovingNode.IsNotNull())
  {
    mitk::Image::Pointer fimage = dynamic_cast<mitk::Image*>(m_FixedNode->GetData());
    mitk::Image::Pointer mimage = dynamic_cast<mitk::Image*>(m_MovingNode->GetData());
    // workaround to ensure that fimage covers a bigger region than mimage
    mitk::Image::RegionType fimageRegion = fimage->GetLargestPossibleRegion();
    mitk::Image::RegionType mimageRegion = mimage->GetLargestPossibleRegion();
    if (!((fimageRegion.GetSize(0)>=mimageRegion.GetSize(0))&&(fimageRegion.GetSize(1)>=mimageRegion.GetSize(1))
        &&(fimageRegion.GetSize(2)>=mimageRegion.GetSize(2))))
    {
      QMessageBox::information(NULL,"Registration","Fixed image must be equal or bigger in size than moving image.");
      return;
    }
    if ( m_Controls.m_RegistrationSelection->currentIndex() == 0)
    {
      mitk::DemonsRegistration::Pointer registration = mitk::DemonsRegistration::New();
      registration->SetSaveDeformationField(false);
      registration->SetSaveResult(false);
      registration->SetReferenceImage(fimage);
      registration->SetNumberOfIterations(atoi(m_Controls.m_Iterations->text().toLatin1()));
      registration->SetStandardDeviation(atof(m_Controls.m_StandardDeviation->text().toLatin1()));
      if (m_Controls.m_UseHistogramMatching->isChecked())
      {
        mitk::HistogramMatching::Pointer histogramMatching = mitk::HistogramMatching::New();
        histogramMatching->SetReferenceImage(fimage);
        histogramMatching->SetInput(mimage);
        histogramMatching->SetNumberOfHistogramLevels(atoi(m_Controls.m_NumberOfHistogramLevels->text().toLatin1()));
        histogramMatching->SetNumberOfMatchPoints(atoi(m_Controls.m_NumberOfMatchPoints->text().toLatin1()));
        histogramMatching->SetThresholdAtMeanIntensity(m_Controls.m_ThresholdAtMeanIntensity->isChecked());
        histogramMatching->Update();
        mitk::Image::Pointer histimage = histogramMatching->GetOutput();
        if (histimage.IsNotNull())
        {
          registration->SetInput(histimage);
        }
        else
        {
          registration->SetInput(mimage);
        }
      }
      else
      {
        registration->SetInput(mimage);
      }
      try
      {
        registration->Update();
      }
      catch (itk::ExceptionObject& excpt)
      {
        QMessageBox::information( this, "Registration exception", excpt.GetDescription(), QMessageBox::Ok );
        mitk::ProgressBar::GetInstance()->Progress(4);
        return;
      }
      m_ResultImage = registration->GetOutput();
      typedef itk::Image<itk::Vector<float,3>, 3> VectorImageType;
      VectorImageType::Pointer deformationField = registration->GetDeformationField();
      m_ResultDeformationField = mitk::ImportItkImage(deformationField)->Clone();
    }
    else if(m_Controls.m_RegistrationSelection->currentIndex() == 1)
    {
      mitk::SymmetricForcesDemonsRegistration::Pointer registration = mitk::SymmetricForcesDemonsRegistration::New();
      registration->SetSaveDeformationField(false);
      registration->SetSaveResult(false);
      registration->SetReferenceImage(fimage);
      registration->SetNumberOfIterations(atoi(m_Controls.m_Iterations->text().toLatin1()));
      registration->SetStandardDeviation(atof(m_Controls.m_StandardDeviation->text().toLatin1()));
      if (m_Controls.m_UseHistogramMatching->isChecked())
      {
        mitk::HistogramMatching::Pointer histogramMatching = mitk::HistogramMatching::New();
        histogramMatching->SetReferenceImage(fimage);
        histogramMatching->SetInput(mimage);
        histogramMatching->SetNumberOfHistogramLevels(atoi(m_Controls.m_NumberOfHistogramLevels->text().toLatin1()));
        histogramMatching->SetNumberOfMatchPoints(atoi(m_Controls.m_NumberOfMatchPoints->text().toLatin1()));
        histogramMatching->SetThresholdAtMeanIntensity(m_Controls.m_ThresholdAtMeanIntensity->isChecked());
        histogramMatching->Update();
        mitk::Image::Pointer histimage = histogramMatching->GetOutput();
        if (histimage.IsNotNull())
        {
          registration->SetInput(histimage);
        }
        else
        {
          registration->SetInput(mimage);
        }
      }
      else
      {
        registration->SetInput(mimage);
      }
      try
      {
        registration->Update();
      }
      catch (itk::ExceptionObject& excpt)
      {
        QMessageBox::information( this, "Registration exception", excpt.GetDescription(), QMessageBox::Ok );
        mitk::ProgressBar::GetInstance()->Progress(4);
        return;
      }
      m_ResultImage = registration->GetOutput();
      typedef itk::Image<itk::Vector<float,3>, 3> VectorImageType;
      VectorImageType::Pointer deformationField = registration->GetDeformationField();
      m_ResultDeformationField = mitk::ImportItkImage(deformationField)->Clone();
    }
  }
}

void QmitkDemonsRegistrationView::SetFixedNode( mitk::DataNode * fixedNode )
{
  m_FixedNode = fixedNode;
}

void QmitkDemonsRegistrationView::SetMovingNode( mitk::DataNode * movingNode )
{
  m_MovingNode = movingNode;
}

void QmitkDemonsRegistrationView::UseHistogramMatching( bool useHM )
{
  if (useHM)
  {
    m_Controls.numberOfHistogramLevels->setEnabled(true);
    m_Controls.m_NumberOfHistogramLevels->setEnabled(true);
    m_Controls.numberOfMatchPoints->setEnabled(true);
    m_Controls.m_NumberOfMatchPoints->setEnabled(true);
    m_Controls.thresholdAtMeanIntensity->setEnabled(true);
    m_Controls.m_ThresholdAtMeanIntensity->setEnabled(true);
  }
  else
  {
    m_Controls.numberOfHistogramLevels->setEnabled(false);
    m_Controls.m_NumberOfHistogramLevels->setEnabled(false);
    m_Controls.numberOfMatchPoints->setEnabled(false);
    m_Controls.m_NumberOfMatchPoints->setEnabled(false);
    m_Controls.thresholdAtMeanIntensity->setEnabled(false);
    m_Controls.m_ThresholdAtMeanIntensity->setEnabled(false);
  }
}
