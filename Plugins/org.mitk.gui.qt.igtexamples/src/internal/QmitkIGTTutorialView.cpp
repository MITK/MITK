/*============================================================================

The Medical Imaging Interaction Toolkit (MITK)

Copyright (c) German Cancer Research Center (DKFZ)
All rights reserved.

Use of this source code is governed by a 3-clause BSD license that can be
found in the LICENSE file.

============================================================================*/

#include "QmitkIGTTutorialView.h"

#include "mitkNDIPassiveTool.h"
#include "mitkNDITrackingDevice.h"
#include "mitkVirtualTrackingDevice.h"
#include "mitkStandardFileLocations.h"
#include "mitkSerialCommunication.h"
#include "mitkCone.h"

#include <QTimer>
#include <QMessageBox>

#include "mitkNDIPolarisTypeInformation.h"


const std::string QmitkIGTTutorialView::VIEW_ID = "org.mitk.views.igttutorial";

QmitkIGTTutorialView::QmitkIGTTutorialView()
: QmitkAbstractView(),
m_Controls(nullptr), m_Source(nullptr), m_Visualizer(nullptr), m_Timer(nullptr)
{
}


QmitkIGTTutorialView::~QmitkIGTTutorialView()
{
}


void QmitkIGTTutorialView::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widget
    m_Controls = new Ui::QmitkIGTTutorialViewControls;
    m_Controls->setupUi(parent);
    this->CreateConnections();
  }
}



void QmitkIGTTutorialView::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->m_StartButton), SIGNAL(clicked()),(QObject*) this, SLOT(OnStartIGT()));
    connect( (QObject*)(m_Controls->m_StopButton), SIGNAL(clicked()),(QObject*) this, SLOT(OnStopIGT()));
  }
}

void QmitkIGTTutorialView::SetFocus()
{
  m_Controls->m_virtualTrackingRadioButton->setFocus();
}

//The next line starts a snippet to display this code in the documentation. If you don't revise the documentation, don't remove it!
    //! [OnStart 1]
void QmitkIGTTutorialView::OnStartIGT()
{
  //This method is called when the "Start Image Guided Therapy" button is pressed. Any kind of navigation application will
  //start with the connection to a tracking system and as we do image guided procedures we want to show
  //something on the screen. In this tutorial we connect to the NDI Polaris tracking system pr a virtual tracking device and we will
  //show the movement of a tool as cone in MITK.

   //! [OnStart 1]

   //! [OnStart 2]
  try
  {
    if(m_Controls->m_NDITrackingRadioButton->isChecked())
    {
      /**************** Variant 1: Use a NDI Polaris Tracking Device ****************/
      //Here we want to use the NDI Polaris tracking device. Therefore we instantiate a object of the class
      //NDITrackingDevice and make some settings which are necessary for a proper connection to the device.
      MITK_INFO << "NDI tracking";
      QMessageBox::warning ( nullptr, "Warning", "You have to set the parameters for the NDITracking device inside the code (QmitkIGTTutorialView::OnStartIGT()) before you can use it.");
      mitk::NDITrackingDevice::Pointer tracker = mitk::NDITrackingDevice::New();  //instantiate
      tracker->SetPortNumber(mitk::SerialCommunication::COM4); //set the comport
      tracker->SetBaudRate(mitk::SerialCommunication::BaudRate115200); //set the baud rate
      tracker->SetType(mitk::NDIPolarisTypeInformation::GetTrackingDeviceName()); //set the type there you can choose between Polaris and Aurora

      //The tools represent the sensors of the tracking device. In this case we have one pointer tool.
      //The TrackingDevice object it self fills the tool with data. So we have to add the tool to the
      //TrackingDevice object.
      // The Polaris system needs a ".rom" file which describes the geometry of the markers related to the tool tip.
      //NDI provides an own software (NDI architect) to generate those files.
      tracker->AddTool("MyInstrument", "c:\\myinstrument.rom");
    //! [OnStart 2]
      /**************** End of Variant 1 ****************/
    //! [OnStart 3]
      //The tracking device object is used for the physical connection to the device. To use the
      //data inside of our tracking pipeline we need a source. This source encapsulate the tracking device
      //and provides objects of the type mitk::NavigationData as output. The NavigationData objects stores
      //position, orientation, if the data is valid or not and special error informations in a covariance
      //matrix.
      //
      //Typically the start of a pipeline is a TrackingDeviceSource. To work correct we have to set a
      //TrackingDevice object. Attention you have to set the tools before you set the whole TrackingDevice
      //object to the TrackingDeviceSource because the source need to know how many outputs should be
      //generated.
      m_Source = mitk::TrackingDeviceSource::New();   //We need the filter objects to stay alive,
      //therefore they must be members.
      m_Source->SetTrackingDevice(tracker); //Here we set the tracking device to the source of the pipeline.
    //! [OnStart 3]

    }
    //! [OnStart 4]
    else
    {
      /**************** Variant 2: Emulate a Tracking Device with mitk::VirtualTrackingDevice ****************/
      // For tests, it is useful to simulate a tracking device in software. This is what mitk::VirtualTrackingDevice does.
      // It will produce random position, orientation and error values for each tool that is added.
      MITK_INFO << "virtual tracking"<<endl;
      mitk::VirtualTrackingDevice::Pointer tracker = mitk::VirtualTrackingDevice::New(); // create virtual tracker

      mitk::ScalarType bounds[] = {0.0, 200.0, 0.0, 200.0, 0.0, 200.0};
      tracker->SetBounds(bounds);
      tracker->AddTool("MyInstrument");      // add a tool to tracker

      //The tracking device object is used for the physical connection to the device. To use the
      //data inside of our tracking pipeline we need a source. This source encapsulate the tracking device
      //and provides objects of the type mitk::NavigationData as output. The NavigationData objects stores
      //position, orientation, if the data is valid or not and special error informations in a covariance
      //matrix.
      //
      //Typically the start of a pipeline is a TrackingDeviceSource. To work correct we have to set a
      //TrackingDevice object. Attention you have to set the tools before you set the whole TrackingDevice
      //object to the TrackingDeviceSource because the source need to know how many outputs should be
      //generated.
      m_Source = mitk::TrackingDeviceSource::New();   //We need the filter objects to stay alive,
      //therefore they must be members.
      m_Source->SetTrackingDevice(tracker); //Here we set the tracking device to the source of the pipeline.
      /**************** End of Variant 2 ****************/
    }
    //! [OnStart 4]

    //! [OnStart 5]
    m_Source->Connect();                  //Now we connect to the tracking system.
    //Note we do not call this on the TrackingDevice object
    //! [OnStart 5]

    //! [OnStart 6]
    //As we wish to visualize our tool we need to have a PolyData which shows us the movement of our tool.
    //Here we take a cone shaped PolyData. In MITK you have to add the PolyData as a node into the DataStorage
    //to show it inside of the rendering windows. After that you can change the properties of the cone
    //to manipulate rendering, e.g. the position and orientation as in our case.
    mitk::Cone::Pointer cone = mitk::Cone::New();                 //instantiate a new cone
    double scale[] = {10.0, 10.0, 10.0};
    cone->GetGeometry()->SetSpacing(scale);                       //scale it a little that so we can see something
    mitk::DataNode::Pointer node = mitk::DataNode::New(); //generate a new node to store the cone into
    //the DataStorage.
    node->SetData(cone);                          //The data of that node is our cone.
    node->SetName("My tracked object");           //The node has additional properties like a name
    node->SetColor(1.0, 0.0, 0.0);                //or the color. Here we make it red.
    this->GetDataStorage()->Add(node);     //After adding the Node with the cone in it to the
    //DataStorage, MITK will show the cone in the
    //render windows.
    //! [OnStart 6]


    //! [OnStart 7]
    //For updating the render windows we use another filter of the MITK-IGT pipeline concept. The
    //NavigationDataObjectVisualizationFilter needs as input a NavigationData and a
    //PolyData. In our case the input is the source and the PolyData our cone.

    //First we create a new filter for the visualization update.
    m_Visualizer = mitk::NavigationDataObjectVisualizationFilter::New();
    m_Visualizer->SetInput(0, m_Source->GetOutput());        //Then we connect to the pipeline.
    m_Visualizer->SetRepresentationObject(0, cone.GetPointer()); // After that we have to assign the cone to the input

    //Now this simple pipeline is ready, so we can start the tracking. Here again: We do not call the
    //StartTracking method from the tracker object itself. Instead we call this method from our source.
    m_Source->StartTracking();
    //! [OnStart 7]

    //! [OnStart 8]
    //Now every call of m_Visualizer->Update() will show us the cone at the position and orientation
    //given from the tracking device.
    //We use a QTimer object to call this Update() method in a fixed interval.
    if (m_Timer == nullptr)
    {
      m_Timer = new QTimer(this);  //create a new timer
    }
    connect(m_Timer, SIGNAL(timeout()), this, SLOT(OnTimer())); //connect the timer to the method OnTimer()

    m_Timer->start(100);  //Every 100ms the method OnTimer() is called. -> 10fps
    //! [OnStart 8]

    //! [OnStart 8a]
    //disable the tracking device selection
    this->m_Controls->m_NDITrackingRadioButton->setDisabled(true);
    this->m_Controls->m_virtualTrackingRadioButton->setDisabled(true);
    //! [OnStart 8a]

  }
  //! [OnStart 9]
  catch (std::exception& e)
  {
    // add cleanup
    MITK_INFO << "Error in QmitkIGTTutorial::OnStartIGT():" << e.what();
  }
  //! [OnStart 9]
}

    //![OnTimer]
void QmitkIGTTutorialView::OnTimer()
{
  //Here we call the Update() method from the Visualization Filter. Internally the filter checks if
  //new NavigationData is available. If we have a new NavigationData the cone position and orientation
  //will be adapted.
  m_Visualizer->Update();

  auto geo = this->GetDataStorage()->ComputeBoundingGeometry3D(this->GetDataStorage()->GetAll());
  mitk::RenderingManager::GetInstance()->InitializeViews( geo );
  this->RequestRenderWindowUpdate();
}
    //![OnTimer]

    //![OnStop]
void QmitkIGTTutorialView::OnStopIGT()
{
  //This method is called when the Stop button is pressed. Here we disconnect the pipeline.
  if (m_Timer == nullptr)
  {
    MITK_INFO << "No Timer was set yet!";
    return;
  }
  //To disconnect the pipeline in a save way we first stop the timer than we disconnect the tracking device.
  //After that we destroy all filters with changing them to nullptr.
  m_Timer->stop();
  disconnect(m_Timer, SIGNAL(timeout()), this, SLOT(OnTimer()));
  m_Timer = nullptr;
  m_Source->StopTracking();
  m_Source->Disconnect();
  m_Source = nullptr;
  m_Visualizer = nullptr;
  m_Source = nullptr;
  this->GetDataStorage()->Remove(this->GetDataStorage()->GetNamedNode("My tracked object"));

  //enable the tracking device selection
  this->m_Controls->m_NDITrackingRadioButton->setEnabled(true);
  this->m_Controls->m_virtualTrackingRadioButton->setEnabled(true);
}
    //![OnStop]
