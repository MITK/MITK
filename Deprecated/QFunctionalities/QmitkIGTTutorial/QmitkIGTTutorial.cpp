/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date$
Version:   $Revision$

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

#include "QmitkIGTTutorial.h"
#include "QmitkIGTTutorialControls.h"
#include <qaction.h>
#include "icon.xpm"
#include "QmitkTreeNodeSelector.h"
#include "QmitkStdMultiWidget.h"
#include "mitkStatusBar.h"
#include "mitkProgressBar.h"

#include "mitkNDIPassiveTool.h"
#include "mitkNDITrackingDevice.h"
#include "mitkStandardFileLocations.h"
#include "mitkSerialCommunication.h"
#include "mitkCone.h"

#include "qtimer.h"

QmitkIGTTutorial::QmitkIGTTutorial(QObject *parent, const char *name, QmitkStdMultiWidget *mitkStdMultiWidget, mitk::DataTreeIteratorBase* it)
: QmitkFunctionality(parent, name, it), m_MultiWidget(mitkStdMultiWidget), m_Controls(NULL),
 m_Source(NULL), m_Visualizer(NULL), m_Timer(NULL)
{
  SetAvailability(true);
}


QmitkIGTTutorial::~QmitkIGTTutorial()
{}


QWidget * QmitkIGTTutorial::CreateMainWidget(QWidget *parent)
{
  if ( m_MultiWidget == NULL )
  {
    m_MultiWidget = new QmitkStdMultiWidget( parent );
  }
  return m_MultiWidget;
}


QWidget * QmitkIGTTutorial::CreateControlWidget(QWidget *parent)
{
  if (m_Controls == NULL)
  {
    m_Controls = new QmitkIGTTutorialControls(parent);
  }
  return m_Controls;
}


void QmitkIGTTutorial::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->m_StartButton), SIGNAL(clicked()),(QObject*) this, SLOT(OnDoIGT()));
    connect( (QObject*)(m_Controls->m_StopButton), SIGNAL(clicked()),(QObject*) this, SLOT(OnStop()));
  }
}


QAction * QmitkIGTTutorial::CreateAction(QActionGroup *parent)
{
  QAction* action;
  action = new QAction( tr( "Tutorial functionality for MITK-IGT" ), QPixmap((const char**)icon_xpm), tr( "QmitkIGTTutorial menu" ), 0, parent, "QmitkIGTTutorial" );
  return action;
}


void QmitkIGTTutorial::TreeChanged()
{
}


void QmitkIGTTutorial::Activated()
{
  QmitkFunctionality::Activated();
}


void QmitkIGTTutorial::OnDoIGT() 
{
  //This method is called when when the Do IGT button is pressed. Any kind of navigation application will 
  //start with the connection to a tracking system and as we do image guided procedures we want to show 
  //something on the screen. In this tutorial we connect to the NDI Polaris tracking system and we will 
  //show the movement of a tool as cone in MITK.

  try
  {
    //Here we want to use the NDI Polaris tracking device. Therefore we instantiate a object of the class
    //NDITrackingDevice and make some settings which are necessary for a proper connection to the device. 
    mitk::NDITrackingDevice::Pointer tracker = mitk::NDITrackingDevice::New();  //instatiate
    tracker->SetPortNumber(mitk::SerialCommunication::COM4); //set the comport
    tracker->SetBaudRate(mitk::SerialCommunication::BaudRate115200); //set the baudrate
    tracker->SetType(mitk::NDIPolaris); //set the type there you can choose between Polaris and Aurora

    //The tools represent the sensors of the tracking device. In this case we have one pointer tool.
    //The TrackingDevice object it self fills the tool with data. So we have to add the tool to the
    //TrackingDevice object.
    
    
    //The Polaris system needs a ".rom" file which describes
    //the geometry of the markers related to the tool tip.
    //NDI provides an own software (NDI architect) to 
    //generate those files.
    tracker->AddTool("MyInstrument", "c:\\myinstrument.rom"); //Create a tool and add it the TrackingDevice object.

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

    m_Source->SetTrackingDevice(tracker);           //Here we set the tracking device to the source of
                                                    //the pipeline.

    m_Source->Connect();                            //Now we connect to the tracking system.
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
    node->SetData(cone);                                          //The data of that node is our cone.
    node->SetName("My tracked object");                           //The node has additional properties like a name
    node->SetColor(1.0, 0.0, 0.0);                                //or the colour. Here we make it red.
    mitk::DataStorage::GetInstance()->Add(node);                  //After adding the Node with the cone in it to the
                                                                  //DataStorage, MITK will show the cone in the
                                                                  //render windows.

    //For updating the render windows we use another filter of the MITK-IGT pipeline concept. The 
    //NavigationDataVisualizationByBaseDataTransformFilter needs as input a NavigationData and a
    //PolyData. In our case the input is the source and the PolyData our cone.

    //First we create a new filter for the visualization update.
    m_Visualizer = mitk::NavigationDataObjectVisualizationFilter::New();
    m_Visualizer->SetInput(0, m_Source->GetOutput(0));        //Then we connect to the pipeline.
    m_Visualizer->SetBaseData(0, cone);  //After that we have to assign the right output 
                                                              //to our cone

    //Now this simple pipeline is ready, so we can start the tracking. Here again: We do not call the 
    //StartTracking method from the tracker object itself. Instead we call this method from our source.
    m_Source->StartTracking(); 

    //Now every call of m_Visualizer->Update() will show us the cone at the position and orientation
    //given from the tracking device.
    //We use a QTimer object to call this Update() method in a fixed intervall.
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


void QmitkIGTTutorial::OnTimer()
{
  //Here we call the Update() method from the Visualization Filter. Internally the filter checks if
  //new NavigationData is available. If we have a new NavigationData the cone position and orientation
  //will be adapted.
  m_Visualizer->Update();
  mitk::RenderingManager::GetInstance()->RequestUpdateAll();  //update the render windows
}


void QmitkIGTTutorial::OnStop()
{
  //This method is called when the Stop button is pressed. Here we disconnect the pipeline.
  if (m_Timer == NULL)
  {
    std::cout << "No Timer was set yet!" << std::endl;
    return;
  }
  //To disconnect the pipline in a save way we first stop the timer than we disconnect the tracking device.
  //After that we destroy all filters with changing them to NULL.
  m_Timer->stop();
  disconnect(m_Timer, SIGNAL(timeout()), this, SLOT(OnTimer()));
  m_Timer = NULL;
  m_Source->StopTracking();
  m_Source->Disconnect();
  m_Source = NULL;
  m_Visualizer = NULL;
  m_Source = NULL;
  mitk::DataStorage::GetInstance()->Remove(mitk::DataStorage::GetInstance()->GetNamedNode("My tracked object"));
}
