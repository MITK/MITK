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

QmitkDemonsRegistrationView::QmitkDemonsRegistrationView(QWidget* parent, Qt::WindowFlags f ) : QWidget( parent, f ),
m_FixedNode(NULL), m_MovingNode(NULL)
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

void QmitkDemonsRegistrationView::CalculateTransformation()
{
  if (m_FixedNode != NULL && m_MovingNode != NULL)
  {
    mitk::Image::Pointer fimage = dynamic_cast<mitk::Image*>(m_FixedNode->GetData());
    mitk::Image::Pointer mimage = dynamic_cast<mitk::Image*>(m_MovingNode->GetData());
    if ( m_Controls.m_RegistrationSelection->currentIndex() == 0)
    {
      QString deformationFieldname;
      QString resultImagename;
      mitk::DemonsRegistration::Pointer registration = mitk::DemonsRegistration::New();
      int saveResult = QMessageBox::question(this, "Save results", "Do you want to save the results?",QMessageBox::Yes,QMessageBox::No);
      if(saveResult == QMessageBox::Yes)
      {
        // ask for filenames to save deformation field and result image
        bool isValid = false;
        while(!isValid)
        {
          deformationFieldname = QFileDialog::getSaveFileName(this, "Where do you want to save the deformation field?",
            "deformationField.mhd",
            "*.mhd"
            );
          if ( !deformationFieldname.isEmpty() && deformationFieldname.right(4) != ".mhd" ) 
          {
            deformationFieldname += ".mhd";
          }
          else if (deformationFieldname.isEmpty())
          {
            QMessageBox::information(this, "Save file", "You have to specify a file name!",QMessageBox::Ok);
            isValid = false;
          }
          //check if file exists
          if(QFile::exists( deformationFieldname ))
          {
            //ask the user whether the file shall be overwritten
            int overwrite = QMessageBox::question(this, "Save file", "File already exists. Shall the file be overwritten?",QMessageBox::Yes,QMessageBox::No);
            if(overwrite == QMessageBox::Yes)
            {
              isValid = true;
            } 
            else
            {
              isValid  = false;
            }
          }
          else
          {
            isValid = true;
          }
        }        
        isValid = false;
        while(!isValid)
        {
          resultImagename = QFileDialog::getSaveFileName(this, "Where do you want to save the result image?",
            "resultImage.mhd",
            "*.mhd"
            );
          if ( !resultImagename.isEmpty() && resultImagename.right(4) != ".mhd" ) 
          {
            resultImagename += ".mhd";
          }
          else if (resultImagename.isEmpty())
          {
            QMessageBox::information(this, "Save file", "You have to specify a file name!",QMessageBox::Ok);
            isValid = false;
          }
          //check if file exists
          if (resultImagename == deformationFieldname)
          {
            QMessageBox::information(this, "Save file", "You have to specify a different file name than for the deformation field!",QMessageBox::Ok);
            isValid = false;
          }
          else if(QFile::exists( resultImagename ))
          {
            //ask the user whether the file shall be overwritten
            int overwrite = QMessageBox::question(this, "Save file", "File already exists. Shall the file be overwritten?",QMessageBox::Yes,QMessageBox::No);
            if(overwrite == QMessageBox::Yes)
            {
              isValid = true;
            } 
            else 
            {
              isValid  = false;
            }
          }
          else
          {
            isValid = true;
          }
        }        
        registration->SetSaveDeformationField(true);
        registration->SetSaveResult(true);
        registration->SetDeformationFieldFileName(deformationFieldname.toLatin1());
        registration->SetResultFileName(resultImagename.toLatin1());
      }
      else
      {
        registration->SetSaveDeformationField(false);
        registration->SetSaveResult(false);
      }

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
      }
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
    else if(m_Controls.m_RegistrationSelection->currentIndex() == 1)
    {
      QString deformationFieldname;
      QString resultImagename;
      mitk::SymmetricForcesDemonsRegistration::Pointer registration = mitk::SymmetricForcesDemonsRegistration::New();
      int saveResult = QMessageBox::question(this, "Save results", "Do you want to save the results?",QMessageBox::Yes,QMessageBox::No);
      if(saveResult == QMessageBox::Yes)
      {
        // ask for filenames to save deformation field and result image
        bool isValid = false;
        while(!isValid)
        {
          deformationFieldname = QFileDialog::getSaveFileName(this, "Where do you want to save the deformation field?",
            "deformationField.mhd",
            "*.mhd"
            );
          if ( !deformationFieldname.isEmpty() && deformationFieldname.right(4) != ".mhd" ) 
          {
            deformationFieldname += ".mhd";
          }
          else if (deformationFieldname.isEmpty())
          {
            QMessageBox::information(this, "Save file", "You have to specify a file name!",QMessageBox::Ok);
            isValid = false;
          }
          //check if file exists
          if(QFile::exists( deformationFieldname ))
          {
            //ask the user whether the file shall be overwritten
            int overwrite = QMessageBox::question(this, "Save file", "File already exists. Shall the file be overwritten?",QMessageBox::Yes,QMessageBox::No);
            if(overwrite == QMessageBox::Yes)
            {
              isValid = true;
            } 
            else
            {
              isValid  = false;
            }
          }
          else
          {
            isValid = true;
          }
        }        
        isValid = false;
        while(!isValid)
        {
          resultImagename = QFileDialog::getSaveFileName(this, "Where do you want to save the result image?",
            "resultImage.mhd",
            "*.mhd"
            );
          if ( !resultImagename.isEmpty() && resultImagename.right(4) != ".mhd" ) 
          {
            resultImagename += ".mhd";
          }
          else if (resultImagename.isEmpty())
          {
            QMessageBox::information(this, "Save file", "You have to specify a file name!",QMessageBox::Ok);
            isValid = false;
          }
          //check if file exists
          if (resultImagename == deformationFieldname)
          {
            QMessageBox::information(this, "Save file", "You have to specify a different file name than for the deformation field!",QMessageBox::Ok);
            isValid = false;
          }
          else if(QFile::exists( resultImagename ))
          {
            //ask the user whether the file shall be overwritten
            int overwrite = QMessageBox::question(this, "Save file", "File already exists. Shall the file be overwritten?",QMessageBox::Yes,QMessageBox::No);
            if(overwrite == QMessageBox::Yes)
            {
              isValid = true;
            } 
            else 
            {
              isValid  = false;
            }
          }
          else
          {
            isValid = true;
          }
        }        
        registration->SetSaveDeformationField(true);
        registration->SetSaveResult(true);
        registration->SetDeformationFieldFileName(deformationFieldname.toLatin1());
        registration->SetResultFileName(resultImagename.toLatin1());
      }
      else
      {
        registration->SetSaveDeformationField(false);
        registration->SetSaveResult(false);
      }

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

void QmitkDemonsRegistrationView::SetFixedNode( mitk::DataTreeNode * fixedNode )
{
  m_FixedNode = fixedNode;
}

void QmitkDemonsRegistrationView::SetMovingNode( mitk::DataTreeNode * movingNode )
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
