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
#include "QmitkSurfaceCreatorComponent.h"
#include "QmitkSurfaceCreatorComponentGUI.h"

#include <QmitkDataTreeComboBox.h>
#include "QmitkCommonFunctionality.h"
#include "QmitkStdMultiWidget.h"

#include "mitkRenderWindow.h"
#include "mitkRenderingManager.h"
#include "mitkProperties.h"
#include <mitkColorSequenceRainbow.h>
#include <mitkManualSegmentationToSurfaceFilter.h>

#include <qlineedit.h>
#include <qslider.h>
#include <qgroupbox.h>
#include <qapplication.h>
#include <qcursor.h>


/***************       CONSTRUCTOR      ***************/
QmitkSurfaceCreatorComponent::QmitkSurfaceCreatorComponent(QObject *parent, const char *name, QmitkStdMultiWidget *mitkStdMultiWidget, mitk::DataTreeIteratorBase* it, bool updateSelector, bool showSelector):
m_Name(name),
m_MultiWidget(mitkStdMultiWidget),
m_DataTreeIteratorClone(NULL),
m_UpdateSelector(updateSelector),
m_ShowSelector(showSelector),
m_Active(false),
m_GUI(NULL),
m_SelectedImage(NULL),
m_MitkImage(NULL),
m_MitkImageIterator(NULL),
m_SurfaceCounter(0),
m_r(0.75),
m_g(0.42),
m_b(-0.75), 
m_Threshold(1)
{
  SetDataTreeIterator(it);
  m_Color.Set(1.0, 0.67, 0.0);
  m_RainbowColor = new mitk::ColorSequenceRainbow();
}

/***************        DESTRUCTOR      ***************/
QmitkSurfaceCreatorComponent::~QmitkSurfaceCreatorComponent()
{

}

/*************** GET FUNCTIONALITY NAME ***************/
QString QmitkSurfaceCreatorComponent::GetFunctionalityName()
{
  return m_Name;
}

/*************** SET FUNCTIONALITY NAME ***************/
void QmitkSurfaceCreatorComponent::SetFunctionalityName(QString name)
{
  m_Name = name;
}

/*************** GET FUNCCOMPONENT NAME ***************/
QString QmitkSurfaceCreatorComponent::GetFunctionalityComponentName()
{
  return m_Name;
}

/*************** GET DATA TREE ITERATOR ***************/
mitk::DataTreeIteratorBase* QmitkSurfaceCreatorComponent::GetDataTreeIterator()
{
  return m_DataTreeIterator.GetPointer();
}

/*************** SET DATA TREE ITERATOR ***************/
void QmitkSurfaceCreatorComponent::SetDataTreeIterator(mitk::DataTreeIteratorBase* it)
{
  m_DataTreeIterator = it;
}

/***************         GET GUI        ***************/
QWidget* QmitkSurfaceCreatorComponent::GetGUI()
{
  return m_GUI;
}

/*************** TREE CHANGED (       ) ***************/
void QmitkSurfaceCreatorComponent::TreeChanged()
{

}

/***************       CONNECTIONS      ***************/
void QmitkSurfaceCreatorComponent::CreateConnections()
{
  if ( m_GUI )
  {
    connect( (QObject*)(m_GUI->GetTreeNodeSelector()), SIGNAL(activated(const mitk::DataTreeFilter::Item *)), (QObject*) this, SLOT(ImageSelected(const mitk::DataTreeFilter::Item *)));        
    connect( (QObject*)(m_GUI->GetSurfaceCreatorGroupBox()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(ShowSurfaceCreatorContent(bool)));
    connect( (QObject*)(m_GUI->GetSelectDataGroupBox()), SIGNAL(toggled(bool)), (QObject*) this, SLOT(ShowImageContent(bool)));

    connect( (QObject*)(m_GUI->GetCreateSurfaceButton()), SIGNAL(pressed()), (QObject*) this, SLOT(CreateSurface()));
    connect( (QObject*)(m_GUI->GetThresholdLineEdit()), SIGNAL(returnPressed()), (QObject*) this, SLOT(CreateSurface()));
  }
}

/***************     IMAGE SELECTED     ***************/
void QmitkSurfaceCreatorComponent::ImageSelected(const mitk::DataTreeFilter::Item * imageIt)
{
  m_SelectedImage = imageIt;
  mitk::DataTreeFilter::Item* currentItem(NULL);
  if(m_GUI)
  {
    if(mitk::DataTreeFilter* filter = m_GUI->GetTreeNodeSelector()->GetFilter())
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
  if(m_GUI != NULL)
  {

    for(unsigned int i = 0;  i < m_AddedChildList.size(); i++) 
    {
      m_AddedChildList[i]->ImageSelected(m_SelectedImage);
    }
  }
  TreeChanged();

  if(m_GUI)
  {
    mitk::DataTreeFilter* filter = m_GUI->GetTreeNodeSelector()->GetFilter();
    m_MitkImageIterator = filter->GetIteratorToSelectedItem();
    
    if(m_MitkImageIterator.GetPointer())
    {
      m_MitkImage = static_cast<mitk::Image*> (m_MitkImageIterator->Get()->GetData());
    }
  }
}

/*************** CREATE CONTAINER WIDGET **************/
QWidget* QmitkSurfaceCreatorComponent::CreateContainerWidget(QWidget* parent)
{
  m_GUI = new QmitkSurfaceCreatorComponentGUI(parent);
  m_GUI->GetTreeNodeSelector()->SetDataTree(GetDataTreeIterator());
 
  if(!m_ShowSelector)
  {
    m_GUI->GetSelectDataGroupBox()->setShown(false);
  }

  return m_GUI;

}

/************** SET SELECTOR VISIBILITY ***************/
void QmitkSurfaceCreatorComponent::SetSelectorVisibility(bool visibility)
{
  if(m_GUI)
  {
    m_GUI->GetSelectDataGroupBox()->setShown(visibility);
  }
  m_ShowSelector = visibility;
}


/***************        ACTIVATED       ***************/
void QmitkSurfaceCreatorComponent::Activated()
{
  m_Active = true;
}

/***************       DEACTIVATED      ***************/
void QmitkSurfaceCreatorComponent::Deactivated()
{
  m_Active = false;
}

///************ SHOW SURFACE CREATOR CONTENT ************/
void QmitkSurfaceCreatorComponent::ShowSurfaceCreatorContent(bool)
{
  if(m_ShowSelector)
  {
    m_GUI->GetSelectDataGroupBox()->setShown(m_GUI->GetSurfaceCreatorGroupBox()->isChecked());
  }
}

///***************    SHOW IMAGE CONTENT   **************/
void QmitkSurfaceCreatorComponent::ShowImageContent(bool)
{
  m_GUI->GetImageContent()->setShown(m_GUI->GetSelectDataGroupBox()->isChecked());
}

/****************************************************************************SURFACE CREATOR***************************************************************************/

void QmitkSurfaceCreatorComponent::SetThreshold(int threshold)
{
  m_Threshold = threshold;
  m_GUI->SetThreshold(threshold);
}

void QmitkSurfaceCreatorComponent::SetThreshold(const QString& threshold)
{
  m_GUI->SetThreshold(atoi(threshold));
}

///***************      CREATE SURFACE     **************/
void QmitkSurfaceCreatorComponent::CreateSurface()
{
  QApplication::setOverrideCursor( QCursor(Qt::WaitCursor) );
  if(m_MitkImage != NULL)
  {
  //Value Gauss
  //float gsDev = 1.5;

  //Value for DecimatePro 
  float targetReduction = 0.05;

  //ImageToSurface Instance
  mitk::DataTreeIteratorClone  iteratorOnImageToBeSkinExtracted;
  if(m_MitkImageIterator.IsNotNull())
  {
      
    iteratorOnImageToBeSkinExtracted = m_MitkImageIterator;
  }
  else 
  {
    iteratorOnImageToBeSkinExtracted = CommonFunctionality::GetIteratorToFirstImage(m_DataTreeIterator.GetPointer());
  }

  mitk::ManualSegmentationToSurfaceFilter::Pointer filter = mitk::ManualSegmentationToSurfaceFilter::New();
  if (filter.IsNull())
  {
    std::cout<<"NULL Pointer for ManualSegmentationToSurfaceFilter"<<std::endl;
    return;
  }

  filter->SetInput( m_MitkImage );  
  filter->SetStandardDeviation( 0.5 );
  filter->SetUseStandardDeviation( true );
  filter->SetThreshold( m_GUI->GetThreshold()); //if( Gauss ) --> TH manipulated for vtkMarchingCube
  

  filter->SetTargetReduction( targetReduction );

  mitk::DataTreeNode::Pointer surfaceNode = mitk::DataTreeNode::New(); 
  surfaceNode->SetData( filter->GetOutput() );
  
  int layer = 0;

  ++m_SurfaceCounter;
  std::ostringstream buffer;
  buffer << m_SurfaceCounter;
  std::string surfaceNodeName = "Surface " + buffer.str();

  m_r += 0.25; if(m_r > 1){m_r = m_r - 1;}
  m_g += 0.25; if(m_g > 1){m_g = m_g - 1;}
  m_b += 0.25; if(m_b > 1){m_b = m_b - 1;}

  iteratorOnImageToBeSkinExtracted->Get()->GetIntProperty("layer", layer);
  mitk::DataTreeNodeFactory::SetDefaultSurfaceProperties(surfaceNode);
  surfaceNode->SetIntProperty("layer", layer+1);
  surfaceNode->SetProperty("Surface", new mitk::BoolProperty(true));
  surfaceNode->SetProperty("name", new mitk::StringProperty(surfaceNodeName));
  
  
  mitk::DataTreeIteratorClone iteratorClone = m_DataTreeIterator;
  bool isSurface = false;

  while(!(iteratorClone->IsAtEnd())&&(isSurface == false))
  {
    iteratorClone->Get()->GetBoolProperty("Surface", isSurface);
    if(isSurface == false)
    {
      ++iteratorClone;
    }
  }
   iteratorClone= iteratorOnImageToBeSkinExtracted;  
   iteratorClone->Add(surfaceNode);
   iteratorClone->GetTree()->Modified();


  mitk::Surface::Pointer surface = filter->GetOutput();
 
  //to show surfaceContur
  m_Color = m_RainbowColor->GetNextColor();
  surfaceNode->SetColor(m_Color);
  surfaceNode->SetVisibility(true);

  m_MultiWidget->RequestUpdate();

  }//if m_MitkImage != NULL

  QApplication::restoreOverrideCursor();
}
