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

#include "QmitkIGTExample.h"
#include "QmitkIGTExampleControls.h"
#include <qaction.h>
#include "icon.xpm"
#include "QmitkTreeNodeSelector.h"
#include "QmitkStdMultiWidget.h"
#include "mitkStatusBar.h"
#include "mitkProgressBar.h"
#include <qtextedit.h>
#include <qpushbutton.h>
#include "mitkNDITrackingDevice.h"
#include "mitkNDIPassiveTool.h"
#include "mitkVector.h"
#include <itksys/SystemTools.hxx>
#include "mitkTrackingTypes.h"
#include "mitkVector.h"
#include "mitkProperties.h"
#include "mitkPropertyList.h"
#include "mitkSerialCommunication.h"
#include <vtkConeSource.h>
#include <mitkCone.h>


QmitkIGTExample::QmitkIGTExample(QObject *parent, const char *name, QmitkStdMultiWidget *mitkStdMultiWidget, mitk::DataTreeIteratorBase* it)
    : QmitkFunctionality(parent, name, it), m_MultiWidget(mitkStdMultiWidget), m_Controls(NULL)
{
  SetAvailability(true);
}


QmitkIGTExample::~QmitkIGTExample()
{}


QWidget * QmitkIGTExample::CreateMainWidget(QWidget *parent)
{
  if ( m_MultiWidget == NULL )
  {
    m_MultiWidget = new QmitkStdMultiWidget( parent );
  }
  return m_MultiWidget;
}


QWidget * QmitkIGTExample::CreateControlWidget(QWidget *parent)
{
  if (m_Controls == NULL)
  {
    m_Controls = new QmitkIGTExampleControls(parent);
    m_Controls->m_TextOutput->setTextFormat(Qt::PlainText);
    out = m_Controls->m_TextOutput;
    mitk::Vector3D offset;
    if (GetFunctionalityOptionsList()->GetPropertyValue<mitk::Vector3D>("NavigationDataDisplacementFilter_Offset", offset) == true)
    {
      m_Controls->SetDisplacementFilterParameters(GetFunctionalityOptionsList());
      out->append("found offset value in persistence storage");
    }
  }
  return m_Controls;
}


void QmitkIGTExample::CreateConnections()
{
  if ( m_Controls )
  {
    connect( (QObject*)(m_Controls->m_StartTrackingButton), SIGNAL(clicked()),(QObject*) this, SLOT(OnTestTracking()));  // execute tracking test code
    connect( (QObject*)(m_Controls->m_StartNavigationButton), SIGNAL(clicked()),(QObject*) this, SLOT(OnTestNavigation())); // build and initialize navigation filter pipeline
    connect( (QObject*)(m_Controls->m_MeasureBtn), SIGNAL(clicked()),(QObject*) this, SLOT(OnMeasure())); // execute navigation filter pipeline to read transformed tracking data
    connect( (QObject*)(m_Controls->m_StopBtn), SIGNAL(clicked()),(QObject*) this, SLOT(OnStop())); // cleanup navigation filter pipeline
    connect( (QObject*)(m_Controls), SIGNAL(ParametersChanged()),(QObject*) this, SLOT(OnParametersChanged()));  // update filter parameters with values from the GUI widget
  }
}


QAction * QmitkIGTExample::CreateAction(QActionGroup *parent)
{
  QAction* action;
  action = new QAction( tr( "Example IGT functionality" ), QPixmap((const char**)icon_xpm), tr( "QmitkIGTExample menu" ), 0, parent, "QmitkIGTExample" );
  return action;
}


void QmitkIGTExample::TreeChanged()
{
}


void QmitkIGTExample::Activated()
{
  QmitkFunctionality::Activated();
}



void QmitkIGTExample::OnTestTracking() 
{  
  WaitCursorOn(); 
  mitk::StatusBar::GetInstance()->DisplayText("Executing test of the tracking component", 4000);

  /* Create & set up tracking device*/
  mitk::NDITrackingDevice::Pointer tracker = mitk::NDITrackingDevice::New();
  /* choose between mitk::NDIPolaris, mitk::NDIAurora, mitk::ClaronMicron, mitk::IntuitiveDaVinci and mitk::AscensionMicroBird*/
  tracker->SetType(mitk::NDIAurora);
  switch (tracker->GetType())
  {
  case mitk::NDIPolaris:
    {
      tracker->SetPortNumber(mitk::SerialCommunication::COM4);
      tracker->SetBaudRate(mitk::SerialCommunication::BaudRate115200); 
      out->append("creating NDI Tracker on COM 4 with 115200 Baud");

      /* create a tool and add to tracking device */
      mitk::NDIPassiveTool::Pointer tool1 = mitk::NDIPassiveTool::New();
      tool1->SetToolName("MyInstrument");
      tool1->LoadSROMFile("c:\\myinstrument.rom");
      tool1->SetTrackingPriority(mitk::Dynamic);
      tracker->Add6DTool(tool1);
      out->append("adding tool 'MyInstrument' with rom file 'c:\\myinstrument.rom'");
      break;
    }
  case mitk::NDIAurora:
    {
      tracker->SetPortNumber(mitk::SerialCommunication::COM1);
      out->append("creating NDI Tracker on COM 1");
      break;
    }
  }

  /*open the connection, load tools that are connected (active Tools) and initialize them. */
  out->append("opening connection to tracking device");
  if (tracker->OpenConnection() == false)
  {
    out->append(QString("ERROR during OpenConnection(): ") + QString(tracker->GetErrorMessage()));
    tracker->CloseConnection();
    WaitCursorOff();
    return;
  }
  else
    out->append("successfully connected to tracking device.");

  /* beep a few times, just because we can */
  out->append("BEEP!");
  tracker->Beep(3);

  /* Start tracking */ 
  if (tracker->StartTracking() == false)
  {
    out->append(QString("ERROR during StartTracking(): ") + QString(tracker->GetErrorMessage()));
    tracker->CloseConnection();
    WaitCursorOff();
    return;
  }
  else
    out->append("tracking device is tracking now.");

  /* read tracking data 50 times */
  out->append("Starting to read tracking data for all tools.");
  mitk::TrackingTool* t = NULL;
  mitk::Point3D pos;
  mitk::Quaternion quat;  
  for(int i=0; i< 50; i++)  // 50x
  {
    for (unsigned int i = 0; i < tracker->GetToolCount(); i++) // each tool
    {
      std::stringstream output;
      t = tracker->GetTool(i);
      if (t == NULL)
        continue;
      output << "Tool " << t->GetToolName() << ":" << std::endl;
      if (t->IsDataValid() == true)
      {
        t->GetPosition(pos);
        output << "  Position      = <" << pos[0] << ", " << pos[1] << ", " << pos[2] << ">" << std::endl; 
        t->GetQuaternion(quat);
        output << "  Orientation   = <" << quat[0] << ", " << quat[1] << ", " << quat[2] << ", " << quat[3] << ">" << std::endl;
        output << "  TrackingError = " << t->GetTrackingError() << std::endl;
      }
      else
        output << "  Data is invalid. Error message: " << t->GetErrorMessage() << std::endl;
      output << "--------------------------------------------" << std::endl;
      out->append(output.str().c_str()); // append string stream content to gui widget
    }
    //wait a little to get the next coordinate
    itksys::SystemTools::Delay(100);
  }
  
  /* Stop tracking */ 
  out->append("Enough tracking data. Stopping tracking now.");
  if (tracker->StopTracking() == false)
  {
    out->append(QString("ERROR during StopTracking(): ") + QString(tracker->GetErrorMessage()));
    tracker->CloseConnection();
    WaitCursorOff();
    return;
  }
  else
    out->append("Tracking stopped.");

  /* Stop tracking */ 
  if (tracker->CloseConnection() == false)
  {
    out->append(QString("ERROR during CloseConnection(): ") + QString(tracker->GetErrorMessage()));  
    WaitCursorOff();
    return;
  }
  else
    out->append("tracking stopped.");

  mitk::StatusBar::GetInstance()->DisplayText("test of the tracking component finished", 2000);
  WaitCursorOff();  // restore normal mouse cursor after you finished
}


void QmitkIGTExample::OnTestNavigation() 
{
  WaitCursorOn(); // always good to show the user that the application is processing and will not react to user input for a while
  mitk::StatusBar::GetInstance()->DisplayText("Executing test of the navigation component", 4000);  // tell the user what you are doing

  /* Create & set up tracking device with a tool */
  mitk::NDITrackingDevice::Pointer tracker = mitk::NDITrackingDevice::New();
  
  /* choose between mitk::NDIPolaris, mitk::NDIAurora, mitk::ClaronMicron, mitk::IntuitiveDaVinci and mitk::AscensionMicroBird*/
  tracker->SetType(mitk::NDIAurora);
  mitk::NDIPassiveTool::Pointer tool1;
  switch (tracker->GetType())
  {
  case mitk::NDIPolaris:
    {
      tracker->SetPortNumber(mitk::SerialCommunication::COM4);
      tracker->SetBaudRate(mitk::SerialCommunication::BaudRate115200); 
      out->append("creating NDI Tracker on COM 4 with 115200 Baud");

      tool1 = mitk::NDIPassiveTool::New();
      tool1->SetToolName("MyInstrument");
      tool1->LoadSROMFile("c:\\myinstrument.rom");
      tool1->SetTrackingPriority(mitk::Dynamic);
      tracker->Add6DTool(tool1);
      out->append("adding tool 'MyInstrument' with rom file 'c:\\myinstrument.rom'");
      break;
    }
  case mitk::NDIAurora:
    {
      tracker->SetPortNumber(mitk::SerialCommunication::COM1);
      out->append("creating NDI Tracker on COM 1");
      break;
    }
  }

  /* Now set up pipeline */
  try
  {
    m_Source = mitk::TrackingDeviceSource::New();  // we need the filter objects to stay alive, therefore they must be members
    m_Source->SetTrackingDevice(tracker);
    out->append("created and initialized TrackingDeviceSource filter");

    //connect to tracking device
    out->append("connected filter pipeline");
    m_Source->Connect();

    m_Displacer = mitk::NavigationDataDisplacementFilter::New();
    mitk::Vector3D offset;
    /* check if there is a Offset parameter stored in our propertylist. If none is found, use hardcoded value */
    if (GetFunctionalityOptionsList()->GetPropertyValue<mitk::Vector3D>("NavigationDataDisplacementFilter_Offset", offset) == false)  // search for Offset parameter
    {
      mitk::FillVector3D(offset, 100000.0, 0.0, 0.0);  // nothing found, use hardcoded value
      GetFunctionalityOptionsList()->SetProperty("NavigationDataDisplacementFilter_Offset", mitk::Vector3DProperty::New(offset));  // add the property to the list
    }
    m_Displacer->SetOffset(offset);
    /* --> Instead, we could have just called m_Displacer->SetParameters(GetFunctionalityOptionsList()) to set all stored parameters at once.
       But then we would have to check, if the PropertyList contains the parameters ( they were stored in the list before by the 
       persistence mechanism or by the GUI Event ParametersChanged that calls the OnParametersChanged() method)
    */

    out->append(QString("created and initialized NavigationDataDisplacementFilter filter using <%1, %2, %3> as offset").arg(offset[0]).arg(offset[1]).arg(offset[2]));
    for (unsigned int i =0; i<m_Source->GetNumberOfOutputs(); i++)
      m_Displacer->SetInput(i, m_Source->GetOutput(i));  // connect filter

    //Now we create a visualization filter object to hang up the tools into the datatree and visualize them in the widgets.
    mitk::NavigationDataVisualizationByBaseDataTransformFilter::Pointer visualizer = mitk::NavigationDataVisualizationByBaseDataTransformFilter::New();
    for (unsigned int i =0; i<m_Displacer->GetNumberOfOutputs(); i++)
      visualizer->SetInput(i, m_Displacer->GetOutput(i));
    
    //create new BaseData for each tool
    for (int i = 0; i<m_Source->GetToolCount();i++)
    {
      mitk::Cone::Pointer mitkToolData = mitk::Cone::New();
      vtkConeSource* vtkData = vtkConeSource::New();
      vtkData->SetRadius(5);
      vtkData->SetHeight(10);
      vtkData->SetDirection(0.0, 0.0, 1.0);
      vtkData->SetCenter(0.0, 0.0, 0.0);
      vtkData->SetResolution(20);
      vtkData->CappingOn();
      vtkData->Update();
      mitkToolData->SetVtkPolyData(vtkData->GetOutput());
      vtkData->Delete();
      
      //create DataTreeNode
      mitk::DataTreeNode::Pointer toolNode = mitk::DataTreeNode::New();
      toolNode->SetData(mitkToolData);
      toolNode->GetPropertyList()->SetProperty("name", mitk::StringProperty::New ( "MyInstrument" ) );
      toolNode->GetPropertyList()->SetProperty("layer", mitk::IntProperty::New(0));
      toolNode->GetPropertyList()->SetProperty("visible",mitk::BoolProperty::New(true));
      toolNode->SetColor(0.0,1.0,0.0);//green
      toolNode->SetOpacity(0.8);
      toolNode->Modified();
      
      //add it to the DataStorage
      mitk::DataStorage::GetInstance()->Add(toolNode);
      out->append("added tool to data tree.");

      //add data object to visualizer
      visualizer->SetBaseData(m_Displacer->GetOutput(i), mitkToolData);
    }

    //copy to member to call update of tools on OnMeasure()
    m_EndOfPipeline = visualizer;

    //start the tracking
    m_Source->StartTracking();
    out->append("started source filter. tracking is running now.");
  }
  catch (std::exception& exp)
  {
    out->append(QString("ERROR during instantiation and initialization of TrackingDeviceSource filter: ") + QString(exp.what()));
    m_Displacer = NULL;
    m_EndOfPipeline = NULL;
    if (m_Source)
    {
      m_Source->StopTracking();
      m_Source->Disconnect();
      m_Source = NULL;
    }
    tool1 = NULL;
    tracker = NULL;
    WaitCursorOff();
    return;
  }
  /* set up GUI, so that measurements can be done*/
  m_Controls->m_MeasureBtn->show();
  m_Controls->m_MeasureBtn->setEnabled(true);
  m_Controls->m_StopBtn->show();
  m_Controls->m_StopBtn->setEnabled(true);  
  m_Controls->m_StartTrackingButton->setEnabled(false);
  m_Controls->m_StartNavigationButton->setEnabled(false);

  //Initialize the views if no data was loaded yet
  mitk::RenderingManager::GetInstance()->InitializeViews(m_DataTreeIterator.GetPointer());

  out->append("Tracking Pipeline ready. Click on Measure! button to get most current navigation data");
  WaitCursorOff();  // restore normal mouse cursor after you finished
}


void QmitkIGTExample::OnMeasure()
{  
  if (m_EndOfPipeline.IsNull())
  {
    out->append("Tracking Pipeline not ready.");
    return;
  }
  WaitCursorOn();
  
  /* Get the output of the las filter with output and print it */
  for (unsigned int i = 0; i < m_EndOfPipeline->GetNumberOfOutputs(); ++i) // for all outputs of the filter
  {
    mitk::NavigationData* nd = m_EndOfPipeline->GetOutput(i);
    if (nd == NULL)
    {
      out->append("WARNING: Navigation Data is NULL");
      continue;
    }
    nd->Update();  // update the navigation data. this will read current tracking data from tracking device

    std::stringstream output;
    output << "Navigation Data of Output " << i << ":" << std::endl;
    if (nd->IsDataValid() == true)
    {
      output << "  Position    = " << nd->GetPosition() << std::endl; 
      output << "  Orientation = <" << nd->GetOrientation() << std::endl;
      output << "  Error       = " << nd->GetCovErrorMatrix() << std::endl;
    }
    else
      output << "  Data is invalid."<< std::endl;
    output << "--------------------------------------------" << std::endl;
    out->append(output.str().c_str()); // append string stream content to gui widget
  }
  WaitCursorOff();
}


void QmitkIGTExample::OnStop()
{
  WaitCursorOn();
  m_Controls->m_MeasureBtn->hide();
  m_Controls->m_MeasureBtn->setEnabled(false);
  m_Controls->m_StopBtn->hide();
  m_Controls->m_StopBtn->setEnabled(false);  
  m_Controls->m_StartTrackingButton->setEnabled(true);
  m_Controls->m_StartNavigationButton->setEnabled(true);
  try
  {
    m_EndOfPipeline = NULL;
    m_Displacer = NULL;
    m_Source->StopTracking();
    m_Source->Disconnect();
    m_Source = NULL;
    WaitCursorOff();
  }  
  catch (std::exception& exp)
  {
    out->append(QString("ERROR during cleanup of filter pipeline: ") + QString(exp.what()));
    m_Source = NULL;
    WaitCursorOff();
    return;
  }
  out->append("Filter pipeline stopped and destroyed. Everything is back to normal."); 
  WaitCursorOff();
}


void QmitkIGTExample::OnParametersChanged()
{
  /* Check if needed objects exist */
  if (m_Controls->m_Parameters.IsNull() /*|| m_Displacer.IsNull()*/)
    return;

  /* get all filter parameters in the form of a PropertyList and pass it to the filter */
  //m_Displacer->SetParameters(m_Controls->m_Parameters.GetPointer());
  //out->append("Using GUI Parameters for Displacement Filter."); 

  /* add the filter PropertyList to the functionalities List, so that it will be saved on application exit 
     this will be restored at the next restart.
  */
  GetFunctionalityOptionsList()->ConcatenatePropertyList(m_Controls->m_Parameters.GetPointer(), true);
  out->append("Adding GUI parameters to persistence storage."); 
  m_Displacer->SetParameters(m_Controls->m_Parameters.GetPointer());
  out->append("Setting GUI parameters to filter.");
}


void QmitkIGTExample::AddToFunctionalityOptionsList(mitk::PropertyList* pl)
{
  QmitkFunctionality::AddToFunctionalityOptionsList(pl);  // let the super class add the properties

  // then initialize everything according to the properties

  if (m_Controls != NULL)
  {
    m_Controls->SetDisplacementFilterParameters(pl);  // update GUI
  }
  // MITK-IGT filters will be initialized with the properties in OnTestNavigation()
}
