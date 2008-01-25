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
    connect( ( QObject* )( m_Controls->LoadFromStudyListView ), SIGNAL( clicked() ), ( QObject* ) this, SLOT( LoadFromStudyListView() ) );
    connect( ( QObject* )( m_Controls->LoadFromPSListView ), SIGNAL( clicked() ), ( QObject* ) this, SLOT( LoadFromPSListView() ) );

    connect( ( QObject* )( m_Controls->SaveToChili ), SIGNAL( clicked() ), ( QObject* ) this, SLOT( SaveToChili() ) );

    connect( ( QObject* )( m_Controls->ImageNumberFilter ), SIGNAL( clicked() ), ( QObject* ) this, SLOT( ChangeReaderType() ) );
    connect( ( QObject* )( m_Controls->SpacingFilter ), SIGNAL( clicked() ), ( QObject* ) this, SLOT( ChangeReaderType() ) );
    connect( ( QObject* )( m_Controls->SingleSpacingFilter ), SIGNAL( clicked() ), ( QObject* ) this, SLOT( ChangeReaderType() ) );
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

/** This function load the selected element from the study-listview */
void QmitkLoadSaveToChiliExample::LoadFromStudyListView()
{
  if( m_Controls->studyContent->selectedItem() != 0 )  //element selected?
  {
    switch ( m_Controls->studyContent->selectedItem()->depth() )
    {
      case 0:  //load complete series
      {
        QmitkPluginListViewItem* entry = dynamic_cast<QmitkPluginListViewItem*>( m_Controls->studyContent->selectedItem() );
        if( entry )
        {
          std::string savedOID = entry->GetChiliOID();
          AddNodesToDataTree( m_Plugin->LoadCompleteSeries( savedOID ) );
        }
        break;
      }
      case 1:  //load all known images or all text
      {
        if( m_Controls->studyContent->selectedItem()->text( 0 ) == "Known Images" )
        {
          QmitkPluginListViewItem* entry = dynamic_cast<QmitkPluginListViewItem*>( m_Controls->studyContent->selectedItem() );
          if( entry )
          {
            std::string savedOID = entry->GetChiliOID();
            mitk::PACSPlugin::PSRelationInformationList getAllElements = m_Plugin->GetSeriesRelationInformation( savedOID );
            while( !getAllElements.empty() )
            {
              mitk::DataTreeNode::Pointer temp = m_Plugin->LoadParentChildElement( savedOID, getAllElements.front().label );
              if( temp.IsNotNull() )
                mitk::DataStorage::GetInstance()->Add( temp );
              getAllElements.pop_front();
            }
          }
        }
        else
          if( m_Controls->studyContent->selectedItem()->text( 0 ) == "Saved Text" )
          {
            QmitkPluginListViewItem* entry = dynamic_cast<QmitkPluginListViewItem*>( m_Controls->studyContent->selectedItem() );
            if( entry )
            {
              std::string savedOID = entry->GetChiliOID();
              AddNodesToDataTree( m_Plugin->LoadAllTextsFromSeries( savedOID ) );
            }
          }
        break;
      }
      case 2:  //load single parent-child-volumes or text-groups with specific MIME-Types
      {
        QmitkPluginListViewItem* entry = dynamic_cast<QmitkPluginListViewItem*>( m_Controls->studyContent->selectedItem() );
        if( entry->parent() != 0 )
        {
          QmitkPluginListViewItem* parent = dynamic_cast<QmitkPluginListViewItem*>( entry->parent() );  //use the parent to know what to do
          if( parent->text( 0 ) == "Known Images" )
          {
            std::string vLabel = entry->GetVolumeLabel();
            mitk::DataTreeNode::Pointer temp = m_Plugin->LoadParentChildElement( entry->GetChiliOID(), vLabel );
            if( temp.IsNotNull() )
              mitk::DataStorage::GetInstance()->Add( temp );
          }
          else
            if( parent->text( 0 ) == "Saved Text" )
            {
              QmitkPluginListViewItem* child = dynamic_cast<QmitkPluginListViewItem*>( entry->firstChild() );
              std::string savedOID = child->GetChiliOID();
              mitk::DataTreeNode::Pointer temp = m_Plugin->LoadOneText( savedOID );
              if( temp.IsNotNull() )
                mitk::DataStorage::GetInstance()->Add( temp );
              while( child->nextSibling() != 0 )
              {
                child = dynamic_cast<QmitkPluginListViewItem*>( child->nextSibling() );
                std::string oid = child->GetChiliOID();
                temp = m_Plugin->LoadOneText( oid );
                mitk::DataStorage::GetInstance()->Add( temp );
              }
            }
        }
        break;
      }
      case 3:  //load single text
      {
        QmitkPluginListViewItem* entry = dynamic_cast<QmitkPluginListViewItem*>( m_Controls->studyContent->selectedItem() );
        if( entry )
        {
          std::string savedOID = entry->GetChiliOID();
          mitk::DataTreeNode::Pointer temp = m_Plugin->LoadOneText( savedOID );
          if( temp.IsNotNull() )
            mitk::DataStorage::GetInstance()->Add( temp );
        }
        break;
      }
      default:
      {
        break;
      }
    }
    m_MultiWidget->InitializeStandardViews( this->GetDataTreeIterator() );
    m_MultiWidget->Fit();
    m_MultiWidget->ReInitializeStandardViews();
  }
}

void QmitkLoadSaveToChiliExample::LoadFromPSListView()
{
  if( m_Controls->PSContent->selectedItem() != 0 )
  {
    QmitkPluginListViewItem* entry = dynamic_cast<QmitkPluginListViewItem*>( m_Controls->PSContent->selectedItem() );
    if( entry )
    {
      std::string vLabel = entry->GetVolumeLabel();
      mitk::DataTreeNode::Pointer temp = m_Plugin->LoadParentChildElement( entry->GetChiliOID(), vLabel );
      if( temp.IsNotNull() )
        mitk::DataStorage::GetInstance()->Add( temp );
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
  m_Controls->studyContent->clear();
  m_Controls->PSContent->clear();

  mitk::PACSPlugin::SeriesInformationList tempSeriesList = m_Plugin->GetSeriesInformationList();
  mitk::PACSPlugin::PSRelationInformationList tempRelationList = m_Plugin->GetStudyRelationInformation();

  //fill the studyInformation-view
  for( std::list<mitk::PACSPlugin::SeriesInformation>::iterator iter = tempSeriesList.begin(); iter != tempSeriesList.end(); iter++ )
  {
    QmitkPluginListViewItem* seriesParent;

    //add seriesDescription
    if( iter->Description == "" )
      seriesParent = new QmitkPluginListViewItem( iter->OID.c_str(), "", m_Controls->studyContent, "no description" );
    else
      seriesParent = new QmitkPluginListViewItem( iter->OID.c_str(), "", m_Controls->studyContent, iter->Description.c_str() );

    //add seriesNumber
    std::ostringstream seriesNumber;
    seriesNumber << "SeriesNumber: ";
    if( iter->Number != -1 )
      seriesNumber << iter->Number;
    new QmitkPluginListViewItem( "", "", seriesParent , seriesNumber.str().c_str() );

    //add volumes which are saved in the parent-child-relation
    mitk::PACSPlugin::PSRelationInformationList elementList = m_Plugin->GetSeriesRelationInformation( iter->OID );
    if( !elementList.empty() )
    {
      QmitkPluginListViewItem* psVolumes = NULL;

      while( !elementList.empty() )
      {
        mitk::PACSPlugin::PSRelationInformation singleElement = elementList.front();
        if( singleElement.image )  //only images should add
        {
          if( psVolumes == NULL )  //no images added before
          {
            psVolumes = new QmitkPluginListViewItem( iter->OID.c_str(), "", seriesParent, "Known Images" );  //create caption
            if( iter->ImageCount != 0 )  //add ImageCount
            {
              std::ostringstream imageCount;
              imageCount << "ImageCount: " << iter->ImageCount;
              new QmitkPluginListViewItem( "", "", psVolumes , imageCount.str().c_str() );
            }
          }
          new QmitkPluginListViewItem( iter->OID.c_str(), singleElement.label.c_str(), psVolumes , singleElement.id.c_str() );
        }
        elementList.pop_front();
      }
    }

    //add text
    mitk::PACSPlugin::TextInformationList tempTextList = m_Plugin->GetTextInformationList( iter->OID.c_str() );
    std::vector< MimeTypeStruct > mimeTypeVector;

    QmitkPluginListViewItem* textItem = NULL;

    for( std::list<mitk::PACSPlugin::TextInformation>::iterator it = tempTextList.begin(); it != tempTextList.end(); it++)
    {
      //text-Name
      std::string textName = it->ChiliText.substr( it->ChiliText.find_last_of("-") + 1 );

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
          new QmitkPluginListViewItem( it->OID.c_str(), "", searchBegin->parentItem, textName.c_str() );
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
          textItem = new QmitkPluginListViewItem( iter->OID.c_str(), "", seriesParent, "Saved Text" );
        //create new "mime-type-group"
        MimeTypeStruct newMimeType;
        newMimeType.mimeType = it->MimeType;
        newMimeType.parentItem = new QmitkPluginListViewItem( "", "", textItem, it->MimeType.c_str() );
        //create single "mime-type-item"
        new QmitkPluginListViewItem( it->OID.c_str(), "", newMimeType.parentItem, textName.c_str() );
        mimeTypeVector.push_back( newMimeType );
      }
    }
  }

  //fill the parent-child-view
  for( mitk::PACSPlugin::PSRelationInformationList::iterator relationIter = tempRelationList.begin(); relationIter != tempRelationList.end(); relationIter ++)
  {
    if( relationIter->parentLabel.empty() )
    {
      QmitkPluginListViewItem* newParent = new QmitkPluginListViewItem( relationIter->oid.c_str(), relationIter->label.c_str(), m_Controls->PSContent, relationIter->id.c_str() );

      //use all Children
      for( std::list<std::string>::iterator childIter = relationIter->childLabel.begin(); childIter != relationIter->childLabel.end(); childIter++ )
      {
        //search element
        for( mitk::PACSPlugin::PSRelationInformationList::iterator searchIter = tempRelationList.begin(); searchIter != tempRelationList.end(); searchIter++ )
        {
          if( (*childIter) == searchIter->label )
            AddElementsToPSContent( (*searchIter), tempRelationList, newParent );
        }
      }
    }
  }
}

void QmitkLoadSaveToChiliExample::AddElementsToPSContent( mitk::PACSPlugin::PSRelationInformation singleElement, mitk::PACSPlugin::PSRelationInformationList elementList, QListViewItem* parent )
{
  //create element
  QmitkPluginListViewItem* newParent = new QmitkPluginListViewItem( singleElement.oid.c_str(), singleElement.label.c_str(), parent, singleElement.id.c_str() );

  //use all Children
  for( std::list<std::string>::iterator childIter = singleElement.childLabel.begin(); childIter != singleElement.childLabel.end(); childIter++ )
  {
    //search element
    for( mitk::PACSPlugin::PSRelationInformationList::iterator searchIter = elementList.begin(); searchIter != elementList.end(); searchIter++ )
    {
      if( (*childIter) == searchIter->label )
        AddElementsToPSContent( (*searchIter), elementList, newParent );
    }
  }
}

