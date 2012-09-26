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

#include "mitkToFCameraMITKPlayerDevice.h"


//itk headers
#include <itksys/SystemTools.hxx>

const std::string QmitkToFConnectionWidget2::VIEW_ID = "org.mitk.views.qmitktofconnectionwidget2";

//Konstruktor des QmitkToFConnectionWidget2
QmitkToFConnectionWidget2::QmitkToFConnectionWidget2(QWidget* parent, Qt::WindowFlags f): QWidget(parent, f)
{
  this->m_IntegrationTime = 0;
  this->m_ModulationFrequency = 0;
  this->m_ToFImageGrabber = mitk::ToFImageGrabber::New();
  m_Controls = NULL;
  CreateQtPartControl(this);
}

//Destruktor of QmitkToFConnectionWidget2
QmitkToFConnectionWidget2::~QmitkToFConnectionWidget2()
{
  //evtl. deinizialisieren von keno´s widget
  //_____________________________________________KENO FRAGEN!!!!!!!!!_____________________________________________
  //RemoveServiceFromList (mitk::ServiceReference ServiceReferencePrivate);  ?!
}

void QmitkToFConnectionWidget2::CreateQtPartControl(QWidget *parent)   //Definition of CreateQtPartControll-Methode in QmitkToFConnectionWidget2; Input= Pointer
{
  if (!m_Controls)
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkToFConnectionWidgetControls2;
    m_Controls->setupUi(parent);
    this->CreateConnections();

    ShowParameterWidget();

    //Keno´s Device  hier initizialisieren-> überall sonst wo die Methode ShowParameterWidget aufgerufen wird, kann die "externe" inizilaisierung weggelassen werden!
    std::string empty= "";
    m_Controls->m_DeviceList->Initialize<mitk::ToFCameraDevice>("ToFDeviceName", empty);
  }
}

void QmitkToFConnectionWidget2::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->m_ConnectCameraButton), SIGNAL(clicked()),(QObject*) this, SLOT(OnConnectCamera()) );

 //todo finde ein equivalent: zB currentIndexChanged in Keno's Widget???  ServiceProperties deviceProps -> ServiceSelectionChanged(ServiceReference)
    //std::string CamName =mitk::ToFCameraMITKPlayerDevice::TOF_INTERFACE_NAME;


    QString selectedCam= QString::fromStdString(mitk::ToFCameraMITKPlayerDevice::TOF_INTERFACE_NAME);
    connect( m_Controls->m_DeviceList, SIGNAL(ServiceSelectionChanged()), this, SLOT(OnConnectCamera(selectedCam)));

    //connect( m_Controls->m_SelectCameraCombobox, SIGNAL(currentIndexChanged(const QString)), this, SLOT(OnSelectCamera(const QString)) );
    //connect( m_Controls->m_SelectCameraCombobox, SIGNAL(activated(const QString)), this, SLOT(OnSelectCamera(const QString)) );
    //connect( m_Controls->m_SelectCameraCombobox, SIGNAL(activated(const QString)), this, SIGNAL(ToFCameraSelected(const QString)) );
    //todo klären was das hier war
  }
}

void QmitkToFConnectionWidget2::ShowParameterWidget()
{
  //QString selectedCamera = m_Controls->m_SelectCameraCombobox->currentText();
  //this->OnSelectCamera(selectedCamera); //diese Zeile nutzen, aber der Text muss von Keno's Widget kommen.
}

mitk::ToFImageGrabber* QmitkToFConnectionWidget2::GetToFImageGrabber()
{
  return this->m_ToFImageGrabber;
}

void QmitkToFConnectionWidget2::OnSelectCamera(const QString selectedCamera)
{
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

    //this->m_ToFImageGrabber->SetCameraDevice() //todo device aus keno's widget holen und hier setzen---------------------------------

    //  this->m_ToFImageGrabber = mitk::ToFImageGrabber::New();
    //  m_ToFImageGrabber->SetCameraDevice(device);

    QString selectedCamera;   //------------------------------------------------------------------------------Defining an empty String ?

    //Activation of "PlayerMode". If the selectedCamera String contains "Player", we start the Player Mode-------------------Should be checked soon
    if (selectedCamera.contains("Player"))
    {
      playerMode = true;
      //IF PMD-Player selected
      if (selectedCamera== "PMD Player")
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
    { //... open a QFileDialog to chose the corresponding file from the disc
      tmpFileName = QFileDialog::getOpenFileName(NULL, "Play Image From...", "", fileFilter);

      //If no fileName is returned by the Dialog,Button and Widget have to return to default + Opening a MessageBox
      if (tmpFileName.isEmpty())
      {
        m_Controls->m_ConnectCameraButton->setChecked(false);
        m_Controls->m_ConnectCameraButton->setEnabled(true);
        m_Controls->m_DeviceList->setEnabled(true);           //Reactivating ServiceListWidget

        this->ShowParameterWidget();
        QMessageBox::information( this, "Template functionality", "Please select a valid image before starting some action.");
        return;
      }

      if(selectedCamera == "PMD Player") //if current device name enhält
      { //set the PMD file name
        this->m_ToFImageGrabber->SetStringProperty("PMDFileName", tmpFileName.toStdString().c_str() );
      }

      if (selectedCamera == "PMD Raw Data Player" || selectedCamera == "MITK Player") //todo eventuell default
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
          //todo keno widet aktivieren
          this->ShowParameterWidget();
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
      m_Controls->m_DeviceList->setEnabled(true);           //Reactivating ServiceListWidget
      this->ShowParameterWidget();
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
    this->ShowParameterWidget();

    // send disconnect signal to the caller functionality
    emit ToFCameraDisconnected();
  }
}
