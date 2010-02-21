/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2009-03-21 19:27:37 +0100 (Sa, 21 Mrz 2009) $
Version:   $Revision: 16719 $ 
 
Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkImageGuidedTherapyTutorialView.h"

#include "QmitkStdMultiWidget.h"
#include "QmitkStdMultiWidgetEditor.h"

#include "mitkNDIPassiveTool.h"
#include "mitkNDITrackingDevice.h"
#include "mitkVirtualTrackingDevice.h"
#include "mitkStandardFileLocations.h"
#include "mitkSerialCommunication.h"
#include "mitkCone.h"

#include <QTimer>
#include <QMessageBox>


const std::string QmitkImageGuidedTherapyTutorialView::VIEW_ID = "org.mitk.views.imageguidedtherapytutorial";

QmitkImageGuidedTherapyTutorialView::QmitkImageGuidedTherapyTutorialView()
: QmitkFunctionality(), 
  m_Controls(NULL),  m_MultiWidget(NULL), m_Source(NULL), m_Visualizer(NULL), m_Timer(NULL)
{
}


QmitkImageGuidedTherapyTutorialView::~QmitkImageGuidedTherapyTutorialView()
{
}


void QmitkImageGuidedTherapyTutorialView::CreateQtPartControl(QWidget *parent)
{
  if (!m_Controls)
  {
    // create GUI widget
    m_Controls = new Ui::QmitkImageGuidedTherapyTutorialViewControls;
    m_Controls->setupUi(parent);
    this->CreateConnections();
  }
}


void QmitkImageGuidedTherapyTutorialView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
  m_MultiWidget = &stdMultiWidget;
}


void QmitkImageGuidedTherapyTutorialView::StdMultiWidgetNotAvailable()
{
  m_MultiWidget = NULL;
}


void QmitkImageGuidedTherapyTutorialView::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->m_StartButton), SIGNAL(clicked()),(QObject*) this, SLOT(OnStartIGT()));
    connect( (QObject*)(m_Controls->m_StopButton), SIGNAL(clicked()),(QObject*) this, SLOT(OnStopIGT()));
  }
}


void QmitkImageGuidedTherapyTutorialView::Activated()
{
  QmitkFunctionality::Activated();
}


void QmitkImageGuidedTherapyTutorialView::Deactivated()
{
  QmitkFunctionality::Deactivated();
}


void QmitkImageGuidedTherapyTutorialView::OnStartIGT()
{
  //This method is called when when the Do IGT button is pressed. Any kind of navigation application will 
  //start with the connection to a tracking system and as we do image guided procedures we want to show 
  //something on the screen. In this tutorial we connect to the NDI Polaris tracking system and we will 
  //show the movement of a tool as cone in MITK.

  //Check if we have a widget for visualization. Makes no sense to start otherwise.
  //If there is no multiwidget, create one.
  //if (m_MultiWidget == NULL)
  //{
  //}
  if (m_MultiWidget == NULL) // if creating the multiwidget failed, stop here.
  {
    QMessageBox::warning ( NULL, "Error", "Starting the tutorial is not possible without an initialized "
      "rendering widget. Please load a dataset first.");
    return;
  }

  try
  {
/**************** Variant 1: Use a NDI Polaris Tracking Device ****************/
    ////Here we want to use the NDI Polaris tracking device. Therefore we instantiate a object of the class
    ////NDITrackingDevice and make some settings which are necessary for a proper connection to the device. 
    //mitk::NDITrackingDevice::Pointer tracker = mitk::NDITrackingDevice::New();  //instantiate
    //tracker->SetPortNumber(mitk::SerialCommunication::COM4); //set the comport
    //tracker->SetBaudRate(mitk::SerialCommunication::BaudRate115200); //set the baud rate
    //tracker->SetType(mitk::NDIPolaris); //set the type there you can choose between Polaris and Aurora

    ////The tools represent the sensors of the tracking device. In this case we have one pointer tool.
    ////The TrackingDevice object it self fills the tool with data. So we have to add the tool to the
    ////TrackingDevice object.
    //mitk::NDIPassiveTool::Pointer tool = mitk::NDIPassiveTool::New(); 
    //tool->SetToolName("MyInstrument"); //Every tool should have a name.
    //tool->LoadSROMFile("c:\\myinstrument.rom"); //The Polaris system needs a ".rom" file which describes
    ////the geometry of the markers related to the tool tip.
    ////NDI provides an own software (NDI architect) to 
    ////generate those files.

    ////tool->LoadSROMFile(mitk::StandardFileLocations::GetInstance()->FindFile("myToolDefinitionFile.srom").c_str());
    //tracker->Add6DTool(tool); //Add the tool to the TrackingDevice object.
/**************** End of Variant 1 ****************/

/**************** Variant 2: Emulate a Tracking Device with mitk::VirtualTrackingDevice ****************/
    // For tests, it is useful to simulate a tracking device in software. This is what mitk::VirtualTrackingDevice does.
    // It will produce random position, orientation and error values for each tool that is added.
    mitk::VirtualTrackingDevice::Pointer tracker = mitk::VirtualTrackingDevice::New(); // create virtual tracker
    mitk::ScalarType bounds[] = {0.0, 200.0, 0.0, 200.0, 0.0, 200.0};
    tracker->SetBounds(bounds);
    tracker->AddTool("MyInstrument");      // add a tool to tracker
/**************** End of Variant 2 ****************/

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
    m_Source->Connect();                  //Now we connect to the tracking system.
                                          //Note we do not call this on the TrackingDevice object


    //As we wish to visualize our tool we need to have a PolyData which shows us the movement of our tool.
    //Here we take a cone shaped PolyData. In MITK you have to add the PolyData as a node into the DataStorage
    //to show it inside of the rendering windows. After that you can change the properties of the cone
    //to manipulate rendering, e.g. the position and orientation as in our case.
    mitk::Cone::Pointer cone = mitk::Cone::New();                 //instantiate a new cone
    float scale[] = {10.0, 10.0, 10.0}; 
    cone->GetGeometry()->SetSpacing(scale);                       //scale it a little that so we can see something
    mitk::DataTreeNode::Pointer node = mitk::DataTreeNode::New(); //generate a new node to store the cone into 
                                                                  //the DataStorage.
    node->SetData(cone);                          //The data of that node is our cone.
    node->SetName("My tracked object");           //The node has additional properties like a name
    node->SetColor(1.0, 0.0, 0.0);                //or the color. Here we make it red.
    this->GetDefaultDataStorage()->Add(node);     //After adding the Node with the cone in it to the
                                                  //DataStorage, MITK will show the cone in the
                                                  //render windows.

    //For updating the render windows we use another filter of the MITK-IGT pipeline concept. The 
    //NavigationDataObjectVisualizationFilter needs as input a NavigationData and a
    //PolyData. In our case the input is the source and the PolyData our cone.

    //First we create a new filter for the visualization update.
    m_Visualizer = mitk::NavigationDataObjectVisualizationFilter::New();
    m_Visualizer->SetInput(0, m_Source->GetOutput(0));        //Then we connect to the pipeline.
    m_Visualizer->SetRepresentationObject(0, cone);  //After that we have to assign the cone to the input

    //Now this simple pipeline is ready, so we can start the tracking. Here again: We do not call the 
    //StartTracking method from the tracker object itself. Instead we call this method from our source.
    m_Source->StartTracking(); 

    //Now every call of m_Visualizer->Update() will show us the cone at the position and orientation
    //given from the tracking device.
    //We use a QTimer object to call this Update() method in a fixed interval.
    if (m_Timer == NULL)
    {
      m_Timer = new QTimer(this);  //create a new timer
    }    
    connect(m_Timer, SIGNAL(timeout()), this, SLOT(OnTimer())); //connect the timer to the method OnTimer()

    m_Timer->start(100);  //Every 100ms the method OnTimer() is called. -> 10fps
                          //Now have look at the OnTimer() method.
  }
  catch (std::exception& e)
  {
    // add cleanup
    std::cout << "Error in QmitkIGTTutorial::OnDoIGT():" << e.what() << std::endl;
  }
}


void QmitkImageGuidedTherapyTutorialView::OnTimer()
{
  //Here we call the Update() method from the Visualization Filter. Internally the filter checks if
  //new NavigationData is available. If we have a new NavigationData the cone position and orientation
  //will be adapted.
  m_Visualizer->Update();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();  //update the render windows
}


void QmitkImageGuidedTherapyTutorialView::OnStopIGT()
{
  //This method is called when the Stop button is pressed. Here we disconnect the pipeline.
  if (m_Timer == NULL)
  {
    std::cout << "No Timer was set yet!" << std::endl;
    return;
  }
  //To disconnect the pipeline in a save way we first stop the timer than we disconnect the tracking device.
  //After that we destroy all filters with changing them to NULL.
  m_Timer->stop();
  disconnect(m_Timer, SIGNAL(timeout()), this, SLOT(OnTimer()));
  m_Timer = NULL;
  m_Source->StopTracking();
  m_Source->Disconnect();
  m_Source = NULL;
  m_Visualizer = NULL;
  m_Source = NULL;
  this->GetDefaultDataStorage()->Remove(this->GetDefaultDataStorage()->GetNamedNode("My tracked object"));
}
