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

//PluginEvents
#include "mitkChiliPluginEvents.h"
//to add a new node
#include "mitkDataTreeNodeFactory.h"
//for the GUI
#include <qlistview.h>
#include <qradiobutton.h>
#include <qmessagebox.h>
//predicates
#include <mitkNodePredicateOR.h>
#include <mitkNodePredicateNOT.h>
#include <mitkNodePredicateData.h>
#include <mitkNodePredicateProperty.h>

QmitkLoadSaveToChiliExample::QmitkLoadSaveToChiliExample( QObject *parent, const char *name, QmitkStdMultiWidget *mitkStdMultiWidget, mitk::DataTreeIteratorBase* it )
: QmitkFunctionality( parent, name, it ),
  m_Plugin( NULL ),
  m_MultiWidget( mitkStdMultiWidget ),
  m_Controls( NULL )
{
  SetAvailability( true );
  // register to chili plugin as observer
  m_Plugin = mitk::PACSPlugin::GetInstance();
  if( m_Plugin )
  {
    itk::ReceptorMemberCommand<QmitkLoadSaveToChiliExample>::Pointer command = itk::ReceptorMemberCommand<QmitkLoadSaveToChiliExample>::New();
    command->SetCallbackFunction( this, &QmitkLoadSaveToChiliExample::PluginEventNewStudySelected );
    m_Plugin->AddObserver( mitk::PluginStudySelected(), command );
  }
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
    connect( ( QObject* )( m_Controls->LoadFromListView ), SIGNAL( clicked() ), ( QObject* ) this, SLOT( LoadFromListView() ) );

    connect( ( QObject* )( m_Controls->SaveToChili ), SIGNAL( clicked() ), ( QObject* ) this, SLOT( SaveToChili() ) );

    connect( ( QObject* )( m_Controls->ImageNumberFilter ), SIGNAL( clicked() ), ( QObject* ) this, SLOT( ChangeReaderType() ) );
    connect( ( QObject* )( m_Controls->SpacingFilter ), SIGNAL( clicked() ), ( QObject* ) this, SLOT( ChangeReaderType() ) );
    connect( ( QObject* )( m_Controls->SingleSpacingFilter ), SIGNAL( clicked() ), ( QObject* ) this, SLOT( ChangeReaderType() ) );

    connect( ( QObject* )( m_Controls->LoadParentChildRelation ), SIGNAL( clicked() ), ( QObject* ) this, SLOT( LoadParentChildRelation() ) );
  }
}

QAction * QmitkLoadSaveToChiliExample::CreateAction( QActionGroup *parent )
{
  QAction* action;
  action = new QAction( tr( "Example for Save and Load to Chili" ), QPixmap( (const char**) icon_xpm ), tr( "&Example for Save and Load to Chili" ), 0, parent, "QmitkLoadSaveToChiliExample" );
  return action;
}

void QmitkLoadSaveToChiliExample::Activated()
{
  QmitkFunctionality::Activated();
}

void QmitkLoadSaveToChiliExample::LoadParentChildRelation()
{
  m_Plugin->LoadParentChildRelation( m_Plugin->GetSeriesInformation().OID );
}

void QmitkLoadSaveToChiliExample::LoadFromListView()
{
  if( m_Controls->contentOfStudy->selectedItem() != 0 )
  {
    if( m_Controls->contentOfStudy->selectedItem()->depth() == 0 )  //load complete series
    {
      QmitkPluginListViewItem* entry = dynamic_cast<QmitkPluginListViewItem*>( m_Controls->contentOfStudy->selectedItem() );
      if( entry )
      {
        std::string savedOID = entry->GetChiliOID();
        AddNodesToDataTree( m_Plugin->LoadCompleteSeries( savedOID ) );
      }
    }
    else
      if( m_Controls->contentOfStudy->selectedItem()->depth() == 1 )
      {
        if( m_Controls->contentOfStudy->selectedItem()->text( 0 ) == "Image" )  //load all images
        {
          QmitkPluginListViewItem* entry = dynamic_cast<QmitkPluginListViewItem*>( m_Controls->contentOfStudy->selectedItem() );
          if( entry )
          {
            std::string savedOID = entry->GetChiliOID();
            AddNodesToDataTree( m_Plugin->LoadAllImagesFromSeries( savedOID ) );
          }
        }
        else
          if( m_Controls->contentOfStudy->selectedItem()->text( 0 ) == "Text" )  //load all text
          {
            QmitkPluginListViewItem* entry = dynamic_cast<QmitkPluginListViewItem*>( m_Controls->contentOfStudy->selectedItem() );
            if( entry )
            {
              std::string savedOID = entry->GetChiliOID();
              AddNodesToDataTree( m_Plugin->LoadAllTextsFromSeries( savedOID ) );
            }
          }
      }
      else
        if( m_Controls->contentOfStudy->selectedItem()->depth() == 3 )  //load single text
        {
          QmitkPluginListViewItem* entry = dynamic_cast<QmitkPluginListViewItem*>( m_Controls->contentOfStudy->selectedItem() );
          if( entry )
          {
            std::string savedOID = entry->GetChiliOID();
            mitk::DataStorage::GetInstance()->Add( m_Plugin->LoadOneText( savedOID ) );
          }

          m_MultiWidget->InitializeStandardViews( this->GetDataTreeIterator() );
          m_MultiWidget->Fit();
          m_MultiWidget->ReInitializeStandardViews();
        }
  }
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

  m_Plugin->SaveToChili( mitk::DataStorage::GetInstance()->GetSubset( notpred ) );
}

void QmitkLoadSaveToChiliExample::ChangeReaderType()
{
  if( m_Controls->ImageNumberFilter->isChecked() )
    m_Plugin->SetReaderType( 0 );
  else
    if( m_Controls->SpacingFilter->isChecked() )
      m_Plugin->SetReaderType( 1 );
    else
      if( m_Controls->SingleSpacingFilter->isChecked() )
        m_Plugin->SetReaderType( 2 );
}

void QmitkLoadSaveToChiliExample::AddNodesToDataTree( std::vector<mitk::DataTreeNode::Pointer> resultNodes)
{
  bool showAll = true;
  bool askQuestion = false;

  mitk::BaseProperty::Pointer propertyImageNumber;
  mitk::BaseProperty::Pointer propertyTimeSlice;

  for( unsigned int n = 0; n < resultNodes.size(); n++ )
  {
    if( resultNodes[n].IsNotNull() )
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
  }

  //initialize the multiwidget (input changed)
  m_MultiWidget->InitializeStandardViews( this->GetDataTreeIterator() );
  m_MultiWidget->Fit();
  m_MultiWidget->ReInitializeStandardViews();
}

void QmitkLoadSaveToChiliExample::PluginEventNewStudySelected( const itk::EventObject& )
{
  m_Controls->contentOfStudy->clear();
  mitk::PACSPlugin::SeriesInformationList tempSeriesList = m_Plugin->GetSeriesInformationList();
  for( std::list<mitk::PACSPlugin::SeriesInformation>::iterator iter = tempSeriesList.begin(); iter != tempSeriesList.end(); iter++ )
  {
    QmitkPluginListViewItem* seriesParent;

    //seriesDescription
    if( iter->Description == "" )
      seriesParent = new QmitkPluginListViewItem( iter->OID.c_str(), m_Controls->contentOfStudy, "no description" );
    else
      seriesParent = new QmitkPluginListViewItem( iter->OID.c_str(), m_Controls->contentOfStudy, iter->Description.c_str() );

    //imageCount, seriesNumber
    QmitkPluginListViewItem* image = new QmitkPluginListViewItem( iter->OID.c_str(), seriesParent, "Image" );
    std::ostringstream seriesNumber;
    seriesNumber << "SeriesNumber: ";
    if( iter->Number != -1 )
      seriesNumber << iter->Number;
    new QmitkPluginListViewItem( "", image , seriesNumber.str().c_str() );

    std::ostringstream imageCount;
    imageCount << "ImageCount: " << iter->ImageCount;
    new QmitkPluginListViewItem( "", image , imageCount.str().c_str() );

    //text
    mitk::PACSPlugin::TextInformationList tempTextList = m_Plugin->GetTextInformationList( iter->OID.c_str() );
    std::vector< MimeTypeStruct > mimeTypeVector;

    QmitkPluginListViewItem* textItem = NULL;

    for( std::list<mitk::PACSPlugin::TextInformation>::iterator it = tempTextList.begin(); it != tempTextList.end(); it++)
    {
      //text-Name
      char* textName;
      textName = strrchr( (char*) it->ChiliText.c_str(), '-' );
      textName++;
      //search if MimeType always exist
      std::vector< MimeTypeStruct >::iterator searchBegin = mimeTypeVector.begin();
      std::vector< MimeTypeStruct >::iterator searchEnd = mimeTypeVector.end();
      bool found = false;

      while( searchBegin != searchEnd )
      {
        //if MimeType exist, add the current found
        if( searchBegin->mimeType == it->MimeType )
        {
          //create singe "mime-type-item"
          new QmitkPluginListViewItem( it->OID.c_str(), searchBegin->parentItem, textName );
          //break up search
          searchBegin = searchEnd;
          found = true;
        }
        else searchBegin++;
      }

      //if this MimeType not shown yet
      if( !found )
      {
        //create the "main-text-item"
        if( textItem == NULL )
          textItem = new QmitkPluginListViewItem( iter->OID.c_str(), seriesParent, "Text" );
        //create new "mime-type-group"
        MimeTypeStruct newMimeType;
        newMimeType.mimeType = it->MimeType;
        newMimeType.parentItem = new QmitkPluginListViewItem( "", textItem, it->MimeType.c_str() );
        //create single "mime-type-item"
        new QmitkPluginListViewItem( it->OID.c_str(), newMimeType.parentItem, textName );
        mimeTypeVector.push_back( newMimeType );
      }
    }
  }
}

