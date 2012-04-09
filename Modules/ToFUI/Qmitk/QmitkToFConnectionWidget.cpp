/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2009-05-20 13:35:09 +0200 (Mi, 20 Mai 2009) $
Version:   $Revision: 17332 $

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

//#define _USE_MATH_DEFINES
#include <QmitkToFConnectionWidget.h>

//QT headers
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qcombobox.h>

//mitk headers
#include "mitkToFConfig.h"
#include <mitkToFImageGrabberCreator.h>

//itk headers
#include <itksys/SystemTools.hxx>

const std::string QmitkToFConnectionWidget::VIEW_ID = "org.mitk.views.qmitktofconnectionwidget";

QmitkToFConnectionWidget::QmitkToFConnectionWidget(QWidget* parent, Qt::WindowFlags f): QWidget(parent, f)
{
  this->m_IntegrationTime = 0;
  this->m_ModulationFrequency = 0;
  this->m_ToFImageGrabber = NULL;

  m_Controls = NULL;
  CreateQtPartControl(this);
}

QmitkToFConnectionWidget::~QmitkToFConnectionWidget()
{
}

void QmitkToFConnectionWidget::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkToFConnectionWidgetControls;
    m_Controls->setupUi(parent);
    this->CreateConnections();
    // set available cameras to combo box
    QString string(MITK_TOF_CAMERAS);
    string.replace(";"," ");
    QStringList list = string.split(",");
    m_Controls->m_SelectCameraCombobox->addItems(list);

    ShowParameterWidget();
  }
}

void QmitkToFConnectionWidget::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->m_ConnectCameraButton), SIGNAL(clicked()),(QObject*) this, SLOT(OnConnectCamera()) );
    connect( m_Controls->m_SelectCameraCombobox, SIGNAL(currentIndexChanged(const QString)), this, SLOT(OnSelectCamera(const QString)) );
    connect( m_Controls->m_SelectCameraCombobox, SIGNAL(activated(const QString)), this, SLOT(OnSelectCamera(const QString)) );
    connect( m_Controls->m_SelectCameraCombobox, SIGNAL(activated(const QString)), this, SIGNAL(ToFCameraSelected(const QString)) );
  }
}

void QmitkToFConnectionWidget::ShowParameterWidget()
{
  QString selectedCamera = m_Controls->m_SelectCameraCombobox->currentText();

  if ((selectedCamera == "PMD CamCube 2.0/3.0")||(selectedCamera == "PMD CamBoard")||(selectedCamera=="PMD O3D")||
    (selectedCamera=="PMD CamBoardRaw")||(selectedCamera=="PMD CamCubeRaw") )
  {
    ShowPMDParameterWidget();
  }
  else if (selectedCamera=="MESA Swissranger 4000")
  {
    ShowMESAParameterWidget();
  }
  else
  {
    this->m_Controls->m_PMDParameterWidget->hide();
    this->m_Controls->m_MESAParameterWidget->hide();
  }
}

void QmitkToFConnectionWidget::ShowPMDParameterWidget()
{
  this->m_Controls->m_PMDParameterWidget->show();
  this->m_Controls->m_MESAParameterWidget->hide();
}

void QmitkToFConnectionWidget::ShowMESAParameterWidget()
{
  this->m_Controls->m_PMDParameterWidget->hide();
  this->m_Controls->m_MESAParameterWidget->show();
}

void QmitkToFConnectionWidget::ShowPlayerParameterWidget()
{
  this->m_Controls->m_PMDParameterWidget->hide();
  this->m_Controls->m_MESAParameterWidget->hide();
}

mitk::ToFImageGrabber* QmitkToFConnectionWidget::GetToFImageGrabber()
{
  return m_ToFImageGrabber;
}

void QmitkToFConnectionWidget::OnSelectCamera(const QString selectedText)
{
  if (selectedText == "PMD CamCube 2.0/3.0" || selectedText == "PMD CamCubeRaw 2.0/3.0" ) // PMD camcube 2
  {
    ShowPMDParameterWidget();
  }
  else if (selectedText == "PMD CamBoard" || selectedText == "PMD CamBoardRaw" ) // pmd camboard
  {
    ShowPMDParameterWidget();
  }
  else if (selectedText == "PMD O3D") // pmd O3d
  {
    ShowPMDParameterWidget();
  }
  else if (selectedText == "MESA Swissranger 4000") // MESA 4000
  {
    ShowMESAParameterWidget();
  }
  else if (selectedText == "PMD Player") // pmd file player
  {
    ShowPlayerParameterWidget();
  }
  else if (selectedText == "PMD Raw Data Player") // pmd raw data player
  {
    ShowPlayerParameterWidget();
  }
  else if (selectedText == "MITK Player") // mitk player
  {
    ShowPlayerParameterWidget();
  }
}

void QmitkToFConnectionWidget::OnConnectCamera()
{
  bool playerMode = false;

  if (m_Controls->m_ConnectCameraButton->text()=="Connect")
  {   
    //reset the status of the GUI buttons
    m_Controls->m_ConnectCameraButton->setEnabled(false);
    m_Controls->m_SelectCameraCombobox->setEnabled(false);
    //repaint the widget
    this->repaint();

    QString tmpFileName("");
    QString fileFilter("");

    //select the camera to connect with
    QString selectedCamera = m_Controls->m_SelectCameraCombobox->currentText();
    if (selectedCamera == "PMD CamCube 2.0/3.0")
    { //PMD CamCube
      this->m_ToFImageGrabber = mitk::ToFImageGrabberCreator::GetInstance()->GetPMDCamCubeImageGrabber();
    }
    else if (selectedCamera == "PMD CamCubeRaw 2.0/3.0")
    { //PMD CamCube
      this->m_ToFImageGrabber = mitk::ToFImageGrabberCreator::GetInstance()->GetPMDRawDataCamCubeImageGrabber();
    }
    else if (selectedCamera == "PMD CamBoard")
    { //PMD CamBoard
      this->m_ToFImageGrabber = mitk::ToFImageGrabberCreator::GetInstance()->GetPMDCamBoardImageGrabber();
    }
    else if (selectedCamera == "PMD CamBoardRaw")
    { //PMD CamBoard
      this->m_ToFImageGrabber = mitk::ToFImageGrabberCreator::GetInstance()->GetPMDRawDataCamBoardImageGrabber();
    }
    else if (selectedCamera == "PMD O3D")
    {//PMD O3
      this->m_ToFImageGrabber = mitk::ToFImageGrabberCreator::GetInstance()->GetPMDO3ImageGrabber();
    }
    else if (selectedCamera == "MESA Swissranger 4000")
    {//MESA SR4000
      this->m_ToFImageGrabber = mitk::ToFImageGrabberCreator::GetInstance()->GetMESASR4000ImageGrabber();
    }
    else if (selectedCamera == "PMD Player")
    {//PMD player
      playerMode = true;
      fileFilter.append("PMD Files (*.pmd)");
      this->m_ToFImageGrabber = mitk::ToFImageGrabberCreator::GetInstance()->GetPMDPlayerImageGrabber();
    }
    else if (selectedCamera == "PMD Raw Data Player")
    {//PMD MITK player
      playerMode = true;
      fileFilter.append("NRRD Images (*.nrrd);;PIC Images - deprecated (*.pic)");
      this->m_ToFImageGrabber = mitk::ToFImageGrabberCreator::GetInstance()->GetPMDMITKPlayerImageGrabber();
    }
    else if (selectedCamera == "MITK Player")
    {//MITK player
      playerMode = true;
      fileFilter.append("NRRD Images (*.nrrd);;PIC Images - deprecated (*.pic)");
      this->m_ToFImageGrabber = mitk::ToFImageGrabberCreator::GetInstance()->GetMITKPlayerImageGrabber();
    }

    // if a player was selected ...
    if (playerMode)
    { //... open a QFileDialog to chose the corresponding file from the disc
      tmpFileName = QFileDialog::getOpenFileName(NULL, "Play Image From...", "", fileFilter);
      if (tmpFileName.isEmpty())
      {
        m_Controls->m_ConnectCameraButton->setChecked(false);
        m_Controls->m_ConnectCameraButton->setEnabled(true);
        m_Controls->m_SelectCameraCombobox->setEnabled(true);
        this->OnSelectCamera(m_Controls->m_SelectCameraCombobox->currentText());
        QMessageBox::information( this, "Template functionality", "Please select a valid image before starting some action.");
        return;
      }
      if(selectedCamera == "PMD Player")
      { //set the PMD file name
        this->m_ToFImageGrabber->SetStringProperty("PMDFileName", tmpFileName.toStdString().c_str() );
      }
      if (selectedCamera == "PMD Raw Data Player" || selectedCamera == "MITK Player")
      {
        std::string msg = "";
        try
        {
          //get 3 corresponding file names
          std::string dir = itksys::SystemTools::GetFilenamePath( tmpFileName.toStdString() );
          std::string baseFilename = itksys::SystemTools::GetFilenameWithoutLastExtension( tmpFileName.toStdString() );
          std::string extension = itksys::SystemTools::GetFilenameLastExtension( tmpFileName.toStdString() );

          if (extension != ".pic" && extension != ".nrrd")
          {
            msg = msg + "Invalid file format, please select a \".nrrd\"-file";
            throw std::logic_error(msg.c_str());
          }
          int found = baseFilename.rfind("_DistanceImage");
          if (found == std::string::npos)
          {
            found = baseFilename.rfind("_AmplitudeImage");
          }
          if (found == std::string::npos)
          {
            found = baseFilename.rfind("_IntensityImage");
          }
          if (found == std::string::npos)
          {
            msg = msg + "Input file name must end with \"_DistanceImage.pic\", \"_AmplitudeImage.pic\" or \"_IntensityImage.pic\"!";
            throw std::logic_error(msg.c_str());
          }
          std::string baseFilenamePrefix = baseFilename.substr(0,found);

          std::string distanceImageFileName = dir + "/" + baseFilenamePrefix + "_DistanceImage" + extension;
          std::string amplitudeImageFileName = dir + "/" + baseFilenamePrefix + "_AmplitudeImage" + extension;
          std::string intensityImageFileName = dir + "/" + baseFilenamePrefix + "_IntensityImage" + extension;

          if (!itksys::SystemTools::FileExists(distanceImageFileName.c_str(), true))
          {
            msg = msg + "Inputfile not exist! " + distanceImageFileName;
            throw std::logic_error(msg.c_str());
          }
          if (!itksys::SystemTools::FileExists(amplitudeImageFileName.c_str(), true))
          {
            msg = msg + "Inputfile not exist! " + amplitudeImageFileName;
            throw std::logic_error(msg.c_str());
          }
          if (!itksys::SystemTools::FileExists(intensityImageFileName.c_str(), true))
          {
            msg = msg + "Inputfile not exist! " + intensityImageFileName;
            throw std::logic_error(msg.c_str());
          }
          //set the file names
          this->m_ToFImageGrabber->SetStringProperty("DistanceImageFileName", distanceImageFileName.c_str());
          this->m_ToFImageGrabber->SetStringProperty("AmplitudeImageFileName", amplitudeImageFileName.c_str());
          this->m_ToFImageGrabber->SetStringProperty("IntensityImageFileName", intensityImageFileName.c_str());

        }
        catch (std::exception &e)
        {
          MITK_ERROR << e.what();
          QMessageBox::critical( this, "Error", e.what() );
          m_Controls->m_ConnectCameraButton->setChecked(false);
          m_Controls->m_ConnectCameraButton->setEnabled(true);
          m_Controls->m_SelectCameraCombobox->setEnabled(true);
          this->OnSelectCamera(m_Controls->m_SelectCameraCombobox->currentText());
          return;
        }
      }

    }
    //if a connection could be established
    if (this->m_ToFImageGrabber->ConnectCamera())
    {
      this->m_Controls->m_PMDParameterWidget->SetToFImageGrabber(this->m_ToFImageGrabber);
      this->m_Controls->m_MESAParameterWidget->SetToFImageGrabber(this->m_ToFImageGrabber);

      if ((selectedCamera == "PMD CamCube 2.0/3.0")||(selectedCamera == "PMD CamBoard")||(selectedCamera=="PMD O3D")||
        (selectedCamera=="PMD CamBoardRaw")||(selectedCamera=="PMD CamCubeRaw 2.0/3.0"))
      {
        this->m_Controls->m_PMDParameterWidget->ActivateAllParameters();
      }
      else if (selectedCamera=="MESA Swissranger 4000")
      {
        this->m_Controls->m_MESAParameterWidget->ActivateAllParameters();
      }
      m_Controls->m_ConnectCameraButton->setText("Disconnect");

      // send connect signal to the caller functionality
      emit ToFCameraConnected();
    }
    else
    {
      QMessageBox::critical( this, "Error", "Connection failed. Check if you have installed the latest driver for your system." );
      m_Controls->m_ConnectCameraButton->setChecked(false);
      m_Controls->m_ConnectCameraButton->setEnabled(true);
      m_Controls->m_SelectCameraCombobox->setEnabled(true);
      this->OnSelectCamera(m_Controls->m_SelectCameraCombobox->currentText());
      return;

    }
    m_Controls->m_ConnectCameraButton->setEnabled(true);

  }
  else if (m_Controls->m_ConnectCameraButton->text()=="Disconnect")
  {
    this->m_ToFImageGrabber->StopCamera();
    this->m_ToFImageGrabber->DisconnectCamera();
    m_Controls->m_ConnectCameraButton->setText("Connect");
    m_Controls->m_SelectCameraCombobox->setEnabled(true);
    this->OnSelectCamera(m_Controls->m_SelectCameraCombobox->currentText());

    this->m_ToFImageGrabber = NULL;
    // send disconnect signal to the caller functionality
    emit ToFCameraDisconnected();

  }
}
