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

// Qt
#include <QMessageBox>

// vtk
#include <vtkConeSource.h> 


const std::string QmitkNavigationDataPlayerView::VIEW_ID = "org.mitk.views.navigationdataplayer";

QmitkNavigationDataPlayerView::QmitkNavigationDataPlayerView()
: QmitkFunctionality()
, m_Controls( 0 )
, m_MultiWidget( NULL )
, m_ColorCycle( NULL )
, m_Trajectory( NULL )
, m_TrajectoryIndex( -1 )
, m_ReloadData( true )
, m_ShowTrajectory( false )
, m_TrajectorySpline( NULL )
{

  m_RepresentationObjects = new std::vector<mitk::DataNode::Pointer>();
  m_TrajectoryPointSet = mitk::PointSet::New();
  m_SplineMapper = mitk::SplineVtkMapper3D::New();
}



QmitkNavigationDataPlayerView::~QmitkNavigationDataPlayerView()
{
  delete m_PlayerWidget;
  delete m_ColorCycle;
  delete m_RepresentationObjects;

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
  m_PlayerWidget = new QmitkIGTPlayerWidget( parent );   
  //m_PlayerWidget->SetWidgetViewToNormalPlayback();
}


void QmitkNavigationDataPlayerView::CreateConnections()
{
   connect( m_PlayerWidget, SIGNAL(PlayingStarted()), this, SLOT(CreatePlaybackVisualization()) );
   connect( m_PlayerWidget, SIGNAL(PlayerUpdated()), this, SLOT(PerformPlaybackVisualization()) );
   connect( m_PlayerWidget, SIGNAL(InputFileChanged()), this, SLOT(Reinit()) );
   connect( m_PlayerWidget, SIGNAL(SignalCurrentTrajectoryChanged(int)), this, SLOT (OnShowTrajectory(int)) );
   connect( m_PlayerWidget, SIGNAL(PlayingStarted()), this, SLOT(OnPlayingStarted()) );
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
  m_TrajectoryPointSet->Clear();
}

void QmitkNavigationDataPlayerView::CreatePlaybackVisualization()
{

  if(m_ReloadData)
  {
    m_Visualizer = mitk::NavigationDataObjectVisualizationFilter::New(); 

    mitk::DataStorage* ds = this->GetDefaultDataStorage();

    unsigned int nrTools = m_PlayerWidget->GetNumberOfTools();

    QStringList toolNames;
    toolNames << "No trajectory selected ..."; // info statement at beginning of trajectories list

    for(int i=0; i<nrTools; ++i)
    {
      std::stringstream ss;
      QString nodeName = QString("Replay Tool %1").arg(i).toLatin1();

      toolNames << nodeName;

      mitk::Color color = this->GetColorCircleColor(i);

      mitk::DataNode::Pointer playbackRepresentation = this->CreateRepresentationObject( nodeName.toStdString(), color  );

      m_Visualizer->SetRepresentationObject(i, playbackRepresentation->GetData());

      // check if node not already exists in DataStorage and delete if so
      //mitk::DataNode::Pointer dn = ds->GetNamedNode(ss.str().c_str());
      /* if(dn.IsNotNull())
      this->RemoveRepresentationObject(this->GetDefaultDataStorage(), dn);*/
      //ds->Remove(dn);

      // adding new representation object to DataStorage
       this->AddRepresentationObject(this->GetDefaultDataStorage(), playbackRepresentation);  
      
    }
    
    this->m_PlayerWidget->SetTrajectoryNames(toolNames);
    m_ReloadData = false;
  }
  
}

mitk::DataNode::Pointer QmitkNavigationDataPlayerView::CreateTrajectory( mitk::PointSet::Pointer points, const std::string& name, const mitk::Color color )
{
  mitk::DataNode::Pointer result = mitk::DataNode::New();

  result->SetData(points);
  result->SetName(name);
  result->SetColor(color);

 /* result->SetProperty("contourcolor", mitk::ColorProperty::New(color));
  result->SetBoolProperty("show contour", true);
  result->SetBoolProperty("updateDataOnRender", false);*/



  return result;
}


mitk::DataNode::Pointer QmitkNavigationDataPlayerView::CreateRepresentationObject(const std::string& name, const mitk::Color color)
{
  mitk::DataNode::Pointer representationNode = mitk::DataNode::New();

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


void QmitkNavigationDataPlayerView::PerformPlaybackVisualization()
{
  if(m_PlayerWidget == NULL || m_Visualizer.IsNull())
    return;

  static int update = 0;
  update++;

  for(unsigned int i = 0 ; i < m_PlayerWidget->GetNavigationDatas().size(); ++i)
  {
    m_Visualizer->SetInput(i, m_PlayerWidget->GetNavigationDatas().at(i));

    if(m_ShowTrajectory && (i == m_TrajectoryIndex) && (update % m_PlayerWidget->GetResolution() == 0) )
      m_TrajectoryPointSet->InsertPoint(update,  m_PlayerWidget->GetNavigationDataPoint(i));
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

void QmitkNavigationDataPlayerView::Reinit()
{

  std::vector<mitk::DataNode::Pointer>::iterator it;
 
  mitk::DataStorage* ds = this->GetDefaultDataStorage();

  for ( it = m_RepresentationObjects->begin() ; it != m_RepresentationObjects->end(); it++ )
  {
    //ds->Remove(*it);
    mitk::DataNode::Pointer dn = ds->GetNamedNode((*it)->GetName());
    if(dn.IsNotNull())
      ds->Remove(dn);
  }

  m_RepresentationObjects->clear();  
  
  if(m_Trajectory.IsNotNull())
      this->GetDefaultDataStorage()->Remove(m_Trajectory);
  
  m_TrajectoryPointSet->Clear();
  this->m_PlayerWidget->ClearTrajectorySelectCombobox();

  m_ReloadData = true;

}

void QmitkNavigationDataPlayerView::AddTrajectory(mitk::DataStorage* ds, mitk::DataNode::Pointer trajectoryNode)
{
  if(ds == NULL)
    return;

  if(m_Trajectory.IsNotNull())
    ds->Remove(m_Trajectory);


  if(ds != NULL && trajectoryNode.IsNotNull())
  {
    m_Trajectory = trajectoryNode;
    ds->Add(m_Trajectory);
  }
}

void QmitkNavigationDataPlayerView::AddRepresentationObject(mitk::DataStorage* ds, mitk::DataNode::Pointer reprObject)
{
   m_RepresentationObjects->push_back(reprObject);
   
 //  ds->RemoveNodeEvent.AddListener( mitk::MessageDelegate1<QmitkNavigationDataPlayerView, mitk::DataNode::Pointer>(this, &QmitkNavigationDataPlayerView::RemoveRepresentationObject));
   ds->Add(reprObject);
}

void QmitkNavigationDataPlayerView::RemoveRepresentationObject(mitk::DataStorage* ds, mitk::DataNode::Pointer reprObject)
{
  std::vector<mitk::DataNode::Pointer>::iterator it;

  for ( it = m_RepresentationObjects->begin() ; it != m_RepresentationObjects->end(); it++ )
  {
    if(*it == reprObject)
    {
      m_RepresentationObjects->erase(it);
      ds->Remove(reprObject);
    }
  }
}

void QmitkNavigationDataPlayerView::OnShowTrajectory(int index)
{

  mitk::DataStorage* ds = this->GetDefaultDataStorage();


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
    mitk::DataNode::Pointer replayObject = m_RepresentationObjects->at(index-1); 

    std::string prefix("Trajectory of ");
    std::string name = replayObject->GetName();

    mitk::Color color = this->GetColorCircleColor(index-1);

    if(m_TrajectoryPointSet.IsNotNull())
      m_TrajectoryPointSet->Clear();

    m_TrajectoryIndex = index-1;

    mitk::DataNode::Pointer trajectory = this->CreateTrajectory( m_TrajectoryPointSet, prefix.append(name), color );
    trajectory->SetMapper(mitk::BaseRenderer::Standard3D, m_SplineMapper);
    
    this->AddTrajectory(this->GetDefaultDataStorage(), trajectory);
  }

  
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