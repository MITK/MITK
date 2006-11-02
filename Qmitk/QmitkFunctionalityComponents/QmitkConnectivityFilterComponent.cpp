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

#include <QmitkDataTreeComboBox.h>


#include "mitkSurface.h"
#include "mitkRenderWindow.h"
#include "mitkRenderingManager.h"
#include "mitkProperties.h"
#include "mitkDataTreeFilterFunctions.h"
#include <QmitkPointListWidget.h>
#include "mitkProperties.h"

#include <vtkPolyData.h>
#include <vtkPointData.h>
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


#include <qlineedit.h>
#include <qslider.h>
#include <qgroupbox.h>
#include <qcheckbox.h>


/***************       CONSTRUCTOR      ***************/
QmitkConnectivityFilterComponent::QmitkConnectivityFilterComponent(QObject * parent, const char * parentName, QmitkStdMultiWidget *mitkStdMultiWidget, mitk::DataTreeIteratorBase* it, bool updateSelector, bool showSelector)
: QmitkInteractionFunctionalityComponent(parent, parentName, mitkStdMultiWidget, it),
m_ConnectivityFilterComponentGUI(NULL),
m_ConnectivityNode(NULL),
m_PointSetNode(NULL),
m_PointSetInteractor(NULL),
m_Seeds(NULL)
{
  SetDataTreeIterator(it);
  SetAvailability(true);

  SetComponentName("PointSet");
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
 mitk::DataTreeNode::Pointer QmitkConnectivityFilterComponent::GetPointSetNode()
 {
  return  m_ConnectivityNode;
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
m_ConnectivityFilterComponentGUI->GetTreeNodeSelectorContentGroupBox()->setShown(m_ConnectivityFilterComponentGUI->GetShowTreeNodeSelectorGroupBox()->isChecked());
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

  return m_ConnectivityFilterComponentGUI;
}

/***************        ACTIVATED       ***************/
void QmitkConnectivityFilterComponent::Activated()
{
  QmitkBaseFunctionalityComponent::Activated();
  m_Active = true;
  TreeChanged();

    if (m_PointSetNode.IsNull())
  {
    //Points are to define the two Points for the ThresholdGradient
    //add Point with crtl + leftMouseButton
    m_Seeds = mitk::PointSet::New();

    m_PointSetNode = mitk::DataTreeNode::New();
    m_PointSetNode->SetData(m_Seeds);
    mitk::ColorProperty::Pointer color = new mitk::ColorProperty(0.2, 0.0, 0.8);
    mitk::Point3D colorTwo; 
    mitk::FillVector3D(colorTwo, 0.2, 0.0, 0.8);
    m_ConnectivityFilterComponentGUI->GetPointListWidget()->SwitchInteraction(&m_PointSetInteractor, &m_PointSetNode, 2, colorTwo,"Points ");  //-1 for unlimited points
    m_PointSetNode->SetProperty("color",color);
    m_PointSetNode->SetIntProperty("layer", 101);
    m_PointSetNode->SetProperty("name", new mitk::StringProperty("SeedPointsForConnectiviy"));

    m_PointSetNode->SetProperty( "SeedPoints", new mitk::BoolProperty(true));

    m_PointSetInteractor = new mitk::PointSetInteractor("pointsetinteractor", m_PointSetNode, 2);

    m_DataTreeIterator.GetPointer()->Add(m_PointSetNode);

    mitk::GlobalInteraction::GetInstance()->AddInteractor(m_PointSetInteractor);
  }
  else 
  {
    mitk::GlobalInteraction::GetInstance()->AddInteractor(m_PointSetInteractor);
  }
}

/***************       DEACTIVATED      ***************/
void QmitkConnectivityFilterComponent::Deactivated()
{
 if (m_PointSetNode.IsNotNull())
 {
   mitk::GlobalInteraction::GetInstance()->RemoveInteractor(m_PointSetInteractor);
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
  // 2 show specified Region
  // 3 delete specified Region
  // 4 Show PointSeededRegions
  // 5 Delete Seed
  // 6 ShowCellSeededRegions
  // 7 ShowClosestPointRegion

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

    case 2: /*show specified Region*/
      {
        pdConnectivity->SetExtractionModeToAllRegions();
        pdConnectivity->Update();
        numberOfExtractedRegions = pdConnectivity->GetNumberOfExtractedRegions();
        int specifiedRegion = atoi(m_ConnectivityFilterComponentGUI->GetParameterLineEdit()->text());
        
        for(int i = 0; i <= numberOfExtractedRegions; i++)
        {
          if(i != specifiedRegion)
          {
            pdConnectivity->SetColorRegions(i);
          }
        }

        //pdConnectivity->AddSpecifiedRegion(specifiedRegion);
        //pdConnectivity->SetExtractionModeToSpecifiedRegions(); 
      }
      break; 

    case 3: /*delete specified Region*/
      {
        int specifiedRegion = atoi(m_ConnectivityFilterComponentGUI->GetParameterLineEdit()->text());

        pdConnectivity->DeleteSpecifiedRegion(specifiedRegion);
        pdConnectivity->SetExtractionModeToAllRegions();  
      }
      break;

    case 4: /*Show PointSeededRegions*/
      {
         int specifiedRegion = atoi(m_ConnectivityFilterComponentGUI->GetParameterLineEdit()->text());

        pdConnectivity->AddSeed(specifiedRegion);
         pdConnectivity->SetExtractionModeToPointSeededRegions();
      }
      break;

    case 5: /*Delete Seed*/
      {
        vtkIdType specifiedSeed = atoi(m_ConnectivityFilterComponentGUI->GetParameterLineEdit()->text());
        pdConnectivity->DeleteSeed(specifiedSeed);
        pdConnectivity->SetExtractionModeToAllRegions();  
      }
      break;

    case 6: /*ShowCellSeededRegions*/
      {
        pdConnectivity->SetExtractionModeToCellSeededRegions();
      }
      break;

    case 7: /*ShowClosestPointRegion*/
      {
                mitk::PointSet::Pointer pointSet = m_Seeds;
        int numberOfPoints = pointSet->GetSize();

        if(numberOfPoints > 0)
        {
        mitk::PointSet::PointType pointOne = pointSet->GetPoint(0);
          float x1 = pointOne[0];
          float y1 = pointOne[1];
          float z1 = pointOne[2];
         
         pdConnectivity->SetClosestPoint(x1, y1, z1);
         pdConnectivity->SetExtractionModeToClosestPointRegion();
        }
      }
      break;

    default: ; break; 
    }

  pdConnectivity->ColorRegionsOn();
  pdConnectivity->Update();

  numberOfExtractedRegions = pdConnectivity->GetNumberOfExtractedRegions();
//
  //to set different colors 
#if ((VTK_MAJOR_VERSION > 4) || ((VTK_MAJOR_VERSION==4) && (VTK_MINOR_VERSION>=4) ))
  double* range = pdConnectivity->GetOutput()->GetPointData()->GetScalars()->GetRange();
#else
  float* range = pdConnectivity->GetOutput()->GetPointData()->GetScalars()->GetRange();
#endif
  float scalarsMin = range[0];
  float scalarsMax = range[1];

  mitk::DataTreeIteratorClone itConnectivity = GetDataTreeIterator();
//  std::cout<<"while:"<<std::endl;
  int i = 0;
  while ( !itConnectivity->IsAtEnd() )
  {
    i++;
//    std::cout<<i<<std::endl;
    mitk::DataTreeNode::Pointer aNode = itConnectivity->Get();
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
        mitk::DataTreeIteratorClone iteratorOnImageToBeSkinExtracted = m_ConnectivityFilterComponentGUI->GetTreeNodeSelector()->GetFilter()->GetIteratorToSelectedItem(); 
        iteratorOnImageToBeSkinExtracted->Get()->GetIntProperty("layer", layer);
        connectivityNode->SetIntProperty("layer", layer+1);
        connectivityNode->SetProperty("deprecated usePointDataForColouring", new mitk::BoolProperty(true));
        connectivityNode->SetProperty("name", new mitk::StringProperty("connectivity") );
        connectivityNode->SetProperty("ScalarsRangeMaximum", new mitk::FloatProperty(scalarsMax));
        connectivityNode->SetProperty("ScalarsRangeMinimum", new mitk::FloatProperty(scalarsMin));

        mitk::DataTreeIteratorClone iteratorConnectivity = m_DataTreeIterator;

        bool isSurface = false;
        bool isConnectivity = false;

        while(!(iteratorConnectivity->IsAtEnd())&&(isSurface == false))
        {
          iteratorConnectivity->Get()->GetBoolProperty("Surface", isSurface);
          if(isSurface == false)
          {
            ++iteratorConnectivity;
          }
        }//end of while(!(iteratorConnectivity->IsAtEnd())&&(isSurface == false))

        if(isSurface == false)
        {
          while(!(iteratorConnectivity->IsAtEnd())&&(isConnectivity == false))
          {
            iteratorConnectivity->Get()->GetBoolProperty("connectivity", isConnectivity);
            if(isConnectivity == false)
            {
              ++iteratorConnectivity;
            }
          }
        }//end of if(isSurface == false)

        if(isSurface == true)
        {
          itConnectivity->Set(connectivityNode);
          itConnectivity->GetTree()->Modified();
        }//end of if(isSurface == true)

        if(isSurface == false)
        {
          if(isConnectivity == true)
          {

            itConnectivity->Set(connectivityNode);
            itConnectivity->GetTree()->Modified();
          }

          if(isConnectivity == false)
          {
            itConnectivity->Add(connectivityNode);
          }
        }//end of if(isSurface == false)

        break;
      }//end of if (sf.IsNotNull())
    }//end of if ( aNode->GetData() != NULL )
    ++itConnectivity;
  }//end of while ( !itConnectivity->IsAtEnd() )
//  multiWidget->RequestUpdate();
}//end of if itemSelector is not empty
}//end of showConnected()