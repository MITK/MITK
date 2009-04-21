/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you want to add, delete, or rename functions or slots, use
** Qt Designer to update this file, preserving your code.
**
** You should not define a constructor or destructor in this file.
** Instead, write your code in functions called init() and destroy().
** These will automatically be called by the form's constructor and
** destructor.
*****************************************************************************/
#include <qvalidator.h>
#include <mitkImageCast.h>
#include <stdio.h>
#include <stdlib.h>
#include <mitkLevelWindowProperty.h>
#include <mitkRenderingManager.h>
#include "itkRegularStepGradientDescentOptimizer.h"
#include <qfiledialog.h>

void QmitkDemonsRegistrationControls::init()
{
  m_FixedNode = NULL;
  m_MovingNode = NULL;
  
  QValidator* validatorHistogramLevels = new QIntValidator(1, 20000000, this);
  m_NumberOfHistogramLevels->setValidator(validatorHistogramLevels);

  QValidator* validatorMatchPoints = new QIntValidator(1, 20000000, this);
  m_NumberOfMatchPoints->setValidator(validatorMatchPoints);

  QValidator* validatorIterations = new QIntValidator(1, 20000000, this);
  m_Iterations->setValidator(validatorIterations);

  QValidator* validatorStandardDeviation = new QDoubleValidator(0, 20000000, 2, this);
  m_StandardDeviation->setValidator(validatorStandardDeviation);
}

int QmitkDemonsRegistrationControls::GetNumberOfIterations()
{
  return atoi(m_Iterations->text().latin1());
}

float QmitkDemonsRegistrationControls::GetStandardDeviation()
{
  return atof(m_StandardDeviation->text().latin1());
}

void QmitkDemonsRegistrationControls::CalculateTransformation()
{
  if (m_FixedNode != NULL && m_MovingNode != NULL)
  {
    mitk::Image::Pointer fimage = dynamic_cast<mitk::Image*>(m_FixedNode->GetData());
    mitk::Image::Pointer mimage = dynamic_cast<mitk::Image*>(m_MovingNode->GetData());
    if ( m_RegistrationSelection->currentItem() == 0)
    {
      mitk::DemonsRegistration::Pointer registration = mitk::DemonsRegistration::New();

      // ask for filenames to save deformation field and result image
      QString deformationFieldname;
      deformationFieldname = QFileDialog::getSaveFileName(
        "deformationField.mhd",
        "*.mhd",
        this,  
        "Choose filename",
        "Where do you want to save the deformation field?");
      if ( !deformationFieldname.isEmpty() && deformationFieldname.right(4) != ".mhd" ) deformationFieldname += ".mhd";

      QString resultImagename;
        resultImagename = QFileDialog::getSaveFileName(
          "resultImage.mhd",
          "*.mhd",
          this,
          "Choose directory",
          "Where do you want to save the result image?");
      if ( !resultImagename.isEmpty() && resultImagename.right(4) != ".mhd" ) resultImagename += ".mhd";

      if(deformationFieldname.isEmpty())
        registration->SetSaveDeformationField(false);
      else
      {
        registration->SetDeformationFieldFileName(deformationFieldname);
      }
      registration->SetDeformationFieldFileName(deformationFieldname);

      if(resultImagename.isEmpty())
        registration->SetSaveResult(false);
      else
      {
        registration->SetResultFileName(resultImagename);
      }
      
      registration->SetReferenceImage(fimage);
      registration->SetNumberOfIterations(atoi(m_Iterations->text().latin1()));
      registration->SetStandardDeviation(atof(m_StandardDeviation->text().latin1()));
      if (m_UseHistogramMatching->isChecked())
      {
        mitk::HistogramMatching::Pointer histogramMatching = mitk::HistogramMatching::New();
        histogramMatching->SetReferenceImage(fimage);
        histogramMatching->SetInput(mimage);
        histogramMatching->SetNumberOfHistogramLevels(atoi(m_NumberOfHistogramLevels->text().latin1()));
        histogramMatching->SetNumberOfMatchPoints(atoi(m_NumberOfMatchPoints->text().latin1()));
        histogramMatching->SetThresholdAtMeanIntensity(m_ThresholdAtMeanIntensity->isChecked());
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
      registration->Update();
      mitk::Image::Pointer image = registration->GetOutput();
      if (image.IsNotNull())
      {
        m_MovingNode->SetData(image);
        mitk::LevelWindowProperty::Pointer levWinProp = mitk::LevelWindowProperty::New();
        mitk::LevelWindow levelWindow;
        levelWindow.SetAuto( image );
        levWinProp->SetLevelWindow(levelWindow);
        m_MovingNode->GetPropertyList()->SetProperty("levelwindow",levWinProp);
      }
    }
    else if(m_RegistrationSelection->currentItem() == 1)
    {
      mitk::SymmetricForcesDemonsRegistration::Pointer registration = mitk::SymmetricForcesDemonsRegistration::New();

      // ask for filenames to save deformation field and result image
      QString deformationFieldname;
      deformationFieldname = QFileDialog::getSaveFileName(
        "deformationField.mhd",
        "*.mhd",
        this,  
        "Choose filename",
        "Where do you want to save the deformation field?");
      if ( !deformationFieldname.isEmpty() && deformationFieldname.right(4) != ".mhd" ) deformationFieldname += ".mhd";

      QString resultImagename;
      resultImagename = QFileDialog::getSaveFileName(
        "resultImage.mhd",
        "*.mhd",
        this,
        "Choose directory",
        "Where do you want to save the result image?");
      if ( !resultImagename.isEmpty() && resultImagename.right(4) != ".mhd" ) resultImagename += ".mhd";

      if(deformationFieldname.isEmpty())
        registration->SetSaveDeformationField(false);
      else
      {
        registration->SetDeformationFieldFileName(deformationFieldname);
      }
      registration->SetDeformationFieldFileName(deformationFieldname);

      if(resultImagename.isEmpty())
        registration->SetSaveResult(false);
      else
      {
        registration->SetResultFileName(resultImagename);
      }

      registration->SetReferenceImage(fimage);
      registration->SetNumberOfIterations(atoi(m_Iterations->text().latin1()));
      registration->SetStandardDeviation(atof(m_StandardDeviation->text().latin1()));
      if (m_UseHistogramMatching->isChecked())
      {
        mitk::HistogramMatching::Pointer histogramMatching = mitk::HistogramMatching::New();
        histogramMatching->SetReferenceImage(fimage);
        histogramMatching->SetInput(mimage);
        histogramMatching->SetNumberOfHistogramLevels(atoi(m_NumberOfHistogramLevels->text().latin1()));
        histogramMatching->SetNumberOfMatchPoints(atoi(m_NumberOfMatchPoints->text().latin1()));
        histogramMatching->SetThresholdAtMeanIntensity(m_ThresholdAtMeanIntensity->isChecked());
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
      registration->Update();
      mitk::Image::Pointer image = registration->GetOutput();
      if (image.IsNotNull())
      {
        m_MovingNode->SetData(image);
        mitk::LevelWindowProperty::Pointer levWinProp = mitk::LevelWindowProperty::New();
        mitk::LevelWindow levelWindow;
        levelWindow.SetAuto( image );
        levWinProp->SetLevelWindow(levelWindow);
        m_MovingNode->GetPropertyList()->SetProperty("levelwindow",levWinProp);
      }
    }
    mitk::RenderingManager::GetInstance()->RequestUpdateAll();
  }
}

void QmitkDemonsRegistrationControls::SetFixedNode( mitk::DataTreeNode * fixedNode )
{
  m_FixedNode = fixedNode;
}

void QmitkDemonsRegistrationControls::SetMovingNode( mitk::DataTreeNode * movingNode )
{
  m_MovingNode = movingNode;
}

void QmitkDemonsRegistrationControls::UseHistogramMatching( bool useHM )
{
  if (useHM)
  {
    numberOfHistogramLevels->setEnabled(true);
    m_NumberOfHistogramLevels->setEnabled(true);
    numberOfMatchPoints->setEnabled(true);
    m_NumberOfMatchPoints->setEnabled(true);
    thresholdAtMeanIntensity->setEnabled(true);
    m_ThresholdAtMeanIntensity->setEnabled(true);
  }
  else
  {
    numberOfHistogramLevels->setEnabled(false);
    m_NumberOfHistogramLevels->setEnabled(false);
    numberOfMatchPoints->setEnabled(false);
    m_NumberOfMatchPoints->setEnabled(false);
    thresholdAtMeanIntensity->setEnabled(false);
    m_ThresholdAtMeanIntensity->setEnabled(false);
  }
}
