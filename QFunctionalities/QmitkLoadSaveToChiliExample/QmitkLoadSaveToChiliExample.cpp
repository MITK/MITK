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

#include "QmitkLoadSaveToChiliExample.h"
#include "QmitkLoadSaveToChiliExampleControls.h"
#include <qaction.h>
#include "icon.xpm"
#include "QmitkTreeNodeSelector.h"
#include "QmitkStdMultiWidget.h"

#include "mitkChiliPlugin.h"
//PluginEvents
#include "mitkChiliPluginEvents.h"
//to add a new node
#include "mitkDataTreeNodeFactory.h"
//for the GUI
#include <qcursor.h>
#include <qapplication.h>
#include <qlabel.h>
#include <qlistview.h>
#include <qlineedit.h>
#include <qmessagebox.h>

#include <mitkNodePredicateOR.h>
#include <mitkNodePredicateNOT.h>
#include <mitkNodePredicateData.h>
#include <mitkNodePredicateProperty.h>

QmitkLoadSaveToChiliExample::QmitkLoadSaveToChiliExample( QObject *parent, const char *name, QmitkStdMultiWidget *mitkStdMultiWidget, mitk::DataTreeIteratorBase* it )
: QmitkFunctionality( parent, name, it ),
  m_MultiWidget( mitkStdMultiWidget ),
  m_Controls( NULL )
{
  SetAvailability( true );
/*
  // register to chili plugin as observer
  mitk::ChiliPlugin* plugin = mitk::ChiliPlugin::GetInstance();

  if( plugin )
  {
    itk::ReceptorMemberCommand<QmitkLoadSaveToChiliExample>::Pointer command = itk::ReceptorMemberCommand<QmitkLoadSaveToChiliExample>::New();
    command->SetCallbackFunction( this, &QmitkLoadSaveToChiliExample::chiliStudySelected );
    plugin->AddObserver( mitk::PluginStudySelected(), command );
  }
*/
}

QmitkLoadSaveToChiliExample::~QmitkLoadSaveToChiliExample()
{
}

QWidget * QmitkLoadSaveToChiliExample::CreateMainWidget( QWidget *parent )
{
  if( m_MultiWidget == NULL )
  {
    m_MultiWidget = new QmitkStdMultiWidget( parent );
  }
  return m_MultiWidget;
}

QWidget * QmitkLoadSaveToChiliExample::CreateControlWidget( QWidget *parent )
{
  if( m_Controls == NULL )
  {
    m_Controls = new QmitkLoadSaveToChiliExampleControls( parent );
  }
  return m_Controls;
}

void QmitkLoadSaveToChiliExample::CreateConnections()
{
  if( m_Controls )
  {
    connect( ( QObject* )( m_Controls->LoadImageFromLightBox ), SIGNAL( clicked() ), ( QObject* ) this, SLOT( LoadImageFromLightBox() ) );
    connect( ( QObject* )( m_Controls->SaveImageToLightBox ), SIGNAL( clicked() ), ( QObject* ) this, SLOT( SaveImageToLightBox() ) );

    connect( ( QObject* )( m_Controls->LoadCompleteSeries ), SIGNAL( clicked() ), ( QObject* ) this, SLOT( LoadCompleteSeries() ) );
    connect( ( QObject* )( m_Controls->LoadAllImages ), SIGNAL( clicked() ), ( QObject* ) this, SLOT( LoadAllImages() ) );
    connect( ( QObject* )( m_Controls->LoadAllTexts ), SIGNAL( clicked() ), ( QObject* ) this, SLOT( LoadAllTexts() ) );
    connect( ( QObject* )( m_Controls->LoadOneText ), SIGNAL( clicked() ), ( QObject* ) this, SLOT( LoadOneText() ) );

    connect( ( QObject* )( m_Controls->SaveToChili ), SIGNAL( clicked() ), ( QObject* ) this, SLOT( SaveToChili() ) );
  }
}

QAction * QmitkLoadSaveToChiliExample::CreateAction( QActionGroup *parent )
{
  QAction* action;
  action = new QAction( tr( "Example for Save and Load to Chili" ), QPixmap( (const char**) icon_xpm ), tr( "&Example for Save and Load to Chili" ), 0, parent, "QmitkLoadSaveToChiliExample" );
  return action;
}

void QmitkLoadSaveToChiliExample::LoadImageFromLightBox()
{
  AddNodesToDataTree( mitk::ChiliPlugin::GetInstance()->LoadImagesFromLightbox( mitk::ChiliPlugin::GetInstance()->GetCurrentLightbox() ) );
}

void QmitkLoadSaveToChiliExample::SaveImageToLightBox()
{

  mitk::DataTreeNode* node = m_Controls->m_TreeNodeSelector->GetSelectedNode();
  if( node )
  {
    mitk::BaseData* data = node->GetData();
    if( data )
    {
      mitk::Image* image = dynamic_cast<mitk::Image*>( data );
      if( image )
      {
        mitk::ChiliPlugin::GetInstance()->SaveImageToLightbox( image, node->GetPropertyList(), mitk::ChiliPlugin::GetInstance()->GetNewLightbox() );
      }
    }
  }
}

void QmitkLoadSaveToChiliExample::LoadCompleteSeries()
{
  AddNodesToDataTree( mitk::ChiliPlugin::GetInstance()->LoadCompleteSeries( mitk::ChiliPlugin::GetInstance()->GetSeriesInformation().OID ) );
}

void QmitkLoadSaveToChiliExample::LoadAllImages()
{
  AddNodesToDataTree( mitk::ChiliPlugin::GetInstance()->LoadAllImagesFromSeries( mitk::ChiliPlugin::GetInstance()->GetSeriesInformation().OID ) );
}

void QmitkLoadSaveToChiliExample::LoadAllTexts()
{
  AddNodesToDataTree( mitk::ChiliPlugin::GetInstance()->LoadAllTextsFromSeries( mitk::ChiliPlugin::GetInstance()->GetSeriesInformation().OID ) );
}

void QmitkLoadSaveToChiliExample::LoadOneText()
{
  //AddNodesToDataTree( mitk::ChiliPlugin::GetInstance()->LoadOneTextFromSeries( mitk::ChiliPlugin::GetInstance()->GetSeriesInformation().OID ) );
}

void QmitkLoadSaveToChiliExample::SaveToChili()
{
  mitk::NodePredicateProperty w1( "name", new mitk::StringProperty("Widgets") );
  mitk::NodePredicateProperty w2( "name", new mitk::StringProperty("widget1Plane") );
  mitk::NodePredicateProperty w3( "name", new mitk::StringProperty("widget2Plane") );
  mitk::NodePredicateProperty w4( "name", new mitk::StringProperty("widget3Plane") );
  mitk::NodePredicateData w5( NULL );
  mitk::NodePredicateOR orpred;
  orpred.AddPredicate( w1 );
  orpred.AddPredicate( w2 );
  orpred.AddPredicate( w3 );
  orpred.AddPredicate( w4 );
  orpred.AddPredicate( w5 );
  mitk::NodePredicateNOT notpred( orpred );

  mitk::ChiliPlugin::GetInstance()->SaveToChili( mitk::DataStorage::GetInstance()->GetSubset( notpred ) );
}

void QmitkLoadSaveToChiliExample::AddNodesToDataTree( std::vector<mitk::DataTreeNode::Pointer> resultNodes)
{

  bool showAll = true;
  bool askQuestion = false;

  mitk::BaseProperty::Pointer propertyImageNumber;
  mitk::BaseProperty::Pointer propertyTimeSlice;

  for( unsigned int n = 0; n < resultNodes.size(); n++ )
  {
    propertyImageNumber = resultNodes[n]->GetProperty( "NumberOfSlices" );
    propertyTimeSlice = resultNodes[n]->GetProperty( "NumberOfTimeSlices" );

    if( propertyImageNumber.IsNotNull() && propertyTimeSlice.IsNotNull() )
    {
      if( propertyImageNumber->GetValueAsString() == "1" && propertyTimeSlice->GetValueAsString() == "1" )
      {
        if(askQuestion == false )
          if( QMessageBox::question( 0, tr("MITK"), QString("MITK detected 2D-ResultImages.\nYou can choose if you want to add them to the mitk::DataTree or not.\nMore Images makes the Application slower, so if you want to see the volume only, dont add them.\n\nDo you want to add all 2D-Images to the MITK::DataTree?"), QMessageBox::Yes, QMessageBox::No ) == QMessageBox::Yes )
          {
            showAll = true;
            askQuestion = true;
            mitk::DataStorage::GetInstance()->Add( resultNodes[n] );
          }
          else
          {
            showAll = false;
            askQuestion = true;
          }
        else
          if( showAll == true ) mitk::DataStorage::GetInstance()->Add( resultNodes[n] );
      }
      else  mitk::DataStorage::GetInstance()->Add( resultNodes[n] );
    }
    else mitk::DataStorage::GetInstance()->Add( resultNodes[n] );
  }

  //initialize the multiwidget (input changed)
  m_MultiWidget->InitializeStandardViews( this->GetDataTreeIterator() );
  m_MultiWidget->Fit();
  m_MultiWidget->ReInitializeStandardViews();
  //update the treenodeselector
  m_Controls->m_TreeNodeSelector->UpdateContent();
}

void QmitkLoadSaveToChiliExample::TreeChanged()
{
  m_Controls->m_TreeNodeSelector->SetDataTreeNodeIterator( this->GetDataTreeIterator() );
}

void QmitkLoadSaveToChiliExample::Activated()
{
  QmitkFunctionality::Activated();
}
