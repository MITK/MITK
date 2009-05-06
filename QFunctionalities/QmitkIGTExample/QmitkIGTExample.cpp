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
#include "icon.xpm"

#include "QmitkStdMultiWidget.h"
#include "QmitkTreeNodeSelector.h"

#include "mitkClaronTool.h"
#include "mitkClaronTrackingDevice.h"
#include "mitkCone.h"
#include "mitkInternalTrackingTool.h"
#include "mitkNDIPassiveTool.h"
#include "mitkNDITrackingDevice.h"
#include "mitkRandomTrackingDevice.h"
#include "mitkNavigationDataObjectVisualizationFilter.h"
#ifdef MITK_USE_MICROBIRD_TRACKER
#include "mitkMicroBirdTrackingDevice.h"
#endif // MITK_USE_MICROBIRD_TRACKER

#include "mitkProgressBar.h"
#include "mitkProperties.h"
#include "mitkPropertyList.h"
#include "mitkSerialCommunication.h"
#include "mitkStatusBar.h"
#include "mitkTrackingTypes.h"
#include "mitkVector.h"
#include "mitkGeometry3D.h"

#include <itksys/SystemTools.hxx>

#include <qaction.h>
#include <qcombobox.h>
#include <qpushbutton.h>
#include <qtextedit.h>
#include <qtimer.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qcheckbox.h>
#include <qprogressbar.h>
#include <qcolor.h>
#include <qwindowsstyle.h>


QmitkIGTExample::QmitkIGTExample(QObject *parent, const char *name, QmitkStdMultiWidget *mitkStdMultiWidget, mitk::DataTreeIteratorBase* it)
: QmitkFunctionality(parent, name, it), m_MultiWidget(mitkStdMultiWidget), m_Controls(NULL)
{
  SetAvailability(true);
  m_Timer = new QTimer(this);
  m_RecordingTimer = new QTimer(this);
  m_PlayingTimer = new QTimer(this);
  m_XValues.clear();
  m_YValues.clear();
}


QmitkIGTExample::~QmitkIGTExample()
{
  this->OnStop();  // cleanup IGT pipeline, if tracking is in progress
  m_Timer->stop();
  m_RecordingTimer->stop();
  m_PlayingTimer->stop();
  m_Recorder = NULL;
  m_Player = NULL;
  m_RecordingTimer = NULL;
  m_PlayingTimer = NULL;
}


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
    connect( (QObject*)(m_Controls->m_MeasureContinuously), SIGNAL(clicked()),(QObject*) this, SLOT(OnMeasureContinuously())); // execute navigation filter pipeline to read transformed tracking data
    connect( (QObject*)(m_Controls->m_StopBtn), SIGNAL(clicked()),(QObject*) this, SLOT(OnStop())); // cleanup navigation filter pipeline
    connect( (QObject*)(m_Controls), SIGNAL(ParametersChanged()),(QObject*) this, SLOT(OnParametersChanged()));  // update filter parameters with values from the GUI widget
    connect( m_Timer, SIGNAL(timeout()), this, SLOT(OnMeasure()) );
    connect( m_RecordingTimer, SIGNAL(timeout()), this, SLOT(OnRecording()) );
    connect( m_PlayingTimer, SIGNAL(timeout()), this, SLOT(OnPlaying()) );
    connect( (QObject*)(m_Controls->m_StartRecordingButton), SIGNAL(clicked()),(QObject*) this, SLOT(OnStartRecording()));  // execute tracking test code
    connect( (QObject*)(m_Controls->m_StartPlayingButton), SIGNAL(clicked()),(QObject*) this, SLOT(OnStartPlaying()));  // execute tracking test code
    connect( (QObject*)(m_Controls->m_ShowErrorPlotBtn), SIGNAL(clicked()),(QObject*) this, SLOT(OnShowErrorPlot()));  // execute tracking test code
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
    mitk::NDITrackingDevice::Pointer trak = mitk::NDITrackingDevice::New(); // TEST
  WaitCursorOn();
  mitk::StatusBar::GetInstance()->DisplayText("Executing test of the tracking component", 4000);

  /* Create & set up tracking device with a tool */
  mitk::TrackingDevice::Pointer tracker = this->ConfigureTrackingDevice();
  if (tracker.IsNull())
  {
    out->append("Error creating tracking device. Did you provide all parameters?");
    return;
  }

  /* open the connection, load tools that are connected (active Tools) and initialize them. */
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
        t->GetOrientation(quat);
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
  mitk::TrackingDevice::Pointer tracker = this->ConfigureTrackingDevice(); // configure selected tracking device

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
      mitk::FillVector3D(offset, 1.0, 1.0, 1800.0);  // nothing found, use default value
      GetFunctionalityOptionsList()->SetProperty("NavigationDataDisplacementFilter_Offset", mitk::Vector3DProperty::New(offset));  // add the property to the list
    }
    m_Displacer->SetOffset(offset);
    /* --> Instead, we could have just called m_Displacer->SetParameters(GetFunctionalityOptionsList()) to set all stored parameters at once.
    But then we would have to check, if the PropertyList contains the parameters ( they were stored in the list before by the
    persistence mechanism or by the GUI Event ParametersChanged that calls the OnParametersChanged() method)
    */

    out->append(QString("created and initialized NavigationDataDisplacementFilter filter using <%1, %2, %3> as offset").arg(offset[0]).arg(offset[1]).arg(offset[2]));
    int numberOfOutputs = m_Source->GetNumberOfOutputs();
    for (unsigned int i = 0; i < numberOfOutputs; i++)
      m_Displacer->SetInput(i , m_Source->GetOutput(i));  // connect filter

    //Now we create a visualization filter object to hang up the tools into the datatree and visualize them in the widgets.
    mitk::NavigationDataObjectVisualizationFilter::Pointer visualizer = mitk::NavigationDataObjectVisualizationFilter::New();
    for (int i = 0; i < m_Displacer->GetNumberOfOutputs(); i++)
      visualizer->SetInput(i, m_Displacer->GetOutput(i));

    //create new BaseData for each tool
    for (int i = 0; i < m_Displacer->GetNumberOfOutputs();i++)
    {
      mitk::Cone::Pointer mitkToolData = mitk::Cone::New();
      float scale[] = {20.0, 20.0, 20.0};
      mitkToolData->GetGeometry()->SetSpacing(scale);
      //create DataTreeNode
      mitk::DataTreeNode::Pointer toolNode = mitk::DataTreeNode::New();
      toolNode->SetData(mitkToolData);
      toolNode->SetName(QString("MyInstrument %1").arg(i).latin1());
      toolNode->SetColor(0.2, 0.3 * i ,0.9 - 0.2 * i); //different colors
      toolNode->Modified();
      //add it to the DataStorage
      mitk::DataStorage::GetInstance()->Add(toolNode);
      visualizer->SetBaseData(i, mitkToolData);
    }
    m_EndOfPipeline = visualizer;

    /* set up trajectories */
    if (m_Controls->m_ShowTrajectories->isChecked())
    {
      m_PointSetFilter = mitk::NavigationDataToPointSetFilter::New();
      m_PointSetFilter->SetOperationMode(mitk::NavigationDataToPointSetFilter::Mode3D);
      for (unsigned int i = 0; i < visualizer->GetNumberOfOutputs(); ++i)
        m_PointSetFilter->SetInput(i, visualizer->GetOutput(i));
      for (unsigned int i = 0; i < m_PointSetFilter->GetNumberOfOutputs(); i++)
      {
        mitk::PointSet* p = m_PointSetFilter->GetOutput(i);
        assert(p);

        mitk::DataTreeNode::Pointer pointSetNode = mitk::DataTreeNode::New();
        pointSetNode->SetData(p);
        pointSetNode->SetName(QString("Trajectory of Output %1").arg(i).latin1());
        mitk::Color color;
        color.Set(0.2, 0.3 * i ,0.9 - 0.2 * i);
        pointSetNode->SetColor(color); //change color of points
        pointSetNode->SetProperty("contourcolor", mitk::ColorProperty::New(color)); // change color of trajectory line
        pointSetNode->SetProperty("pointsize", mitk::FloatProperty::New(2.0)); // enlarge visualization of points
        pointSetNode->SetProperty("contoursize", mitk::FloatProperty::New(1.0)); // enlarge visualization of trajectory line
        pointSetNode->SetBoolProperty("show contour", true);
    pointSetNode->SetBoolProperty("updateDataOnRender", false); // do not call Update() on the pointset during render (this would cause a execution of the pipeline that is still connected to the pointset)

        mitk::DataStorage::GetInstance()->Add(pointSetNode); //add it to the DataStorage
        out->append(QString("Creating Pointset %1 for trajectory visualization").arg(i));
      }
    }

    //start the tracking
    m_Source->StartTracking();
    out->append("started source filter. tracking is running now.");
  }
  catch (std::exception& exp)
  {
    out->append(QString("ERROR during instantiation and initialization of TrackingDeviceSource filter: ") + QString(exp.what()));
    m_Displacer = NULL;
    m_PointSetFilter = NULL;
    m_EndOfPipeline = NULL;
    if (m_Source.IsNotNull())
    {
      m_Source->StopTracking();
      m_Source->Disconnect();
      m_Source = NULL;
    }
    tracker = NULL;
    WaitCursorOff();
    return;
  }
  /* set up GUI, so that measurements can be done*/
  m_Controls->m_MeasureBtn->show();
  m_Controls->m_MeasureBtn->setEnabled(true);
  m_Controls->m_MeasureContinuously->show();
  m_Controls->m_MeasureContinuously->setEnabled(true);
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
  /* Get the output of the last filter with output and print it */
  for (unsigned int i = 0; i < m_EndOfPipeline->GetNumberOfOutputs(); ++i) // for all outputs of the filter
  {
    mitk::NavigationData* nd = m_EndOfPipeline->GetOutput(i);
    if (nd == NULL)
    {
      out->append("WARNING: Navigation Data is NULL");
      continue;
    }
    nd->Update();  // update the navigation data. this will read current tracking data from tracking device.
    // this will also update the visualization filter who causes a repainting of the scene

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
  if (m_PointSetFilter.IsNotNull()) // update tracjectory generation if it is in use
    m_PointSetFilter->Update();

  if (m_MessageFilter.IsNotNull()) // update error visualization if it is in use
    m_MessageFilter->Update();

  mitk::BaseRenderer::GetInstance(m_MultiWidget->mitkWidget4->GetRenderWindow())->RequestUpdate();  // update 3D render window
}


void QmitkIGTExample::OnMeasureContinuously()
{
  if (m_Controls->m_MeasureContinuously->text()=="Start measure continuously")
  {
    m_Timer->start(100);
    m_Controls->m_MeasureContinuously->setText("Stop measure continuously");
  }
  else if (m_Controls->m_MeasureContinuously->text()=="Stop measure continuously")
  {
    m_Timer->stop();
    m_Controls->m_MeasureContinuously->setText("Start measure continuously");
  }
}


void QmitkIGTExample::OnStop()
{
  WaitCursorOn();
  m_Controls->m_MeasureBtn->hide();
  m_Controls->m_MeasureBtn->setEnabled(false);
  m_Controls->m_MeasureContinuously->hide();
  m_Controls->m_MeasureContinuously->setEnabled(false);
  m_Controls->m_StopBtn->hide();
  m_Controls->m_StopBtn->setEnabled(false);
  m_Controls->m_StartTrackingButton->setEnabled(true);
  m_Controls->m_StartNavigationButton->setEnabled(true);
  try
  {
    if (m_Source.IsNotNull())
    {
      m_Source->StopTracking();
      m_Source->Disconnect();
      m_Source = NULL;
    }
    m_EndOfPipeline = NULL;
    m_PointSetFilter = NULL;
    m_Displacer = NULL;
    m_MessageFilter = NULL;
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


mitk::TrackingDevice::Pointer QmitkIGTExample::ConfigureTrackingDevice()
{
  mitk::TrackingDevice::Pointer tracker;
  QString selectedDevice = m_Controls->GetSelectedTrackingDevice();
  if ((selectedDevice == "NDI Polaris")
    || (selectedDevice == "NDI Aurora"))
  {
    mitk::NDITrackingDevice::Pointer trackerNDI = mitk::NDITrackingDevice::New();
    trackerNDI->SetDeviceName(m_Controls->m_Port->text().latin1());
    trackerNDI->SetBaudRate(mitk::SerialCommunication::BaudRate115200);
    out->append(QString("creating NDI Tracker on ") + m_Controls->m_Port->text() + QString(" with 115200 Baud"));
    tracker = trackerNDI;

    if (selectedDevice == "NDI Polaris")
    {
      trackerNDI->SetType(mitk::NDIPolaris);
      QStringList tools = m_Controls->m_ToolList;
      QStringList::iterator it;
      unsigned int index = 0;
      for (it = tools.begin(); it != tools.end(); ++it )
      {
        trackerNDI->AddTool(QString("MyInstrument %1").arg(index++).latin1(), (*it).latin1());
        out->append(QString("adding tool 'MyInstrument' with rom file '") + (*it) + QString("'"));
      }
    }
    else if (selectedDevice == "NDI Aurora")
    {
      trackerNDI->SetType(mitk::NDIAurora);
    }
  }
  else if (selectedDevice == "Micron Tracker")
  {
    mitk::ClaronTrackingDevice::Pointer trackerMT = mitk::ClaronTrackingDevice::New();
    out->append("creating Micron Tracker");
    QStringList tools = m_Controls->m_ToolList;
    QStringList::iterator it;
    unsigned int index = 0;
    for (it = tools.begin(); it != tools.end(); ++it )
    {
      trackerMT->AddTool(QString("MyInstrument %1").arg(index++).latin1(), (*it).latin1()); // create tool with name and tool definition file
      out->append(QString("adding tool 'MyInstrument' with tool definition file '") + (*it) + QString("'"));
    }
    tracker = trackerMT;
  }
  else if (selectedDevice == "MicroBird")
  {
#ifdef MITK_USE_MICROBIRD_TRACKER
    mitk::MicroBirdTrackingDevice::Pointer trackerMB = mitk::MicroBirdTrackingDevice::New();
    mitk::InternalTrackingTool::Pointer toolMB = mitk::InternalTrackingTool::New();
    trackerMB->AddTool(toolMB);
    out->append("creating MicroBird tracking device with one tool");
#else
    out->append("MicroBird support not available in this version. Please select a different tracking device");
#endif // MITK_USE_MICROBIRD_TRACKER
  }
  else if (selectedDevice == "RandomTrackingDevice")
  {
    mitk::RandomTrackingDevice::Pointer trackerRandom = mitk::RandomTrackingDevice::New();
    trackerRandom->AddTool("toolRandom1");
    trackerRandom->AddTool("toolRandom2");
    tracker = trackerRandom;
    out->append("creating virtual random tracking device with two tools");
  }
  else
  {
    tracker = NULL;
  }
  return tracker;
}


void QmitkIGTExample::OnStartRecording()
{
  try
  {
    mitk::TrackingDevice::Pointer tracker = this->ConfigureTrackingDevice();
    if (tracker.IsNull())
    {
      out->append("Error creating tracking device. Did you provide all parameters?");
      return;
    }
    m_Source = mitk::TrackingDeviceSource::New();
    m_Source->SetTrackingDevice(tracker); //here we set the device for the pipeline source

    m_Source->Connect();        //here we connect to the tracking system

    //we need the stringstream for building up our filename
    std::stringstream filename;

    //the .xml extension and an counter is added automatically
    filename << itksys::SystemTools::GetCurrentWorkingDirectory() << "/Test Output";

    m_Recorder = mitk::NavigationDataRecorder::New();
    m_Recorder->SetFileName(filename.str());

    //now every output of the displacer object is connected to the recorder object
    for (unsigned int i = 0; i < m_Source->GetNumberOfOutputs(); i++)
    {
      m_Recorder->AddNavigationData(m_Source->GetOutput(i));  // here we connect to the recorder
    }

    m_Source->StartTracking();  //start the tracking
    m_Recorder->StartRecording(); //after finishing the settings you can start the recording mechanism 

    out->append(QString("Starting Recording from ") + QString(m_Controls->GetSelectedTrackingDevice())
      + QString(" to file ") + QString(m_Recorder->GetFileName()) + QString(" now."));

    //now every update of the recorder stores one line into the file for 
    //each added NavigationData
    m_RecordingTimer->start(100);
  }
  catch (std::exception& e)
  {
    out->append(QString("An error occured: ") + QString(e.what()));
  }
}


void QmitkIGTExample::OnRecording()
{
  m_Recorder->Update();
  mitk::StatusBar::GetInstance()->DisplayText("Recording tracking data now", 75); // Display recording message for 75ms in status bar
}


void QmitkIGTExample::OnStartPlaying()
{
  /* Stop recording */
  m_RecordingTimer->stop();
  m_Recorder->StopRecording();
  out->append("Stopped recording");

  std::stringstream filename;
  //the .xml extension and an counter is added automatically
  filename << itksys::SystemTools::GetCurrentWorkingDirectory() << "/Test Output-0.xml";

  m_Player = mitk::NavigationDataPlayer::New();
  //this is first part of the file name the .xml extension and an counter is added automatically
  m_Player->SetFileName(filename.str()); 
  m_Player->StartPlaying(); //this starts the player 
  //this is necessary because we do not know how many outputs the player has

  out->append(QString("Starting replay from ") + QString(m_Player->GetFileName()));

  /* Visualize output of player using a mitk::PointSet */
  m_PointSetFilter = mitk::NavigationDataToPointSetFilter::New();
  m_PointSetFilter->SetOperationMode(mitk::NavigationDataToPointSetFilter::Mode3D);
  for (unsigned int i = 0; i < m_Player->GetNumberOfOutputs(); i++)
    m_PointSetFilter->SetInput(i, m_Player->GetOutput(i));  // connect the player with the pointset filter

  for (unsigned int i = 0; i < m_PointSetFilter->GetNumberOfOutputs(); i++)
  {
    mitk::PointSet* p = m_PointSetFilter->GetOutput(i);
    assert(p);

    mitk::DataTreeNode::Pointer pointSetNode = mitk::DataTreeNode::New();
    pointSetNode->SetData(p);
    pointSetNode->SetName(QString("Trajectory of Output %1").arg(i).latin1());
    mitk::Color color;
    color.Set(0.25 * i, 1 - 0.25 * i, 0.5);
    pointSetNode->SetColor(color); //change color of points
    pointSetNode->SetProperty("contourcolor", mitk::ColorProperty::New(color)); // change color of trajectory line
    pointSetNode->SetProperty("pointsize", mitk::FloatProperty::New(20.0)); // enlarge visualization of points
    pointSetNode->SetProperty("contoursize", mitk::FloatProperty::New(20.0)); // enlarge visualization of trajectory line
    pointSetNode->SetBoolProperty("show contour", true);
    
    mitk::DataStorage::GetInstance()->Add(pointSetNode); //add it to the DataStorage
    out->append(QString("Creating Pointset %1 for replay visualization").arg(i));
  }

  m_PlayingTimer->start(100);  // start the playback timer
  out->append("starting replay");
}


void QmitkIGTExample::OnPlaying()
{
  m_PointSetFilter->Update();
  mitk::BaseRenderer::GetInstance(m_MultiWidget->mitkWidget4->GetRenderWindow())->RequestUpdate();  // update only 3D render window
  mitk::StatusBar::GetInstance()->DisplayText("Replaying tracking data now", 75); // Display replay message for 75ms in status bar
}


void QmitkIGTExample::OnShowErrorPlot()
{
  if (m_Displacer.IsNull()) // only possible after tracking pipeline is initalized
  {
    out->append("You have to click on 'Start MITK-IGT-Navigation Test' first");
    return; 
  }
  /* Set up error display */
  m_Controls->m_ErrorPlot->SetPlotTitle("Error Values of tool 1");
  m_Controls->m_ErrorPlot->SetAxisTitle( QwtPlot::xBottom, "Timestamp" ); 
  m_Controls->m_ErrorPlot->SetAxisTitle( QwtPlot::yLeft, "Error" ); 
  int curveId = m_Controls->m_ErrorPlot->InsertCurve( "Error value of tool 1" ); 
  m_Controls->m_ErrorPlot->SetCurvePen( curveId, QPen( red ) ); 
  m_XValues.clear();
  m_YValues.clear();
  m_Controls->m_ErrorPlot->setEnabled(true);
  m_Controls->m_ErrorPlot->show();

  m_Controls->m_ErrorBar->setTotalSteps(100); // needs to be set to meaningful values depending on tracking device and application requirements
  m_Controls->m_ErrorBar->setPercentageVisible(false);
  m_Controls->m_ErrorBar->setCenterIndicator(true);
  m_Controls->m_ErrorBar->setStyle(new QWindowsStyle()); // to be able to use custom colors
  m_Controls->m_ErrorBar->reset();
  
  /*set up IGT pipeline -> GUI connection */
  m_MessageFilter = mitk::NavigationDataToMessageFilter::New();
  m_MessageFilter->SetInput(m_Displacer->GetOutput()); // connect with first output of Displacer 
  m_MessageFilter->AddErrorChangedListener(mitk::MessageDelegate1<QmitkIGTExample, mitk::NavigationData::CovarianceMatrixType>(this, &QmitkIGTExample::OnErrorValueChanged));  
}


void QmitkIGTExample::OnErrorValueChanged(mitk::NavigationData::CovarianceMatrixType v)
{
  /* calculate overall error (this should be replaced with a more meaningful implementation depending on the application requirements) */
  mitk::ScalarType errorValue = 0.0;
  for (unsigned int i = 0; i < v.ColumnDimensions; ++i)
    for (unsigned int j = 0; j < v.RowDimensions; ++j)
      errorValue += v(i, j);

  if ((m_MessageFilter.IsNotNull()) && (m_MessageFilter->GetOutput() != NULL))
  {
    mitk::ScalarType timeValue =  m_MessageFilter->GetOutput()->GetTimeStamp();
    m_XValues.push_back(timeValue);
    m_YValues.push_back(errorValue);

    m_Controls->m_ErrorPlot->SetCurveData( 0, m_XValues, m_YValues ); // hardcoded for curve id 0!
    m_Controls->m_ErrorPlot->Replot();
  }
  const mitk::ScalarType errorThreshold = 0.8; // needs to be set to meaningful value depending on application requirements
  
  mitk::ScalarType progressClampError = (errorValue < 1.0) ? errorValue* 100 : 100;// use primitive mapping of error values to the progress bar range of 0..100. needs to be adjusted to meaningful values
  m_Controls->m_ErrorBar->setProgress(progressClampError); 

  /* color the progress bar */
  QColor aboveThresholdColor(255, 0, 0);
  QColor belowThresholdColor(0, 255, 0);

  QPalette pal = m_Controls->m_ErrorBar->palette();
  QColor newColor;
  if (errorValue > errorThreshold)
  {
    newColor = aboveThresholdColor;
    out->append(QString("error value %1 is above threshold of %2!").arg(errorValue).arg(errorThreshold));
  }
  else
  {
    newColor = belowThresholdColor;
  }
  pal.setColor(QColorGroup::Highlight, newColor);   // Set the bar color
  pal.setColor(QColorGroup::Foreground, newColor); // Set the percentage text color
  m_Controls->m_ErrorBar->setPalette(pal);
}
