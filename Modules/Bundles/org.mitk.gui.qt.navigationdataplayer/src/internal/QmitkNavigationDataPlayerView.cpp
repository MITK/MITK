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
, m_ReloadData( true )
{

  m_RepresentationObjectsSet = new std::set<mitk::DataNode::Pointer>();
}




QmitkNavigationDataPlayerView::~QmitkNavigationDataPlayerView()
{
  delete m_PlayerWidget;
  delete m_ColorCycle;
  delete m_RepresentationObjectsSet;
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
  m_PlayerWidget->SetWidgetViewToNormalPlayback();
}


void QmitkNavigationDataPlayerView::CreateConnections()
{
   connect( (QObject*) m_PlayerWidget, SIGNAL(PlayingStarted()), this, SLOT(CreatePlaybackVisualization()) );
   connect( (QObject*) m_PlayerWidget, SIGNAL(PlayerUpdated()), this, SLOT(PerformPlaybackVisualization()) );
}


void QmitkNavigationDataPlayerView::StdMultiWidgetAvailable (QmitkStdMultiWidget &stdMultiWidget)
{
  m_MultiWidget = &stdMultiWidget;
}


void QmitkNavigationDataPlayerView::StdMultiWidgetNotAvailable()
{
  m_MultiWidget = NULL;
}


void QmitkNavigationDataPlayerView::CreatePlaybackVisualization()
{

  if(m_ReloadData)
  {

    m_ColorCycle = new mitk::ColorSequenceCycleH();
    m_Visualizer = mitk::NavigationDataObjectVisualizationFilter::New(); 

    mitk::DataStorage* ds = this->GetDefaultDataStorage();

    unsigned int nrTools = m_PlayerWidget->GetNumberOfTools();

    for(int i=0; i<nrTools; ++i)
    {
      //mitk::DataNode::Pointer playbackRepresentation; 
      std::stringstream ss;
      ss << "Tool " << i << " Replay";

      mitk::DataNode::Pointer playbackRepresentation = this->CreateRepresentationObject( ss.str(), m_ColorCycle->GetNextColor()  );

      m_Visualizer->SetRepresentationObject(i, playbackRepresentation->GetData());

      // check if node not already exists in DataStorage and delete if so
      //mitk::DataNode::Pointer dn = ds->GetNamedNode(ss.str().c_str());
      /* if(dn.IsNotNull())
      this->RemoveRepresentationObject(this->GetDefaultDataStorage(), dn);*/
      //ds->Remove(dn);

      // adding new representation object to DataStorage
      this->AddRepresentationObject(this->GetDefaultDataStorage(), playbackRepresentation);  
    }
    m_ReloadData = false;
  }
}



mitk::DataNode::Pointer QmitkNavigationDataPlayerView::CreateRepresentationObject(const std::string name, const mitk::Color color)
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
  representationNode->SetOpacity(0.9);
  representationNode->Modified();

  return representationNode;
}


void QmitkNavigationDataPlayerView::PerformPlaybackVisualization()
{
  if(m_PlayerWidget == NULL || m_Visualizer.IsNull())
    return;

  for(unsigned int i = 0 ; i < m_PlayerWidget->GetNavigationDatas().size(); ++i)
  {
    m_Visualizer->SetInput(i, m_PlayerWidget->GetNavigationDatas().at(i));
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

  std::set<mitk::DataNode::Pointer>::iterator it;
 
  mitk::DataStorage* ds = this->GetDefaultDataStorage();

  for ( it = m_RepresentationObjectsSet->begin() ; it != m_RepresentationObjectsSet->end(); it++ )
  {
    mitk::DataNode::Pointer dn = ds->GetNamedNode((*it)->GetName());
    if(dn.IsNotNull())
      ds->Remove(dn);
  }

  m_RepresentationObjectsSet->clear();

  m_ReloadData = true;

}

void QmitkNavigationDataPlayerView::AddRepresentationObject(mitk::DataStorage* ds, mitk::DataNode::Pointer reprObject)
{
   m_RepresentationObjectsSet->insert(reprObject);
   
 //  ds->RemoveNodeEvent.AddListener( mitk::MessageDelegate1<QmitkNavigationDataPlayerView, mitk::DataNode::Pointer>(this, &QmitkNavigationDataPlayerView::RemoveRepresentationObject));
   ds->Add(reprObject);
}

void QmitkNavigationDataPlayerView::RemoveRepresentationObject(mitk::DataStorage* ds, mitk::DataNode::Pointer reprObject)
{
  std::set<mitk::DataNode::Pointer>::iterator it;

  it = m_RepresentationObjectsSet->find(reprObject);

//  if(it != m_RepresentationObjectsSet->end())
    m_RepresentationObjectsSet->erase(it);
    ds->Remove(reprObject);

}