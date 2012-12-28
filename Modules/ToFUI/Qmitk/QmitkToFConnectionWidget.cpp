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

//itk headers
#include <itksys/SystemTools.hxx>

//Setting the View_ID
const std::string QmitkToFConnectionWidget::VIEW_ID = "org.mitk.views.qmitktofconnectionwidget2";

//Constructor of QmitkToFConnectionWidget
QmitkToFConnectionWidget::QmitkToFConnectionWidget(QWidget* parent, Qt::WindowFlags f): QWidget(parent, f)
{
  this->m_IntegrationTime = 0;
  this->m_ModulationFrequency = 0;
  this->m_ToFImageGrabber = mitk::ToFImageGrabber::New();
  //Setting m_Controls= NULL on Startup-> CreateQtPartControl will generate the Gui Widget if !m_Controls
  m_Controls = NULL;
  //Calling CreateQtPartControl
  CreateQtPartControl(this);
}

//Destructor of QmitkToFConnectionWidget
QmitkToFConnectionWidget::~QmitkToFConnectionWidget()
{
  //Keno´s MitkServiceListWidget must not be deinizialized here. Qmitk methods destroy their children automatically before self-destruction
}

void QmitkToFConnectionWidget::CreateQtPartControl(QWidget *parent)   //Definition of CreateQtPartControll-Methode in QmitkToFConnectionWidget; Input= Pointer
{
  if (!m_Controls) //Define if not alreaddy exists
  {
    // create GUI widgets
    m_Controls = new Ui::QmitkToFConnectionWidgetControls2;
    m_Controls->setupUi(parent);
    //and hide them on startup
    this->HideAllParameterWidgets();

    // initzializing MitkServiceListWidget here
    std::string empty= "";
    m_Controls->m_DeviceList->Initialize<mitk::ToFCameraDevice>("ToFDeviceName", empty);// the empty could just be any kind of filter

    this->CreateConnections();
    //OnSelectCamera(); //todo: warum geht das hier nicht? -> Sasch fragen
  }
}
//Creating the SIGNAL-SLOT-Connectuions
void QmitkToFConnectionWidget::CreateConnections()
{
  if ( m_Controls )
  {
    //ConnectCameraButton as a trigger for OnConnectCamera()
    connect( (QObject*)(m_Controls->m_ConnectCameraButton), SIGNAL(clicked()),(QObject*) this, SLOT(OnConnectCamera()) );

    //QmitkServiceListWidget::ServiceSelectionChanged as a Signal for the OnSlectCamera() slot
    connect( m_Controls->m_DeviceList, SIGNAL(ServiceSelectionChanged(mitk::ServiceReference)), this, SLOT(OnSelectCamera()));

    /*Creating an other Datanode structur for Kinect is done here: As soon as a Kinect is connected, the KinectParameterWidget is enabled,
    which can be used to trigger the KinectAcqusitionModeChanged-Method, to create a working Data-Node-structure*/
    connect( m_Controls->m_KinectParameterWidget, SIGNAL(AcquisitionModeChanged()), this, SIGNAL(KinectAcquisitionModeChanged()) );

  }
}

mitk::ToFImageGrabber::Pointer QmitkToFConnectionWidget::GetToFImageGrabber()
{
  return m_ToFImageGrabber;
}

//The OnSelectCamer-Method is in charge of activating the appropiate ParameterWidgets
void QmitkToFConnectionWidget::OnSelectCamera()
{
  //Here we are getting our decvie through the QmitkServiceListWidget-Instance m_DeviceList through the GetSelectedService-Method
  mitk::ToFCameraDevice* device = m_Controls->m_DeviceList->GetSelectedService<mitk::ToFCameraDevice>();

  //getting the selectedCamera through a static Method used to transform the device->GetNameOfClass
  QString selectedCamera = QString::fromStdString(device->GetNameOfClass());

  this->HideAllParameterWidgets();
  //reactivating the Widgets on slecting a device
  if (selectedCamera.contains("PMD")) //Check if selectedCamera string contains ".." for each device
  {
    this->m_Controls->m_PMDParameterWidget->show(); //and activate the correct widget
  }
  else if (selectedCamera.contains("MESA"))
  {
    this->m_Controls->m_MESAParameterWidget->show();
  }
  else if (selectedCamera.contains("Kinect"))
  {
    this->m_Controls->m_KinectParameterWidget->show();
  }
  emit  (selectedCamera);
}
//This Methods hides all Widgets (later each widget is activated on its own)
void QmitkToFConnectionWidget::HideAllParameterWidgets()
{
  this->m_Controls->m_PMDParameterWidget->hide();
  this->m_Controls->m_MESAParameterWidget->hide();
  this->m_Controls->m_KinectParameterWidget->hide();
}

//OnConnectCamera-Method; represents one of the main parts of ToFConnectionWidget2.
void QmitkToFConnectionWidget::OnConnectCamera()
{
  //Introducing the boolean variable playerMode and set it to false by default
  bool playerMode = false;

  //After connecting a device
  if (m_Controls->m_ConnectCameraButton->text()=="Connect")
  {

    //Reset the status of some GUI-Elements
    m_Controls->m_ConnectCameraButton->setEnabled(false); //ConnectCameraButton gets disabled, what leads to other changes later
    m_Controls->m_DeviceList->setEnabled(false);          //Deactivating the Instance of QmitkServiceListWidget
    //repaint the widget
    this->repaint();

    QString tmpFileName("");
    QString fileFilter("");

    //Getting the device- and the slectedCamera-variables using the ServiceListWidget as we did it in the CameraSelect-Method
    mitk::ToFCameraDevice* device = m_Controls->m_DeviceList->GetSelectedService<mitk::ToFCameraDevice>();
    QString selectedCamera = QString::fromStdString(device->GetNameOfClass());

    emit ToFCameraSelected(selectedCamera);

    //Creating a new  instance of m_ToFImageGrabber
    this->m_ToFImageGrabber = NULL;
    this->m_ToFImageGrabber = mitk::ToFImageGrabber::New();

    //Feeding it with the Info from ServiceListWidget
    this->m_ToFImageGrabber->SetCameraDevice(device);

    // Calling Alex FixForKinect, if the Kinect is selected
    if (selectedCamera.contains("Kinect") )
    {
      MITK_INFO<< "Kinect is connected here";
      //If the particular property is selected, the suitable data-node will be generated
      this->m_ToFImageGrabber->SetBoolProperty("RGB", m_Controls->m_KinectParameterWidget->IsAcquisitionModeRGB());//--------------------------------------------------------
      this->m_ToFImageGrabber->SetBoolProperty("IR", m_Controls->m_KinectParameterWidget->IsAcquisitionModeIR());
    }

//Activation of "PlayerMode". If the selectedCamera String contains "Player", we start the Player Mode
    if (selectedCamera.contains("Player"))
    {
      playerMode = true;
      //IF PMD-Player selected
      if (selectedCamera.contains("PMD"))
      {
        fileFilter.append("PMD Files (*.pmd)");   //And seting the corresponding fileFilter
      }
      else
      {
        fileFilter.append("NRRD Images (*.nrrd);;PIC Images - deprecated (*.pic)");
      }
    }

    // if a player was selected, the playerMode-variable is true, and we will enter the following code
    if (playerMode)
    {
      //open a QFileDialog to chose the corresponding file from the disc
      tmpFileName = QFileDialog::getOpenFileName(NULL, "Play Image From...", "", fileFilter);

      //If no fileName is returned by the Dialog,Button and Widget have to return to default(disconnected) + Opening a MessageBox
      if (tmpFileName.isEmpty())
      {
        m_Controls->m_ConnectCameraButton->setChecked(false);
        m_Controls->m_ConnectCameraButton->setEnabled(true);  //re-enabling the ConnectCameraButton
        m_Controls->m_DeviceList->setEnabled(true);           //Reactivating ServiceListWidget

        this->OnSelectCamera();       //Calling the OnSelctCamera-Method -> Hides all Widget and just activates the needed ones
        QMessageBox::information( this, "Template functionality", "Please select a valid image before starting some action.");
        return;
      }

      if(selectedCamera.contains("PMDPlayer"))  //If PMD-Player is selected, set ToFImageGrabberProperty correspondingly
      {
        this->m_ToFImageGrabber->SetStringProperty("PMDFileName", tmpFileName.toStdString().c_str() );
      }
      else    //Default action
      {
        std::string msg = "";
        try
        {
          //get 3 corresponding file names
          std::string dir = itksys::SystemTools::GetFilenamePath( tmpFileName.toStdString() );
          std::string baseFilename = itksys::SystemTools::GetFilenameWithoutLastExtension( tmpFileName.toStdString() );
          std::string extension = itksys::SystemTools::GetFilenameLastExtension( tmpFileName.toStdString() );

          //"Incorrect format"-warning while using .nrrd or .pic files
          if (extension != ".pic" && extension != ".nrrd")
          {
            msg = msg + "Invalid file format, please select a \".nrrd\"-file";
            throw std::logic_error(msg.c_str());
          }

          //Checking for npos. If available, check for the Amplitude-, Intensity- and RGBImage

          int found = baseFilename.rfind("_DistanceImage");  //Defining "found" variable+checking if baseFilname contains "_DistanceImage". If not, found == npos(0)

          if (found == std::string::npos)  //If found =0
          {
            found = baseFilename.rfind("_AmplitudeImage");  //If "_AmplitudeImage" is found, the found variable is 1-> the next if statment is false
          }

          if (found == std::string::npos)
          {
            found = baseFilename.rfind("_IntensityImage"); //found = true if baseFilename cotains "_IntesityImage"
          }

          if (found == std::string::npos)
          {
            found = baseFilename.rfind("_RGBImage");
          }

          if (found == std::string::npos) //If none of the Nodes is found, display an error
          {
            msg = msg + "Input file name must end with \"_DistanceImage\", \"_AmplitudeImage\", \"_IntensityImage\" or \"_RGBImage\"!";
            throw std::logic_error(msg.c_str());
          }

          std::string baseFilenamePrefix = baseFilename.substr(0,found);//Set the baseFilenamePrefix as a substring from baseFilname

          //Set corresponding FileNames
          std::string distanceImageFileName = dir + "/" + baseFilenamePrefix + "_DistanceImage" + extension; //Set the name as: directory+FilenamePrefix+""+extension
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
          m_Controls->m_DeviceList->setEnabled(true);
          this->OnSelectCamera();
          return;
        }
      }
    }
//End "PlayerMode"

    m_Controls->m_ConnectCameraButton->setText("Disconnect");  //Reset the ConnectCameraButton to disconnected

    //if a connection could be established
    try
    {

    if (this->m_ToFImageGrabber->ConnectCamera())
    {
      this->m_Controls->m_PMDParameterWidget->SetToFImageGrabber(this->m_ToFImageGrabber);
      this->m_Controls->m_MESAParameterWidget->SetToFImageGrabber(this->m_ToFImageGrabber);
      this->m_Controls->m_KinectParameterWidget->SetToFImageGrabber(this->m_ToFImageGrabber);

      //Activating the respective widgets
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
      else
      {
        this->HideAllParameterWidgets();
      }
      // send connect signal to the caller functionality
      emit ToFCameraConnected();
    }
    else
        //##### TODO: Remove this else part once all controllers are throwing exceptions
        //if they cannot to any device!
    {
        //Throw an error if the Connection failed and reset the Widgets <- better catch an exception!
      QMessageBox::critical( this, "Error", "Connection failed. Check if you have installed the latest driver for your system." );
      m_Controls->m_ConnectCameraButton->setChecked(false);
      m_Controls->m_ConnectCameraButton->setEnabled(true);
      m_Controls->m_ConnectCameraButton->setText("Connect");
      m_Controls->m_DeviceList->setEnabled(true);           //Reactivating ServiceListWidget
      this->OnSelectCamera();
      return;
    }
    }catch(std::exception &e)
    {
        //catch exceptions of camera which cannot connect give a better reason
        QMessageBox::critical( this, "Connection failed.", e.what() );
        m_Controls->m_ConnectCameraButton->setChecked(false);
        m_Controls->m_ConnectCameraButton->setEnabled(true);
        m_Controls->m_ConnectCameraButton->setText("Connect");
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
