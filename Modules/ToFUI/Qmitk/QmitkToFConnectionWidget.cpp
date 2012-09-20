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

//#define _USE_MATH_DEFINES
#include <QmitkToFConnectionWidget.h>

//QT headers
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qcombobox.h>

//mitk headers
#include "mitkToFConfig.h"
//#include "mitkToFCameraPMDCamCubeDevice.h"
//#include "mitkToFCameraPMDRawDataCamCubeDevice.h"
//#include "mitkToFCameraPMDCamBoardDevice.h"
//#include "mitkToFCameraPMDRawDataCamBoardDevice.h"
//#include "mitkToFCameraPMDO3Device.h"
//#include "mitkToFCameraPMDPlayerDevice.h"
//#include "mitkToFCameraPMDMITKPlayerDevice.h"
#include "mitkToFCameraMITKPlayerDevice.h"
//#include "mitkToFCameraMESASR4000Device.h"
//#include "mitkKinectDevice.h"

//itk headers
#include <itksys/SystemTools.hxx>

const std::string QmitkToFConnectionWidget::VIEW_ID = "org.mitk.views.qmitktofconnectionwidget";
//Konstruktor des QmitkToFConnectionWidgets
QmitkToFConnectionWidget::QmitkToFConnectionWidget(QWidget* parent, Qt::WindowFlags f): QWidget(parent, f)
{
  this->m_IntegrationTime = 0;
  this->m_ModulationFrequency = 0;
  this->m_ToFImageGrabber = mitk::ToFImageGrabber::New();
  m_Controls = NULL;
  CreateQtPartControl(this);
}
//Destruktor of QmitkToFConnectionWidget ->"Q...." for a Qt created Method,in MITK-TOF ->"..mitkToF..."
QmitkToFConnectionWidget::~QmitkToFConnectionWidget()
{
}

void QmitkToFConnectionWidget::CreateQtPartControl(QWidget *parent)   //Definition of CreateQtPartControll-Methode in QmitkToFConnectionWidget; Input= Pointer
{
  if (!m_Controls)  //Define if not alreaddy exists
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkToFConnectionWidgetControls;
    m_Controls->setupUi(parent);
    this->CreateConnections();
    // set available cameras to combo box
    QString string(MITK_TOF_CAMERAS);                           // Initializin a QString "string"(same behaviour), der die Namen der Kameras beinhaltet
    string.replace(";"," ");                                    // erstezen von ";" durch ","
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

  this->OnSelectCamera(selectedCamera);
}

mitk::ToFImageGrabber* QmitkToFConnectionWidget::GetToFImageGrabber()
{
  return m_ToFImageGrabber;
}

void QmitkToFConnectionWidget::OnSelectCamera(const QString selectedCamera)
{
  if ((selectedCamera == "PMD CamCube 2.0/3.0")||(selectedCamera == "PMD CamBoard")||(selectedCamera=="PMD O3D")||
    (selectedCamera=="PMD CamBoardRaw")||(selectedCamera=="PMD CamCubeRaw 2.0/3.0") )
  {
    this->HideAllParameterWidgets();
    this->m_Controls->m_PMDParameterWidget->show();
  }
  else if (selectedCamera=="MESA Swissranger 4000")
  {
    this->HideAllParameterWidgets();
    this->m_Controls->m_MESAParameterWidget->show();
  }
  else if (selectedCamera=="Microsoft Kinect")
  {
    this->HideAllParameterWidgets();
    this->m_Controls->m_KinectParameterWidget->show();
  }
  else
  {
    this->HideAllParameterWidgets();
  }
}

void QmitkToFConnectionWidget::HideAllParameterWidgets()
{
  this->m_Controls->m_PMDParameterWidget->hide();
  this->m_Controls->m_MESAParameterWidget->hide();
  this->m_Controls->m_KinectParameterWidget->hide();
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
      //this->m_ToFImageGrabber->SetCameraDevice(mitk::ToFCameraPMDCamCubeDevice::New());
    }
    else if (selectedCamera == "PMD CamCubeRaw 2.0/3.0")
    { //PMD CamCube
      //this->m_ToFImageGrabber->SetCameraDevice(mitk::ToFCameraPMDRawDataCamCubeDevice::New());
    }
    else if (selectedCamera == "PMD CamBoard")
    { //PMD CamBoard
      //this->m_ToFImageGrabber->SetCameraDevice(mitk::ToFCameraPMDCamBoardDevice::New());
    }
    else if (selectedCamera == "PMD CamBoardRaw")
    { //PMD CamBoard
      //this->m_ToFImageGrabber->SetCameraDevice(mitk::ToFCameraPMDRawDataCamBoardDevice::New());
    }
    else if (selectedCamera == "PMD O3D")
    {//PMD O3
      //this->m_ToFImageGrabber->SetCameraDevice(mitk::ToFCameraPMDO3Device::New());
    }
    else if (selectedCamera == "MESA Swissranger 4000")
    {//MESA SR4000
      //this->m_ToFImageGrabber->SetCameraDevice(mitk::ToFCameraMESASR4000Device::New());
    }
    else if (selectedCamera == "Microsoft Kinect")
    {//KINECT
      //this->m_ToFImageGrabber->SetCameraDevice(mitk::KinectDevice::New());
    }
    else if (selectedCamera == "PMD Player")
    {//PMD player
      playerMode = true;
      fileFilter.append("PMD Files (*.pmd)");
      //this->m_ToFImageGrabber->SetCameraDevice(mitk::ToFCameraPMDPlayerDevice::New());
    }
    else if (selectedCamera == "PMD Raw Data Player")
    {//PMD MITK player
      playerMode = true;
      fileFilter.append("NRRD Images (*.nrrd);;PIC Images - deprecated (*.pic)");
      //this->m_ToFImageGrabber->SetCameraDevice(mitk::ToFCameraPMDRawDataDevice::New());
    }
    else if (selectedCamera == "MITK Player")
    {//MITK player
      playerMode = true;
      fileFilter.append("NRRD Images (*.nrrd);;PIC Images - deprecated (*.pic)");
      this->m_ToFImageGrabber->SetCameraDevice(mitk::ToFCameraMITKPlayerDevice::New());
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
            found = baseFilename.rfind("_RGBImage");
          }
          if (found == std::string::npos)
          {
            msg = msg + "Input file name must end with \"_DistanceImage\", \"_AmplitudeImage\", \"_IntensityImage\" or \"_RGBImage\"!";
            throw std::logic_error(msg.c_str());
          }
          std::string baseFilenamePrefix = baseFilename.substr(0,found);

          std::string distanceImageFileName = dir + "/" + baseFilenamePrefix + "_DistanceImage" + extension;
          std::string amplitudeImageFileName = dir + "/" + baseFilenamePrefix + "_AmplitudeImage" + extension;
          std::string intensityImageFileName = dir + "/" + baseFilenamePrefix + "_IntensityImage" + extension;
          std::string rgbImageFileName = dir + "/" + baseFilenamePrefix + "_RGBImage" + extension;

          if (!itksys::SystemTools::FileExists(distanceImageFileName.c_str(), true))
          {
            this->m_ToFImageGrabber->SetStringProperty("DistanceImageFileName", "");
          }
          else
          {
            this->m_ToFImageGrabber->SetStringProperty("DistanceImageFileName", distanceImageFileName.c_str());
          }
          if (!itksys::SystemTools::FileExists(amplitudeImageFileName.c_str(), true))
          {
          }
          else
          {
            this->m_ToFImageGrabber->SetStringProperty("AmplitudeImageFileName", amplitudeImageFileName.c_str());
          }
          if (!itksys::SystemTools::FileExists(intensityImageFileName.c_str(), true))
          {
            this->m_ToFImageGrabber->SetStringProperty("IntensityImageFileName", "");
          }
          else
          {
            this->m_ToFImageGrabber->SetStringProperty("IntensityImageFileName", intensityImageFileName.c_str());
          }
          if (!itksys::SystemTools::FileExists(rgbImageFileName.c_str(), true))
          {
            this->m_ToFImageGrabber->SetStringProperty("RGBImageFileName", "");
          }
          else
          {
            this->m_ToFImageGrabber->SetStringProperty("RGBImageFileName", rgbImageFileName.c_str());
          }
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
    m_Controls->m_ConnectCameraButton->setText("Disconnect");

    //if a connection could be established
    if (this->m_ToFImageGrabber->ConnectCamera())
    {
      this->m_Controls->m_PMDParameterWidget->SetToFImageGrabber(this->m_ToFImageGrabber);
      this->m_Controls->m_MESAParameterWidget->SetToFImageGrabber(this->m_ToFImageGrabber);
      this->m_Controls->m_KinectParameterWidget->SetToFImageGrabber(this->m_ToFImageGrabber);

      if (/*(selectedCamera == "PMD CamCube 2.0/3.0") ||*/ (selectedCamera == "PMD CamBoard") ||
        (selectedCamera== "PMD O3D") || (selectedCamera== "PMD CamBoardRaw") ||
        (selectedCamera== "PMD CamCubeRaw 2.0/3.0"))
      {
        this->m_Controls->m_PMDParameterWidget->ActivateAllParameters();
      }
      else if (selectedCamera=="MESA Swissranger 4000")
      {
        this->m_Controls->m_MESAParameterWidget->ActivateAllParameters();
      }
      else if (selectedCamera=="Microsoft Kinect")
      {
        this->m_Controls->m_KinectParameterWidget->ActivateAllParameters();
      }

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

    // send disconnect signal to the caller functionality
    emit ToFCameraDisconnected();

  }
}
