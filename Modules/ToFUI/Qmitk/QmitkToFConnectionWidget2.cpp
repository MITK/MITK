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
#include <QmitkToFConnectionWidget2.h>

//QT headers
#include <qmessagebox.h>
#include <qfiledialog.h>
#include <qcombobox.h>

//mitk headers
#include "mitkToFConfig.h"

//itk headers
#include <itksys/SystemTools.hxx>

const std::string QmitkToFConnectionWidget2::VIEW_ID = "org.mitk.views.qmitktofconnectionwidget2";

//Constructor of QmitkToFConnectionWidget2
QmitkToFConnectionWidget2::QmitkToFConnectionWidget2(QWidget* parent, Qt::WindowFlags f): QWidget(parent, f)
{
  this->m_IntegrationTime = 0;
  this->m_ModulationFrequency = 0;
  this->m_ToFImageGrabber = mitk::ToFImageGrabber::New();
  m_Controls = NULL;
  CreateQtPartControl(this);
}

//Destructor of QmitkToFConnectionWidget2
QmitkToFConnectionWidget2::~QmitkToFConnectionWidget2()
{
  //MitkServiceListWidget must not be deinizialized here. Qmitk methods destroy ther children automatically before destucting themselfes!
}

void QmitkToFConnectionWidget2::CreateQtPartControl(QWidget *parent)   //Definition of CreateQtPartControll-Methode in QmitkToFConnectionWidget2; Input= Pointer
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkToFConnectionWidgetControls2;
    m_Controls->setupUi(parent);

    // initzializing MitkServiceListWidget here
    std::string empty= "";
    m_Controls->m_DeviceList->Initialize<mitk::ToFCameraDevice>("ToFDeviceName", empty);

    this->CreateConnections();
    //OnSelectCamera(); //todo: warum geht das hier nicht?
  }
}

void QmitkToFConnectionWidget2::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->m_ConnectCameraButton), SIGNAL(clicked()),(QObject*) this, SLOT(OnConnectCamera()) );
    connect( m_Controls->m_DeviceList, SIGNAL(ServiceSelectionChanged(mitk::ServiceReference)), this, SLOT(OnSelectCamera()));
  }
}

mitk::ToFImageGrabber::Pointer QmitkToFConnectionWidget2::GetToFImageGrabber()
{
  return m_ToFImageGrabber;
}

void QmitkToFConnectionWidget2::OnSelectCamera()
{
  mitk::ToFCameraDevice* device = m_Controls->m_DeviceList->GetSelectedService<mitk::ToFCameraDevice>();
  QString selectedCamera = QString::fromStdString(device->GetNameOfClass());
  //verstecke alle widgets
  this->HideAllParameterWidgets();

  if (selectedCamera.contains("PMD")) // Changed IF  //finde Teilsstrings in einem string
  {
    this->m_Controls->m_PMDParameterWidget->show();
  }
  else if (selectedCamera.contains("MESA"))
  {
    this->m_Controls->m_MESAParameterWidget->show();
  }
  else if (selectedCamera.contains("Kinect"))
  {
    this->m_Controls->m_KinectParameterWidget->show();
  }
  emit ToFCameraSelected(selectedCamera);
}

void QmitkToFConnectionWidget2::HideAllParameterWidgets()
{
  this->m_Controls->m_PMDParameterWidget->hide();
  this->m_Controls->m_MESAParameterWidget->hide();
  this->m_Controls->m_KinectParameterWidget->hide();
}

void QmitkToFConnectionWidget2::OnConnectCamera()
{
  bool playerMode = false;

  //Changes to be done after connecting
  if (m_Controls->m_ConnectCameraButton->text()=="Connect")
  {
    //Disabling some GUI-Elements
    m_Controls->m_ConnectCameraButton->setEnabled(false);
    m_Controls->m_DeviceList->setEnabled(false);
    //repaint the widget
    this->repaint();

    QString tmpFileName("");
    QString fileFilter("");

    mitk::ToFCameraDevice* device = m_Controls->m_DeviceList->GetSelectedService<mitk::ToFCameraDevice>();
    QString selectedCamera = QString::fromStdString(device->GetNameOfClass());

    this->m_ToFImageGrabber = mitk::ToFImageGrabber::New();

    this->m_ToFImageGrabber->SetCameraDevice(device); //todo device aus keno's widget holen und hier setzen---------------------------------

    //Activation of "PlayerMode". If the selectedCamera String contains "Player", we start the Player Mode
    if (selectedCamera.contains("Player"))
    {
      playerMode = true;
      //IF PMD-Player selected
      if (selectedCamera.contains("PMD"))
      {
        fileFilter.append("PMD Files (*.pmd)");
      }
      else
      {
        fileFilter.append("NRRD Images (*.nrrd);;PIC Images - deprecated (*.pic)");
      }
    }

    // if a player was selected ...
    if (playerMode)
    {
      //... open a QFileDialog to chose the corresponding file from the disc
      tmpFileName = QFileDialog::getOpenFileName(NULL, "Play Image From...", "", fileFilter);
      MITK_INFO << tmpFileName.toStdString();

      //If no fileName is returned by the Dialog,Button and Widget have to return to default + Opening a MessageBox
      if (tmpFileName.isEmpty())
      {
        m_Controls->m_ConnectCameraButton->setChecked(false);
        m_Controls->m_ConnectCameraButton->setEnabled(true);
        m_Controls->m_DeviceList->setEnabled(true);           //Reactivating ServiceListWidget

        this->OnSelectCamera();
        QMessageBox::information( this, "Template functionality", "Please select a valid image before starting some action.");
        return;
      }

      if(selectedCamera.contains("PMDPlayer")) //if current device name enhält
      { //set the PMD file name
        this->m_ToFImageGrabber->SetStringProperty("PMDFileName", tmpFileName.toStdString().c_str() );
        MITK_INFO <<"PMD selected";
      }
      else    //The PMD RAW Data Player does not exist anymore so we can just use (selectedCamera.contains("PMD Raw Data Player") || selectedCamera.contains("MITK Player") ) //todo eventuell default
      {
        MITK_INFO << "An other Player than PMD is selected. Maybe the MITK-Player";
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
            MITK_INFO << "DistanceImageFileName " << distanceImageFileName.c_str();
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
          m_Controls->m_DeviceList->setEnabled(true);
          this->OnSelectCamera();
          return;
        }
      }
    }
    //End "PlayerMode"

    m_Controls->m_ConnectCameraButton->setText("Disconnect");

    //if a connection could be established
    if (this->m_ToFImageGrabber->ConnectCamera())
    {
      //todo: probieren ob das auch in CreateQtPartControl passieren kann
      this->m_Controls->m_PMDParameterWidget->SetToFImageGrabber(this->m_ToFImageGrabber);
      this->m_Controls->m_MESAParameterWidget->SetToFImageGrabber(this->m_ToFImageGrabber);
      this->m_Controls->m_KinectParameterWidget->SetToFImageGrabber(this->m_ToFImageGrabber);


      //todo nochmal gleiche fallunterscheidung. Oder aber if/else eventuell komplett weglassen------------------------------------------------------------------
      if (selectedCamera.contains("PMD"))
      {
        this->m_Controls->m_PMDParameterWidget->ActivateAllParameters();
      }
      else if (selectedCamera.contains("MESA"))
      {
        this->m_Controls->m_MESAParameterWidget->ActivateAllParameters();
      }
      else if (selectedCamera.contains("Kinect"))
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
      m_Controls->m_DeviceList->setEnabled(true);           //Reactivating ServiceListWidget
      this->OnSelectCamera();
      return;

    }
    m_Controls->m_ConnectCameraButton->setEnabled(true);

  }
  else if (m_Controls->m_ConnectCameraButton->text()=="Disconnect")
  {
    this->m_ToFImageGrabber->StopCamera();
    this->m_ToFImageGrabber->DisconnectCamera();
    m_Controls->m_ConnectCameraButton->setText("Connect");
    m_Controls->m_DeviceList->setEnabled(true);           //Reactivating ServiceListWidget
    this->OnSelectCamera();

    // send disconnect signal to the caller functionality
    emit ToFCameraDisconnected();
  }
}
