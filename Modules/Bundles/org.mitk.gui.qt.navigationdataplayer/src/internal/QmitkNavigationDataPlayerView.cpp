/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Language:  C++
Date:      $Date: 2010-03-31 16:40:27 +0200 (Mi, 31 Mrz 2010) $
Version:   $Revision: 21975 $ 

Copyright (c) German Cancer Research Center, Division of Medical and
Biological Informatics. All rights reserved.
See MITKCopyright.txt or http://www.mitk.org/copyright.html for details.

This software is distributed WITHOUT ANY WARRANTY; without even
the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
PURPOSE.  See the above copyright notices for more information.

=========================================================================*/

// Blueberry
#include <berryISelectionService.h>
#include <berryIWorkbenchWindow.h>

// Qmitk
#include "QmitkNavigationDataPlayerView.h"
#include "QmitkStdMultiWidget.h"

//mitk
#include <mitkCone.h>
#include <mitkNodePredicateNot.h>
#include <mitkNodePredicateProperty.h>

// Qt
#include <QMessageBox>

// vtk
#include <vtkConeSource.h> 


const std::string QmitkNavigationDataPlayerView::VIEW_ID = "org.mitk.views.navigationdataplayer";

QmitkNavigationDataPlayerView::QmitkNavigationDataPlayerView()
: QmitkFunctionality()
, m_Controls( 0 )
, m_MultiWidget( NULL )
, m_Trajectory( NULL )
, m_TrajectoryIndex( -1 )
, m_ReloadData( true )
, m_ShowTrajectory( false )
, m_SplineMapper( NULL )
, m_PointSetMapper( NULL )
{

  m_TrajectoryPointSet = mitk::PointSet::New(); // PointSet for trajectory points
}



QmitkNavigationDataPlayerView::~QmitkNavigationDataPlayerView()
{
  delete m_PlayerWidget;
}


void QmitkNavigationDataPlayerView::CreateQtPartControl( QWidget *parent )
{
  // build up qt view, unless already done
  if ( !m_Controls )
  {
    // create GUI widgets from the Qt Designer's .ui file
    m_Controls = new Ui::QmitkNavigationDataPlayerViewControls;
    m_Controls->setupUi( parent );

    this->CreateBundleWidgets( parent );
    this->CreateConnections();
  }
}



void QmitkNavigationDataPlayerView::CreateBundleWidgets(QWidget* parent)
{  
  m_PlayerWidget = new QmitkIGTPlayerWidget( parent );   // this bundle's ND player widget
}


void QmitkNavigationDataPlayerView::CreateConnections()
{
  connect( m_PlayerWidget, SIGNAL(SignalPlayingStarted()), this, SLOT(OnCreatePlaybackVisualization()) );
  connect( m_PlayerWidget, SIGNAL(SignalPlayerUpdated()), this, SLOT(OnPerformPlaybackVisualization()) );
  connect( m_PlayerWidget, SIGNAL(SignalInputFileChanged()), this, SLOT(OnReinit()) );
  connect( m_PlayerWidget, SIGNAL(SignalCurrentTrajectoryChanged(int)), this, SLOT (OnShowTrajectory(int)) );
  connect( m_PlayerWidget, SIGNAL(SignalPlayingStarted()), this, SLOT(OnPlayingStarted()) );
  connect( m_PlayerWidget, SIGNAL(SignalSplineModeToggled(bool)), this, SLOT(OnEnableSplineTrajectoryMapper(bool)) );
}


void QmitkNavigationDataPlayerView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
  m_MultiWidget = &stdMultiWidget;
}


void QmitkNavigationDataPlayerView::StdMultiWidgetNotAvailable()
{
  m_MultiWidget = NULL;
}


void QmitkNavigationDataPlayerView::OnPlayingStarted()
{
  m_TrajectoryPointSet->Clear(); // clear trajectory data before every replay
}

void QmitkNavigationDataPlayerView::OnCreatePlaybackVisualization()
{
  if(m_ReloadData) // only if input file changed
  {
    m_Visualizer = mitk::NavigationDataObjectVisualizationFilter::New(); 

    mitk::DataStorage* ds = this->GetDefaultDataStorage();

    unsigned int nrTools = m_PlayerWidget->GetNumberOfTools(); // obtain number of tools from replay file

    QStringList toolNames;
    toolNames << "No trajectory selected ..."; // info statement at beginning of trajectories list

    for(int i=0; i<nrTools; ++i)
    {
      std::stringstream ss;
      QString nodeName = QString("Replay Tool %1").arg(i).toLatin1();

      toolNames << nodeName;

      mitk::Color color = this->GetColorCircleColor(i);  // color for replay object

      mitk::DataNode::Pointer playbackRepresentation = this->CreateRepresentationObject( nodeName.toStdString(), color  ); // create replay DataNode object

      m_Visualizer->SetRepresentationObject(i, playbackRepresentation->GetData()); // add replay object to visualizer

      // add new representation object to DataStorage
      this->AddRepresentationObject(this->GetDefaultDataStorage(), playbackRepresentation); 

    }

    this->m_PlayerWidget->SetTrajectoryNames(toolNames); // set names in player widget trajectory selection combobox
    m_ReloadData = false;


    /// request global reiinit
    mitk::NodePredicateNot::Pointer pred
      = mitk::NodePredicateNot::New(mitk::NodePredicateProperty::New("includeInBoundingBox"
      , mitk::BoolProperty::New(false)));

    mitk::DataStorage::SetOfObjects::ConstPointer rs = this->GetDataStorage()->GetSubset(pred);
    // calculate bounding geometry of these nodes
    mitk::TimeSlicedGeometry::Pointer bounds = this->GetDataStorage()->ComputeBoundingGeometry3D(rs, "visible");

    // initialize the views to the bounding geometry
    mitk::RenderingManager::GetInstance()->InitializeViews(bounds);

    /// global reinit end



  }

}

mitk::DataNode::Pointer QmitkNavigationDataPlayerView::CreateTrajectory( mitk::PointSet::Pointer points, const std::string& name, const mitk::Color color )
{
  mitk::DataNode::Pointer result = mitk::DataNode::New();

  // instantiate return DataNode
  result->SetData(points);
  result->SetName(name);
  result->SetColor(color);

  mitk::PointSetVtkMapper3D::Pointer mapper;

  // create mapper for trajectory visualization
  if(m_PlayerWidget->IsTrajectoryInSplineMode())
    mapper = this->GetTrajectoryMapper(Splines);
  else
    mapper = this->GetTrajectoryMapper(Points);

  result->SetMapper(mitk::BaseRenderer::Standard3D, mapper);

  // trajectory properties
  result->SetProperty("contourcolor", mitk::ColorProperty::New(color));
  result->SetBoolProperty("show contour", true);
  result->SetBoolProperty("updateDataOnRender", false);

  return result;
}


mitk::DataNode::Pointer QmitkNavigationDataPlayerView::CreateRepresentationObject(const std::string& name, const mitk::Color color)
{
  mitk::DataNode::Pointer representationNode = mitk::DataNode::New();

  // creating cone DataNode for tool visualization
  mitk::Cone::Pointer cone = mitk::Cone::New();
  vtkConeSource* vtkData = vtkConeSource::New();

  vtkData->SetRadius(7.5);
  vtkData->SetHeight(15.0);
  vtkData->SetDirection(0.0, 0.0, 1.0);
  vtkData->SetCenter(0.0, 0.0, 0.0);
  vtkData->SetResolution(20);
  vtkData->CappingOn();
  vtkData->Update();

  cone->SetVtkPolyData(vtkData->GetOutput());

  vtkData->Delete();

  representationNode->SetData(cone);

  representationNode->GetPropertyList()->SetProperty("name", mitk::StringProperty::New( name.c_str() ));
  representationNode->GetPropertyList()->SetProperty("layer", mitk::IntProperty::New(0));
  representationNode->GetPropertyList()->SetProperty("visible", mitk::BoolProperty::New(true));
  representationNode->SetColor(color);
  representationNode->SetOpacity(1.0);
  representationNode->Modified();

  return representationNode;
}


void QmitkNavigationDataPlayerView::OnPerformPlaybackVisualization()
{
  if(m_PlayerWidget == NULL || m_Visualizer.IsNull())
    return;

  static int update = 0;
  static int counter = -1;

  for(unsigned int i = 0 ; i < m_PlayerWidget->GetNavigationDatas().size(); ++i)
  {
    m_Visualizer->SetInput(i, m_PlayerWidget->GetNavigationDatas().at(i)); // pass updated tool NDs to visualizer


    // show trajectory for selected tool with user given resolution
    if(m_ShowTrajectory && (i == m_TrajectoryIndex) && (update++ % m_PlayerWidget->GetResolution() == 0) )
    {

      mitk::PointSet::PointType currentPoint = m_PlayerWidget->GetNavigationDataPoint(i); // current ND point for tool trajectory

      // if realtime mode is selected, trajectory points that are equal in position to their antecessor 
      // will not be inserted in the trajectory pointset to avoid "vtk can't create normals" warning
      if(m_PlayerWidget->GetCurrentPlaybackMode() == QmitkIGTPlayerWidget::RealTimeMode)
      {
        mitk::PointSet::PointType lastPoint; 
        if(counter == -1)
        {
          lastPoint[0] = -1;
          lastPoint[1] = -1;
          lastPoint[2] = -1;
        }
        else
          lastPoint = m_TrajectoryPointSet->GetPoint(counter); // antecessor point is last point from PointSet

        mitk::PointSet::PointType currentPoint = m_PlayerWidget->GetNavigationDataPoint(i);

        // check for position differences between last and current point
        bool diff0 = lastPoint[0] != currentPoint[0];
        bool diff1 = lastPoint[1] != currentPoint[1];
        bool diff2 = lastPoint[2] != currentPoint[2];

        if(diff0 || diff1 || diff2)
          m_TrajectoryPointSet->InsertPoint(++counter,  currentPoint); // insert only if there are differences
      }
      else
      {
        m_TrajectoryPointSet->InsertPoint(++counter, currentPoint); // insert point in trajectory PointSet
      }

    }
  }

  this->RenderScene();
}

void QmitkNavigationDataPlayerView::RenderScene()
{ 
  try
  {
    if (m_Visualizer.IsNull() || this->GetActiveStdMultiWidget() == NULL)
      return;

    try
    {
      m_Visualizer->Update();
    }
    catch(std::exception& e) 
    {
      std::cout << "Exception during QmitkNavigationDataPlayerView::RenderScene():" << e.what() << "\n";
    }

    //update all Widgets
    // mitk::RenderingManager::GetInstance()->RequestUpdateAll(mitk::RenderingManager::REQUEST_UPDATE_3DWINDOWS);

    // update only Widget4
    mitk::BaseRenderer::GetInstance(m_MultiWidget->mitkWidget4->GetRenderWindow())->RequestUpdate();  // update 3D render window
  }
  catch (std::exception& e)
  {
    std::cout << "RenderAll exception: " << e.what() << "\n";
  }
  catch (...)
  {
    std::cout << "RenderAll unknown exception\n";
  }  
}

void QmitkNavigationDataPlayerView::OnReinit()
{

  std::vector<mitk::DataNode::Pointer>::iterator it;

  mitk::DataStorage* ds = this->GetDefaultDataStorage();

  // clear tool representation objects from DataStorage
  for ( it = m_RepresentationObjects.begin() ; it != m_RepresentationObjects.end(); it++ )
  {
    //ds->Remove(*it);
    mitk::DataNode::Pointer dn = ds->GetNamedNode((*it)->GetName());
    if(dn.IsNotNull())
      ds->Remove(dn);
  }

  m_RepresentationObjects.clear();  // clear tool representation objects vector

  if(m_Trajectory.IsNotNull())
    this->GetDefaultDataStorage()->Remove(m_Trajectory); // remove trajectory DataNode from DataStorage

  m_TrajectoryPointSet->Clear(); // clear trajectory PointSet
  this->m_PlayerWidget->ClearTrajectorySelectCombobox(); // clear trajectory selection combobox in player widget

  m_ReloadData = true; // set flag to true so representation data will be reload if play is triggered again 

}

void QmitkNavigationDataPlayerView::AddTrajectory(mitk::DataStorage* ds, mitk::DataNode::Pointer trajectoryNode)
{
  if(ds == NULL)
    return;

  if(m_Trajectory.IsNotNull())
    ds->Remove(m_Trajectory);  // remove trajectory from DataStorage if already exists


  // add trajectory to DataStorage
  if(ds != NULL && trajectoryNode.IsNotNull())
  {
    m_Trajectory = trajectoryNode;
    ds->Add(m_Trajectory);
  }
}

void QmitkNavigationDataPlayerView::AddRepresentationObject(mitk::DataStorage* ds, mitk::DataNode::Pointer reprObject)
{
  m_RepresentationObjects.push_back(reprObject);
  ds->Add(reprObject);
}

void QmitkNavigationDataPlayerView::RemoveRepresentationObject(mitk::DataStorage* ds, mitk::DataNode::Pointer reprObject)
{
  std::vector<mitk::DataNode::Pointer>::iterator it;

  for ( it = m_RepresentationObjects.begin() ; it != m_RepresentationObjects.end(); it++ )
  {
    if(*it == reprObject)
    {
      m_RepresentationObjects.erase(it);
      ds->Remove(reprObject);
    }
  }
}

void QmitkNavigationDataPlayerView::OnShowTrajectory(int index)
{

  mitk::DataStorage* ds = this->GetDefaultDataStorage();


  // no trajectory selected
  if(index == 0)
  {
    m_ShowTrajectory = false;
    m_TrajectoryIndex = -1;

    if(m_Trajectory.IsNotNull())
      ds->Remove(m_Trajectory);
  }

  else
  {
    m_ShowTrajectory = true;

    // index-1 because combobox is filled with infovalue at index = 0
    mitk::DataNode::Pointer replayObject = m_RepresentationObjects.at(index-1); 

    std::string prefix("Trajectory of ");
    std::string name = replayObject->GetName();

    mitk::Color color = this->GetColorCircleColor(index-1);

    if(m_TrajectoryPointSet.IsNotNull())
      m_TrajectoryPointSet->Clear();

    m_TrajectoryIndex = index-1;

    mitk::DataNode::Pointer trajectory = this->CreateTrajectory( m_TrajectoryPointSet, prefix.append(name), color );
    this->AddTrajectory(this->GetDefaultDataStorage(), trajectory);
  }
}


void QmitkNavigationDataPlayerView::OnEnableSplineTrajectoryMapper(bool enable)
{
  if(m_Trajectory.IsNull())
    return;

  // if enabled set spline mapper
  if(enable)
    m_Trajectory->SetMapper(mitk::BaseRenderer::Standard3D, this->GetTrajectoryMapper(Splines));

  // if disabled set pointset mapper
  else
    m_Trajectory->SetMapper(mitk::BaseRenderer::Standard3D, this->GetTrajectoryMapper(Points));


  mitk::RenderingManager::GetInstance()->RequestUpdateAll(); // request update after mapper change
}



mitk::Color QmitkNavigationDataPlayerView::GetColorCircleColor(int index)
{
  mitk::Color result;

  mitk::ColorSequenceCycleH colorCycle;

  for(int i=0; i <= index; ++i)
  {
    result = colorCycle.GetNextColor();
  }

  return result;
}


mitk::PointSetVtkMapper3D::Pointer QmitkNavigationDataPlayerView::GetTrajectoryMapper(TrajectoryStyle style)
{
  if(style == Points)
  { 
    if(m_PointSetMapper.IsNull())
      m_PointSetMapper = mitk::PointSetVtkMapper3D::New();

    return m_PointSetMapper;
  }

  else if(style == Splines)
  {
    if(m_SplineMapper.IsNull())
      m_SplineMapper = mitk::SplineVtkMapper3D::New();

    return  m_SplineMapper.GetPointer();
  }
  else
    return NULL;
}



