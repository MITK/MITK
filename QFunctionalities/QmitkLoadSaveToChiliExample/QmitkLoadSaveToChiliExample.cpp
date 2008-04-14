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

//Qt
#include <qaction.h>
#include <qlabel.h>
#include <qlistview.h>
#include <qradiobutton.h>
#include <qmessagebox.h>
#include <qtabwidget.h>
//MITK
#include <mitkProperties.h>
#include <mitkNodePredicateOR.h>
#include <mitkNodePredicateNOT.h>
#include <mitkNodePredicateData.h>
#include <mitkNodePredicateProperty.h>
#include "mitkChiliPluginEvents.h"
#include "mitkDataTreeNodeFactory.h"

#include <QmitkDataTreeViewItem.h>
#include "QmitkStdMultiWidget.h"
#include "QmitkTreeNodeSelector.h"

#include "icon.xpm"

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
    m_ObserverTag = m_Plugin->AddObserver( mitk::PluginStudySelected(), command );
  }
}

QmitkLoadSaveToChiliExample::~QmitkLoadSaveToChiliExample()
{
  if( m_Plugin )
    m_Plugin->RemoveObserver( m_ObserverTag );
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
    CheckPluginCapabilties();
    SetDataTreeIterator( GetDataTreeIterator() );
  }
  return m_Controls;
}

void QmitkLoadSaveToChiliExample::CreateConnections()
{
  if( m_Controls )
  {
    connect( ( QObject* )( m_Controls->LoadFromStudyListView ), SIGNAL( clicked() ), ( QObject* ) this, SLOT( LoadFromStudyListView() ) );
    connect( ( QObject* )( m_Controls->LoadFromPSListView ), SIGNAL( clicked() ), ( QObject* ) this, SLOT( LoadFromPSListView() ) );

    connect( ( QObject* )( m_Controls->SaveDataTree ), SIGNAL( clicked() ), ( QObject* ) this, SLOT( SaveDataTree() ) );
    connect( ( QObject* )( m_Controls->SaveSelectedNodes ), SIGNAL( clicked() ), ( QObject* ) this, SLOT( SaveSingleNodes() ) );

    connect( ( QObject* )( m_Controls->ImageNumberFilter ), SIGNAL( clicked() ), ( QObject* ) this, SLOT( ChangeReaderType() ) );
    connect( ( QObject* )( m_Controls->SpacingFilter ), SIGNAL( clicked() ), ( QObject* ) this, SLOT( ChangeReaderType() ) );
    connect( ( QObject* )( m_Controls->SingleSpacingFilter ), SIGNAL( clicked() ), ( QObject* ) this, SLOT( ChangeReaderType() ) );

    connect( ( QObject* )( m_Controls->tabWidget ), SIGNAL( currentChanged ( QWidget * ) ), ( QObject* ) this, SLOT( CheckPluginCapabilties() ) );
  }
}

void QmitkLoadSaveToChiliExample::CheckPluginCapabilties()
{
  std::string tabLabel = m_Controls->tabWidget->tabLabel( m_Controls->tabWidget->currentPage() ).ascii();

  if( tabLabel == "Load" && m_Plugin && m_Plugin->GetPluginCapabilities().canLoad )
    m_Controls->warningLabel->hide();
  else
    if(  tabLabel == "Save" && m_Plugin && m_Plugin->GetPluginCapabilities().canSave )
      m_Controls->warningLabel->hide();
    else
     m_Controls->warningLabel->show();
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

void QmitkLoadSaveToChiliExample::TreeChanged()
{
  SetDataTreeIterator( GetDataTreeIterator() );
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
          AddNodesToDataTree( m_Plugin->LoadFromSeries( entry->GetChiliOID().ascii() ) );
        break;
      }
      case 1:  //load all known images or all text
      {
        if( m_Controls->studyContent->selectedItem()->text( 0 ) == "Known Images" )
        {
          QmitkPluginListViewItem* entry = dynamic_cast<QmitkPluginListViewItem*>( m_Controls->studyContent->selectedItem() );
          if( entry )
          {
            mitk::PACSPlugin::PSRelationInformationList getAllElements = m_Plugin->GetSeriesRelationInformation( entry->GetChiliOID().ascii() );
            while( !getAllElements.empty() )
            {
              mitk::DataTreeNode::Pointer temp = m_Plugin->LoadParentChildElement( entry->GetChiliOID().ascii(), getAllElements.front().Label );
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
              AddNodesToDataTree( m_Plugin->LoadTextsFromSeries( entry->GetChiliOID().ascii() ) );
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
            mitk::DataTreeNode::Pointer temp = m_Plugin->LoadParentChildElement( entry->GetChiliOID().ascii(), entry->GetVolumeLabel().ascii() );
            if( temp.IsNotNull() )
              mitk::DataStorage::GetInstance()->Add( temp );
          }
          else
            if( parent->text( 0 ) == "Saved Text" )
            {
              QmitkPluginListViewItem* child = dynamic_cast<QmitkPluginListViewItem*>( entry->firstChild() );
              mitk::DataTreeNode::Pointer temp = m_Plugin->LoadSingleText( child->GetChiliOID().ascii() );
              if( temp.IsNotNull() )
                mitk::DataStorage::GetInstance()->Add( temp );
              while( child->nextSibling() != 0 )
              {
                child = dynamic_cast<QmitkPluginListViewItem*>( child->nextSibling() );
                temp = m_Plugin->LoadSingleText( child->GetChiliOID().ascii() );
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
          mitk::DataTreeNode::Pointer temp = m_Plugin->LoadSingleText( entry->GetChiliOID().ascii() );
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
      mitk::DataTreeNode::Pointer temp = m_Plugin->LoadParentChildElement( entry->GetChiliOID().ascii(), entry->GetVolumeLabel().ascii() );
      if( temp.IsNotNull() )
      {
        mitk::DataStorage::GetInstance()->Add( temp );

        m_MultiWidget->InitializeStandardViews( this->GetDataTreeIterator() );
        m_MultiWidget->Fit();
        m_MultiWidget->ReInitializeStandardViews();
      }
    }
  }
}

void QmitkLoadSaveToChiliExample::SaveDataTree()
{
  mitk::NodePredicateProperty w1( "name", mitk::StringProperty::New("Widgets") );
  mitk::NodePredicateProperty w2( "name", mitk::StringProperty::New("widget1Plane") );
  mitk::NodePredicateProperty w3( "name", mitk::StringProperty::New("widget2Plane") );
  mitk::NodePredicateProperty w4( "name", mitk::StringProperty::New("widget3Plane") );
  mitk::NodePredicateProperty w5( "helper object", mitk::BoolProperty::New( true ) );
  mitk::NodePredicateData w6( NULL );
  mitk::NodePredicateOR orpred;
  orpred.AddPredicate( w1 );
  orpred.AddPredicate( w2 );
  orpred.AddPredicate( w3 );
  orpred.AddPredicate( w4 );
  orpred.AddPredicate( w5 );
  orpred.AddPredicate( w6 );
  mitk::NodePredicateNOT notpred( orpred );

  m_Plugin->SaveToChili( mitk::DataStorage::GetInstance()->GetSubset( notpred ) );
}

void QmitkLoadSaveToChiliExample::SaveSingleNodes()
{
  QListViewItemIterator it( m_Controls->DataTreeView, QListViewItemIterator::Selected );
  mitk::DataStorage::SetOfObjects::Pointer resultset = mitk::DataStorage::SetOfObjects::New();
  unsigned int count = 0;
  while ( it.current() )
  {
    QmitkDataTreeViewItem* entry = dynamic_cast<QmitkDataTreeViewItem*>( it.current() );
    if( entry )
    {
      mitk::DataTreeIteratorBase* selectedIterator = entry->GetDataTreeIterator();
      if (selectedIterator != NULL)
      {
        mitk::DataTreeNode* node = selectedIterator->Get();
        if (node != NULL )
        {
          mitk::BaseProperty::Pointer testProperty = node->GetProperty( "helper object" );
          if( testProperty.IsNull() || testProperty->GetValueAsString() == "0" )
          {
            resultset->InsertElement( count, node );
            count++;
          }
        }
      }
    }
    it++;
  }
  if( count != 0 )
    m_Plugin->SaveToChili( mitk::DataStorage::SetOfObjects::ConstPointer( resultset ) );
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
  //set parent-child-relations
  m_Plugin->SetRelationsToDataStorage();

  //initialize the multiwidget (input changed)
  m_MultiWidget->InitializeStandardViews( this->GetDataTreeIterator() );
  m_MultiWidget->Fit();
  m_MultiWidget->ReInitializeStandardViews();
}

void QmitkLoadSaveToChiliExample::PluginEventNewStudySelected( const itk::EventObject& )
{
  m_Controls->studyContent->clear();
  m_Controls->PSContent->clear();

  if( m_Plugin->GetPluginCapabilities().canLoad )
  {
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
          if( singleElement.Image )  //only images should add
          {
            if( psVolumes == NULL )  //no images added before
              psVolumes = new QmitkPluginListViewItem( iter->OID.c_str(), "", seriesParent, "Known Images" );  //create caption
            new QmitkPluginListViewItem( iter->OID.c_str(), singleElement.Label.c_str(), psVolumes , singleElement.ID.c_str() );
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
      if( relationIter->ParentLabel.empty() )
      {
        QmitkPluginListViewItem* newParent = new QmitkPluginListViewItem( relationIter->OID.c_str(), relationIter->Label.c_str(), m_Controls->PSContent, relationIter->ID.c_str() );

        //use all Children
        for( std::list<std::string>::iterator childIter = relationIter->ChildLabel.begin(); childIter != relationIter->ChildLabel.end(); childIter++ )
        {
          //search element
          for( mitk::PACSPlugin::PSRelationInformationList::iterator searchIter = tempRelationList.begin(); searchIter != tempRelationList.end(); searchIter++ )
          {
            if( (*childIter) == searchIter->Label )
              AddElementsToPSContent( (*searchIter), tempRelationList, newParent );
          }
        }
      }
    }
  }
}

void QmitkLoadSaveToChiliExample::AddElementsToPSContent( mitk::PACSPlugin::PSRelationInformation singleElement, mitk::PACSPlugin::PSRelationInformationList elementList, QListViewItem* parent )
{
  //create element
  QmitkPluginListViewItem* newParent = new QmitkPluginListViewItem( singleElement.OID.c_str(), singleElement.Label.c_str(), parent, singleElement.ID.c_str() );

  //use all Children
  for( std::list<std::string>::iterator childIter = singleElement.ChildLabel.begin(); childIter != singleElement.ChildLabel.end(); childIter++ )
  {
    //search element
    for( mitk::PACSPlugin::PSRelationInformationList::iterator searchIter = elementList.begin(); searchIter != elementList.end(); searchIter++ )
    {
      if( (*childIter) == searchIter->Label )
        AddElementsToPSContent( (*searchIter), elementList, newParent );
    }
  }
}

void QmitkLoadSaveToChiliExample::SetDataTreeIterator(mitk::DataTreeIteratorBase* it)
{
  if( it == NULL )
    return;

  while( m_Controls->DataTreeView->firstChild() )
  {
    delete m_Controls->DataTreeView->firstChild();
  }

  m_DataTreeIterator = it;
  mitk::DataTreeIteratorClone tempIt = m_DataTreeIterator;

  if( !tempIt->IsAtEnd() )
  {
    new QmitkDataTreeViewItem( m_Controls->DataTreeView, "Loaded Data", "root", tempIt.GetPointer() );
    ++tempIt;
  }
}
