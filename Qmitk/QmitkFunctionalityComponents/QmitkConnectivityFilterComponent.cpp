/*=========================================================================

Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
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

#include "QmitkConnectivityFilterComponent.h"
#include "QmitkConnectivityFilterComponentGUI.h"

#include "mitkDataTreeFilterFunctions.h"
#include "mitkProperties.h"
#include "mitkRenderingManager.h"
#include "mitkRenderWindow.h"
#include "mitkSurface.h"

#include <mitkDataTreeNodeFactory.h>
#include "mitkDisplayCoordinateOperation.h"
#include "mitkProperties.h"

#include <QmitkDataTreeComboBox.h>
#include "QmitkSeedPointSetComponent.h"
#include "QmitkStdMultiWidget.h"
#include <QmitkPointListWidget.h>

#include <vtkCell.h>
#include <vtkCellPicker.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkRenderer.h>


//#include "QmitkSeedPointSetComponent.h"

//#include <vtkImageGaussianSmooth.h>
#include <vtkPolyDataConnectivityFilter.h>
//#include <vtkRenderer.h>
//#include <vtkRenderWindow.h>
//#include <vtkRenderWindowInteractor.h>
//#include <vtkImageShrink3D.h>
#include <vtkPolyDataMapper.h>
//#include <vtkActor.h>
//#include <vtkMarchingCubes.h>
//#include <vtkSmoothPolyDataFilter.h>
#include <vtkPolyDataNormals.h>
//#include <vtkImageGaussianSmooth.h>
//#include <vtkImageThreshold.h>
//#include <mitkOpenGLRenderer.h>
//#include <mitkVtkRenderWindow.h>
//#include <vtkRenderWindow.h>
#include <vtkSTLWriter.h>

#include <qlayout.h>
#include <qlineedit.h>
#include <qslider.h>
#include <qgroupbox.h>
#include <qcheckbox.h>


/***************       CONSTRUCTOR      ***************/
QmitkConnectivityFilterComponent::QmitkConnectivityFilterComponent(QObject * parent, const char * parentName, bool updateSelector, bool showSelector, QmitkStdMultiWidget *mitkStdMultiWidget, mitk::DataTreeIteratorBase* it)
: QmitkFunctionalityComponentContainer(parent, parentName, updateSelector, showSelector, mitkStdMultiWidget, it),
m_ConnectivityFilterComponentGUI(NULL),
m_ConnectivityNode(NULL),
m_DataIt(it),
m_PointSet(NULL),
m_ConnectivityCounter(0),
m_MultiWidget(mitkStdMultiWidget)
{
  SetDataTreeIterator(it);
  SetAvailability(true);

  SetComponentName("ConnectivityFilterComponent");
}

/***************        DESTRUCTOR      ***************/
QmitkConnectivityFilterComponent::~QmitkConnectivityFilterComponent()
{

}

///*************** GET DATA TREE ITERATOR ***************/
//mitk::DataTreeIteratorBase* QmitkPixelGreyValueManipulatorComponent::GetDataTreeIterator()
//{
//  return m_DataTreeIterator.GetPointer();
//}

/*************** SET DATA TREE ITERATOR ***************/
void QmitkConnectivityFilterComponent::SetDataTreeIterator(mitk::DataTreeIteratorBase* it)
{
  m_DataTreeIterator = it;
}

/*************** GET TREE NODE SELECTOR ***************/
QmitkDataTreeComboBox* QmitkConnectivityFilterComponent::GetTreeNodeSelector()
{
  return m_ConnectivityFilterComponentGUI->GetTreeNodeSelector();
}

/***************   GET POINT SET NODE   ***************/
//mitk::DataTreeNode::Pointer QmitkConnectivityFilterComponent::GetPointSetNode()
//{
// return  m_ConnectivityNode;
//}

/***************   GET IMAGE CONTENT   ***************/
QGroupBox*  QmitkConnectivityFilterComponent::GetImageContent()
{
  return (QGroupBox*) m_ConnectivityFilterComponentGUI->GetImageContent();
}

/************ Update DATATREECOMBOBOX(ES) *************/
void QmitkConnectivityFilterComponent::UpdateDataTreeComboBoxes()
{
  if(GetTreeNodeSelector() != NULL)
  {
    GetTreeNodeSelector()->Update();
  }
}

/***************       CONNECTIONS      ***************/
void QmitkConnectivityFilterComponent::CreateConnections()
{
  if ( m_ConnectivityFilterComponentGUI )
  {
    connect( (QObject*)(m_ConnectivityFilterComponentGUI->GetTreeNodeSelector()), SIGNAL(activated(const mitk::DataTreeFilter::Item *)), (QObject*) this, SLOT(ImageSelected(const mitk::DataTreeFilter::Item *)));
    connect((QObject*)(m_ConnectivityFilterComponentGUI->GetShowTreeNodeSelectorGroupBox()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(ShowTreeNodeSelector()));
    connect((QObject*)(m_ConnectivityFilterComponentGUI->GetShowComponent()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(ShowComponentContent()));
    //Button to start the connectFilter pressed:
    connect((QObject*)(m_ConnectivityFilterComponentGUI->GetStartFilterPushButton()), SIGNAL(clicked()), (QObject*) this, SLOT(StartConnectivityFilter()));

    //to connect the toplevel checkable GroupBox with the method SetContentContainerVisibility to inform all containing komponent to shrink or to expand
    connect( (QObject*)(m_ConnectivityFilterComponentGUI->GetShowComponent()),  SIGNAL(toggled(bool)), (QObject*) this, SLOT(SetContentContainerVisibility(bool))); 

    //to watch wether a mouse-click is important or not
    //connect( (QObject*)(m_ConnectivityFilterComponentGUI->GetFilterModeComboBox()), SIGNAL(activated(int)), (QObject*) this, SLOT(TextBoxChanged(int)));
  }
}

/*************** SHOW COMPONENT CONTENT ***************/
void QmitkConnectivityFilterComponent::ShowComponentContent()
{
  m_ConnectivityFilterComponentGUI->GetComponentContent()->setShown(m_ConnectivityFilterComponentGUI->GetShowComponent()->isChecked());
}

/*************** SHOW TREE NODE SELECTOR **************/
void QmitkConnectivityFilterComponent::ShowTreeNodeSelector()
{
  m_ConnectivityFilterComponentGUI->GetImageContent()->setShown(m_ConnectivityFilterComponentGUI->GetShowTreeNodeSelectorGroupBox()->isChecked());
}

/***************     IMAGE SELECTED     ***************/
void QmitkConnectivityFilterComponent::ImageSelected(const mitk::DataTreeFilter::Item * imageIt)
{
  m_SelectedItem = imageIt;
  mitk::DataTreeFilter::Item* currentItem(NULL);
  if(m_ConnectivityFilterComponentGUI)
  {
    if(mitk::DataTreeFilter* filter = m_ConnectivityFilterComponentGUI->GetTreeNodeSelector()->GetFilter())
    {
      if(imageIt)
      {
        currentItem = const_cast <mitk::DataTreeFilter::Item*> ( filter->FindItem( imageIt->GetNode() ) );
      }
    }
  }
  if(currentItem)
  {
    currentItem->SetSelected(true);
  }

  TreeChanged();
}

/*************** CREATE CONTAINER WIDGET **************/
QWidget* QmitkConnectivityFilterComponent::CreateControlWidget(QWidget* parent)
{
  m_ConnectivityFilterComponentGUI = new QmitkConnectivityFilterComponentGUI(parent);
  m_GUI = m_ConnectivityFilterComponentGUI;

  m_ConnectivityFilterComponentGUI->GetTreeNodeSelector()->SetDataTree(GetDataTreeIterator());
  m_ConnectivityFilterComponentGUI->GetTreeNodeSelector()->GetFilter()->SetFilter(mitk::IsBaseDataType<mitk::Surface>());

  if(m_ShowSelector)
  {
    m_ConnectivityFilterComponentGUI->GetImageContent()->setShown(m_ConnectivityFilterComponentGUI->GetShowTreeNodeSelectorGroupBox()->isChecked());
  }
  else
  {
    m_ConnectivityFilterComponentGUI->GetShowTreeNodeSelectorGroupBox()->setShown(m_ShowSelector);
  }

  //vtkRenderer* renderer = (vtkRenderer*)m_MultiWidget->GetRenderWindow4()->GetRenderer();

  CreatePointSet();
  //m_PointSet->Deactivated();
  return m_ConnectivityFilterComponentGUI;

}
void QmitkConnectivityFilterComponent::TextBoxChanged(int /*number*/)
{
  if(m_ConnectivityFilterComponentGUI->GetFilterModeComboBox()->currentItem()==3)
  {}

}
void QmitkConnectivityFilterComponent::ExecuteOperation(mitk::Operation * /*operation*/ )
{
  //mitk::DisplayCoordinateOperation * displayOperation = operation;
  //mitk::Point2D p2d = displayOperation->GetCurrentDisplayCoordinate();

  //mitk::OpenGLRenderer* renderer = dynamic_cast<mitk::OpenGLRenderer*>( displayOperation->GetRenderer() );
  //if ( renderer != NULL )
  //{
  //  //
  //  // get the cell which was picked at pos (x,y)
  //  //
  //  vtkCellPicker * picker = vtkCellPicker::New();
  //  picker->SetTolerance( 0.001f );
  //  if ( picker->Pick( p2d[ 0 ], p2d[ 1 ], 0.0f, renderer->GetVtkRenderer() ) )
  //  {
  //    //
  //    // Try to determine the id of the vessel which was picked.
  //    // Therefore, we first have to get the vessels poly data from the mapper,
  //    // then to get the point data from the poly data. Then, we lookup
  //    // the vesselId of one of the points of the picked cell...
  //    //
  //    vtkPolyDataMapper * mapper = dynamic_cast<vtkPolyDataMapper*>( picker->GetMapper() );
  //    assert ( mapper );

  //    vtkPolyData* polyData = mapper->GetInput();
  //    assert( polyData );

  //    vtkCell* cell = polyData->GetCell( picker->GetCellId( ) );
  //    assert( cell );
  //  }
  //}

  //mitk::DisplayCoordinateOperation* doOp = new mitk::DisplayCoordinateOperation(OpSENDCOORDINATES,  posEvent->GetSender(), posEvent->GetDisplayPosition(), posEvent->GetDisplayPosition(), posEvent->GetDisplayPosition());
  //    m_Destination->ExecuteOperation(doOp);

}
/*std::cout<<"Points Coordinater:"
<<cell->GetPoints()->GetPoint(0)[0]<<" CellID: "<<picker->GetCellId()
<<" SupID: "<<picker->GetSubId()<<" classname: "<<cell->GetClassName()<<std::endl;*/ 
/*
float coordX, coordY, coordZ;
coordX = cell->GetPoints()->GetPoint( picker->GetSubId() ) [ 0 ];
coordY = cell->GetPoints()->GetPoint( picker->GetSubId() ) [ 1 ];
coordZ = cell->GetPoints()->GetPoint( picker->GetSubId() ) [ 2 ];
//std::cout<<"Coord:  "<<CoordX<<" "<<CoordY<<" "<<CoordZ<<std::endl;
vtkIdList* pointIds = cell->GetPointIds();
assert( pointIds );

vtkPointData* pointData = polyData->GetPointData();
assert( pointData );

// get the vesselId data from the vessels polyData
int index;
vtkUnsignedIntArray* vesselIds = dynamic_cast<vtkUnsignedIntArray*>( pointData->GetArray( "vesselIds", index ) );
if ( index < 0 )
{
std::cout << "No vessel graph picked!" << std::endl;
return ;
}
assert( vesselIds );

// get the vesselId of one of the points of the picked cell
vtkIdType pickedVessel = vesselIds->GetValue( pointIds->GetId( 0 ) );
VesselGraphDataPointer vg = this->GetVesselGraphData( mapper );
if ( vg.IsNotNull() )
{
//
// Activate the vessel with the picked id.
// The colors are updated automatically by the mapper,
// which uses an VesselGraphToLookupTableFilter internally
//
vg->DeactivateAll();
vg->SetActive( pickedVessel, true );
m_LastPickedVesselGraph = vg;
m_LastPickedVesselEdgeID = pickedVessel;

SpVesselEdge pickedEdge = vg->GetEdgeData( pickedVessel, 0 ).edge;
unsigned int elementCount = pickedEdge->getElementCount( );
//std::cout<<"Counte: "<<ElementCount<<std::endl;
//std::cout<<"ele: "<<PickedEdge->getVesselElement( 0 ).getPosition().x<<std::endl;

float ele0X = pickedEdge->getVesselElement( 0 ).getPosition().x;
float ele0Y = pickedEdge->getVesselElement( 0 ).getPosition().y;
float ele0Z = pickedEdge->getVesselElement( 0 ).getPosition().z;
float min = sqrt( ( coordX - ele0X ) * ( coordX - ele0X ) + ( coordY - ele0Y ) * ( coordY - ele0Y ) + ( coordZ - ele0Z ) * ( coordZ - ele0Z ) );
unsigned int minEleNr = 0;

for ( int i = 1;i < elementCount;i++ )
{
float eleX = pickedEdge->getVesselElement( i ).getPosition().x;
float eleY = pickedEdge->getVesselElement( i ).getPosition().y;
float eleZ = pickedEdge->getVesselElement( i ).getPosition().z;
float abstand = sqrt( ( coordX - eleX ) * ( coordX - eleX ) + ( coordY - eleY ) * ( coordY - eleY ) + ( coordZ - eleZ ) * ( coordZ - eleZ ) );
//         std::cout<<"abstand: "<<i<<" "<<Abstand<<PickedEdge->getVesselElement( i ).getPosition()<<std::endl;
if ( min > abstand )
{
min = abstand;
minEleNr = i;
}
}
//std::cout<<"Minimum: "<<Min<<" nr: "<<MinEleNr<<std::endl;
float minEleX = pickedEdge->getVesselElement( minEleNr ).getPosition().x;
float minEleY = pickedEdge->getVesselElement( minEleNr ).getPosition().y;
float minEleZ = pickedEdge->getVesselElement( minEleNr ).getPosition().z;

mitk::VesselCenterPickedEvent vesselCenterPickedEvent;
vesselCenterPickedEvent.SetPickedPoint3D( minEleX, minEleY, minEleZ );
this->InvokeEvent( vesselCenterPickedEvent );
}
}
}}*/

/*************** CREATE SEEDPOINT WIDGET **************/
void QmitkConnectivityFilterComponent::CreatePointSet()
{
  m_PointSet = new QmitkSeedPointSetComponent(GetParent(), GetFunctionalityName(), GetMultiWidget(), m_DataIt);
  m_PointSet->CreateControlWidget(m_ConnectivityFilterComponentGUI);
  m_AddedChildList.push_back(m_PointSet);
  m_ConnectivityFilterComponentGUI->layout()->add(m_PointSet->GetGUI());
  m_PointSet->CreateConnections();
  m_PointSet->SetDataTreeName("SeedPointsForConnectivity");
  m_ConnectivityFilterComponentGUI->repaint();
}

/*************** GET CONTENT CONTAINER  ***************/
QGroupBox * QmitkConnectivityFilterComponent::GetContentContainer()
{
  return m_ConnectivityFilterComponentGUI->GetComponentContent();
}

/************ GET MAIN CHECK BOX CONTAINER ************/
QGroupBox * QmitkConnectivityFilterComponent::GetMainCheckBoxContainer()
{
  return m_ConnectivityFilterComponentGUI->GetShowComponent();
}

/*********** SET CONTENT CONTAINER VISIBLE ************/
void QmitkConnectivityFilterComponent::SetContentContainerVisibility(bool)
{
  if(GetMainCheckBoxContainer() != NULL)
  {
    if(GetMainCheckBoxContainer()->isChecked())
    {
      Activated();
    }
    else
    {
      Deactivated();
    }
  }
  for(unsigned int i = 0;  i < m_AddedChildList.size(); i++)
  {
    if(m_AddedChildList[i]->GetContentContainer() != NULL)
    {
      m_AddedChildList[i]->GetContentContainer()->setShown(GetMainCheckBoxContainer()->isChecked());
    }
    if(m_AddedChildList[i]->GetMainCheckBoxContainer() != NULL)
    {
      m_AddedChildList[i]->GetMainCheckBoxContainer()->setChecked(GetMainCheckBoxContainer()->isChecked());
    }
    m_AddedChildList[i]->SetContentContainerVisibility(GetMainCheckBoxContainer()->isChecked());
  } 
}


/***************      SET POINTSET      ***************/
void QmitkConnectivityFilterComponent::SetPointSet(QmitkSeedPointSetComponent* pointSet)
{
  m_PointSet = pointSet;
}

/***************        ACTIVATED       ***************/
void QmitkConnectivityFilterComponent::Activated()
{
  QmitkBaseFunctionalityComponent::Activated();
  m_Active = true;
  TreeChanged();
  for(unsigned int i = 0;  i < m_AddedChildList.size(); i++)
  {
    m_AddedChildList[i]->Activated();
  } 
}

/***************       DEACTIVATED      ***************/
void QmitkConnectivityFilterComponent::Deactivated()
{
  QmitkBaseFunctionalityComponent::Deactivated();
  m_Active = false;
  for(unsigned int i = 0;  i < m_AddedChildList.size(); i++)
  {
    m_AddedChildList[i]->Deactivated();
  } 
}


//********************************************SHOW LARGEST CONNECTED******************************
//connect if button "SHOW LARGEST CONNECTED" was pressed
//shows the largest connected surface and removes all disconnected areas
void QmitkConnectivityFilterComponent::StartConnectivityFilter()
{
  //Get the FilterMode
  // 0 show largest Region
  // 1 show all Regions
  // 2 showClosestPointRegion
  // 3 delete Point Marked Region


  int filterMode = m_ConnectivityFilterComponentGUI->GetFilterModeComboBox()->currentItem();

  vtkPolyDataConnectivityFilter* pdConnectivity = vtkPolyDataConnectivityFilter::New();
  m_SelectedItem = m_ConnectivityFilterComponentGUI->GetTreeNodeSelector()->GetFilter()->GetSelectedItem();
  if(m_ConnectivityFilterComponentGUI->GetTreeNodeSelector()->GetFilter()->GetItems()->Size() > 0)//if itemSelector is not empty
  {
    mitk::Surface* surface = dynamic_cast<mitk::Surface*>(m_SelectedItem->GetNode()->GetData());
    pdConnectivity->SetInput(surface->GetVtkPolyData());

    int numberOfExtractedRegions = pdConnectivity->GetNumberOfExtractedRegions();
    pdConnectivity->InitializeSpecifiedRegionList();

    switch(filterMode) 
    { 
    case 0: /*show largest Region*/
      {
        ; 
      }
      break; 

    case 1: /*show all Regions*/
      {
        pdConnectivity->SetExtractionModeToAllRegions();               
      }
      break; 

    case 2: /*Point Marked Surface*/
      {
        if(m_PointSet!=NULL)
        {
          if(m_PointSet->GetPointSetNode().IsNotNull())
          {
            //std::cout<<"Punkte sind da:"<<std::endl;
            mitk::PointSet::Pointer pointSet = dynamic_cast<mitk::PointSet*>(m_PointSet->GetPointSetNode()->GetData());
            int numberOfPoints = pointSet->GetSize();
            if(numberOfPoints > 0)
            {
              mitk::PointSet::PointType pointOne = pointSet->GetPoint(0);

              double x1 = pointOne[0];
              double y1 = pointOne[1];
              double z1 = pointOne[2];

              pdConnectivity->SetClosestPoint(x1, y1, z1);
              pdConnectivity->SetExtractionModeToClosestPointRegion();
              //pdConnectivity->
            }
          }
        }
      }
      break;

    case 3:
      {
        if(m_PointSet!=NULL)
        {
          if(m_PointSet->GetPointSetNode().IsNotNull())
          {
            //std::cout<<"Punkte sind da:"<<std::endl;
            mitk::PointSet::Pointer pointSet = dynamic_cast<mitk::PointSet*>(m_PointSet->GetPointSetNode()->GetData());
            int numberOfPoints = pointSet->GetSize();
            if(numberOfPoints > 0)
            {
              vtkRenderer* renderer = (vtkRenderer*)(m_MultiWidget->GetRenderWindow4()->GetRenderer());
              for(int i = 0; i<numberOfPoints; i++)
              {
                mitk::PointSet::PointType pointNumberX  = pointSet->GetPoint(i); 
                vtkCellPicker * picker = vtkCellPicker::New();
                //picker->SetTolerance( 0.001f );
                //if ( picker->Pick(pointNumberX[0], pointNumberX[1], 0.0f, renderer))
                if ( picker->Pick(pointNumberX[0], pointNumberX[1], 0.0f, renderer))
                {
                  vtkPolyDataMapper * mapper = dynamic_cast<vtkPolyDataMapper*>( picker->GetMapper() );
                  assert ( mapper );

                  vtkPolyData* polyData = mapper->GetInput();
                  assert( polyData );

                  //vtkCell* cell = polyData->GetCell( picker->GetCellId());
                  //assert( cell );
                  pdConnectivity->DeleteSpecifiedRegion(picker->GetCellId( )); 
                }
              }
            }
          }
        }
      }
      break;

      //case 3: /*Delete Point Marked Surface*/
      //   {
      // if(m_PointSet!=NULL)
      // {
      //if(m_PointSet->GetPointSetNode().IsNotNull())
      //{
      //	//std::cout<<"Punkte sind da:"<<std::endl;
      //	mitk::PointSet::Pointer pointSet = dynamic_cast<mitk::PointSet*>(m_PointSet->GetPointSetNode()->GetData());
      //	int numberOfPoints = pointSet->GetSize();
      //	if(numberOfPoints > 0)
      //	{
      //		mitk::PointSet::PointType pointOne = pointSet->GetPoint(0);
      //		
      //		double x1 = pointOne[0];
      //		double y1 = pointOne[1];
      //		double z1 = pointOne[2];

      //		pdConnectivity->SetClosestPoint(x1, y1, z1);
      //		pdConnectivity->SetExtractionModeToClosestPointRegion();
      //		int id = pdConnectivity->GetNumberOfExtractedRegions();
      //		pdConnectivity->DeleteSpecifiedRegion(id);

      //	}
      //}
      // }
      //}
      //break;

    default: ; break; 
    }

    pdConnectivity->ColorRegionsOn();
    pdConnectivity->Update();

    numberOfExtractedRegions = pdConnectivity->GetNumberOfExtractedRegions();
    //
    //to set different colors 
    vtkFloatingPointType* range = pdConnectivity->GetOutput()->GetPointData()->GetScalars()->GetRange();

    float scalarsMin = range[0];
    float scalarsMax = range[1];

    mitk::DataTreeIteratorClone itConnectivity = GetDataTreeIterator();
    //  std::cout<<"while:"<<std::endl;
    int i = 0;
    while ( !itConnectivity->IsAtEnd() )
    {
      i++;
      //    std::cout<<i<<std::endl;
      mitk::DataTreeNode::Pointer aNode = const_cast<mitk::DataTreeNode*>(m_SelectedItem->GetNode());
      if ( aNode->GetData() != NULL )
      {
        mitk::Surface::Pointer sf = dynamic_cast<mitk::Surface*>( aNode->GetData() );
        if (sf.IsNotNull())
        {
          mitk::DataTreeNode::Pointer connectivityNode = mitk::DataTreeNode::New();
          vtkPolyData* connected = pdConnectivity->GetOutput();
          sf->SetVtkPolyData(connected, 0);
          connectivityNode->SetData(sf);

          int layer = 0;

          ++m_ConnectivityCounter;


          std::ostringstream buffer;
          QString connectivityDataName = m_ConnectivityFilterComponentGUI->GetTreeNodeSelector()->currentText();
          buffer << connectivityDataName.ascii();
          buffer << "_";
          buffer << m_ConnectivityCounter;

          std::string connectivityNodeName = "Connectivity_" + buffer.str();

          mitk::DataTreeIteratorClone iteratorOnImageToBeSkinExtracted = m_ConnectivityFilterComponentGUI->GetTreeNodeSelector()->GetFilter()->GetIteratorToSelectedItem(); 
          iteratorOnImageToBeSkinExtracted->Get()->GetIntProperty("layer", layer);
          connectivityNode->SetIntProperty("layer", layer+1);
          mitk::DataTreeNodeFactory::SetDefaultSurfaceProperties(connectivityNode);
          connectivityNode->SetProperty("deprecated usePointDataForColouring", new mitk::BoolProperty(true));
          connectivityNode->SetProperty("name", new mitk::StringProperty(connectivityNodeName) );
          connectivityNode->SetProperty("ScalarsRangeMaximum", new mitk::FloatProperty(scalarsMax));
          connectivityNode->SetProperty("ScalarsRangeMinimum", new mitk::FloatProperty(scalarsMin));
          if(filterMode ==1)
          {
            connectivityNode->SetProperty( "scalar visibility", new mitk::BoolProperty( true ));
          }
          else
          {
            connectivityNode->SetProperty( "scalar visibility", new mitk::BoolProperty( false ));
          }
          connectivityNode->SetProperty( "deprecated usePointDataForColouring", new mitk::BoolProperty(false) );

          mitk::DataTreeIteratorClone iteratorConnectivity = m_DataTreeIterator;

          //bool isSurface = false;
          //bool isConnectivity = false;

          while(!(iteratorConnectivity->IsAtEnd()))
          {
            if(iteratorConnectivity->Get() != m_SelectedItem->GetNode())
            {
              //std::cout <<"Ungleicher Knoten"<<std::endl;
            }
            else 
              if(iteratorConnectivity->Get() == m_SelectedItem->GetNode())
              {

                //std::cout <<"Gleicher Knoten"<<std::endl;
                if(filterMode==2)
                {
                  iteratorConnectivity->Add(connectivityNode);
                  emit nodeExported( connectivityNode );
                }
                else
                {
                  iteratorConnectivity->Set(connectivityNode);
                }
                //mitk::Geometry3D *  mitk::BoundingObjectGroup::GetGeometry (int t) const
                iteratorConnectivity->GetTree()->Modified();
                //std::cout <<"SET"<<std::endl;
                break;
              }
              else
              {
                iteratorConnectivity->Add(connectivityNode);
                std::cout <<"ADD"<<std::endl;
                break;
              }

              ++iteratorConnectivity;
          }

          break;
        }//end of if (sf.IsNotNull())
      }//end of if ( aNode->GetData() != NULL )
      ++itConnectivity;
    }//end of while ( !itConnectivity->IsAtEnd() )
    //multiWidget->RequestUpdate();
    m_MultiWidget->GetRenderWindow1()->repaint();
    m_MultiWidget->GetRenderWindow2()->repaint();
    m_MultiWidget->GetRenderWindow3()->repaint();
    m_MultiWidget->GetRenderWindow4()->repaint();
  }//end of if itemSelector is not empty
}//end of showConnected()

