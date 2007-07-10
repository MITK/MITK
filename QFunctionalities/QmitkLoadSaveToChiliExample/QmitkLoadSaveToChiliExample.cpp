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
//reader and writer
#include "mitkLightBoxImageReader.h"
#include "mitkLightBoxResultImageWriter.h"
//to add a new node
#include "mitkDataTreeNodeFactory.h"
//need it for the writer
#include <mitkLevelWindowProperty.h>
//for the GUI
#include <qcursor.h>
#include <qapplication.h>
#include <qlabel.h>
#include <qlistview.h>
#include <qlineedit.h>
#include <qmessagebox.h>

QmitkLoadSaveToChiliExample::QmitkLoadSaveToChiliExample( QObject *parent, const char *name, QmitkStdMultiWidget *mitkStdMultiWidget, mitk::DataTreeIteratorBase* it )
: QmitkFunctionality( parent, name, it ),
  m_MultiWidget( mitkStdMultiWidget ),
  m_Controls( NULL )
{
  SetAvailability( true );
  // register to chili plugin as observer
  mitk::ChiliPlugin* plugin = mitk::ChiliPlugin::GetInstance();
  if( plugin )
  {
    itk::ReceptorMemberCommand<QmitkLoadSaveToChiliExample>::Pointer command = itk::ReceptorMemberCommand<QmitkLoadSaveToChiliExample>::New();
    command->SetCallbackFunction( this, &QmitkLoadSaveToChiliExample::chiliStudySelected );
    plugin->AddObserver( mitk::PluginStudySelected(), command );
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
    connect( ( QObject* )( m_Controls->m_TreeNodeSelector ), SIGNAL( Activated( mitk::DataTreeIteratorClone ) ), ( QObject* ) this, SLOT( ImageSelected ( mitk::DataTreeIteratorClone ) ) );
    //connect the buttons to the implementation
    connect( ( QObject* )( m_Controls->LoadFromLightBox ), SIGNAL( clicked() ), ( QObject* ) this, SLOT( LoadFromLightbox() ) );
    connect( ( QObject* )( m_Controls->SaveToLightBox ), SIGNAL( clicked() ), ( QObject* ) this, SLOT( SaveToLightbox() ) );
    connect( ( QObject* )( m_Controls->SetDescription ), SIGNAL( clicked() ), ( QObject* ) this, SLOT( SetDescription() ) );
    connect( ( QObject* )( m_Controls->FileDownload ), SIGNAL( clicked() ), ( QObject* ) this, SLOT( FileDownload() ) );
    connect( ( QObject* )( m_Controls->FileUpload ), SIGNAL( clicked() ), ( QObject* ) this, SLOT( FileUpload() ) );
  }
}

QAction * QmitkLoadSaveToChiliExample::CreateAction( QActionGroup *parent )
{
  QAction* action;
  action = new QAction( tr( "Example for Save and Load to Chili" ), QPixmap( (const char**) icon_xpm ), tr( "&Example for Save and Load to Chili" ), 0, parent, "QmitkLoadSaveToChiliExample" );
  return action;
}

void QmitkLoadSaveToChiliExample::SetDescription()
{
  //get description
  std::string temp = m_Controls->DescriptionInput->text();
  //check description and selected node
  if( temp != "" )
  {
    if( m_Controls->m_TreeNodeSelector->GetSelectedNode() )
    {
      //set the nodeproperty "name" and update the treenodeselector
      m_Controls->m_TreeNodeSelector->GetSelectedNode()->SetProperty( "name", new mitk::StringProperty( temp ) );
      m_Controls->m_TreeNodeSelector->UpdateContent();
    }
    else 
    {
      QMessageBox::information( 0, "ChiliExample", "No node selected!" );
    }
  }
  else 
  {
    QMessageBox::information( 0, "ChiliExample", "Please enter a non-empty series description!" );
  }
}

void QmitkLoadSaveToChiliExample::LoadFromLightbox()
{
  //make the cursor to a clock
  QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
  //read the current lightbox
  mitk::LightBoxImageReader::Pointer reader = mitk::LightBoxImageReader::New();
  reader->SetLightBoxToCurrentLightBox();
  mitk::Image::Pointer image = reader->GetOutput();
  image->Update();
  //the image is not initialized if the lightbox is empty
  if( image->IsInitialized() )
  {
    //create a new datatreenode, setdate, set the nodeproperty "name", add
    mitk::DataTreeNode::Pointer node = mitk::DataTreeNode::New();
    node->SetData( image );
    mitk::DataTreeNodeFactory::SetDefaultImageProperties( node );
    mitk::ChiliPlugin::GetInstance()->AddPropertyListToNode( reader->GetImageTagsAsPropertyList(), node.GetPointer() );
    mitk::DataStorage::GetInstance()->Add( node /* , parent */ );
    //initialize the multiwidget (input changed)
    m_MultiWidget->InitializeStandardViews( this->GetDataTreeIterator() );
    m_MultiWidget->Fit();
    m_MultiWidget->ReInitializeStandardViews();
    //update the treenodeselector and make the cursor normal
    m_Controls->m_TreeNodeSelector->UpdateContent();
    QApplication::restoreOverrideCursor();
  }
  else
  {
    QApplication::restoreOverrideCursor();
    QMessageBox::information( 0, "ChiliExample", "The selected Lightbox is empty!" );
  }
}

void QmitkLoadSaveToChiliExample::SaveToLightbox()
{
  //check the current node, we can only save real images
  mitk::DataTreeNode* node = m_Controls->m_TreeNodeSelector->GetSelectedNode();
  if ( node )
  {
    mitk::BaseData* data = node->GetData();
    if ( data )
    {
      mitk::Image* image = dynamic_cast<mitk::Image*>( data );
      if ( image )
      {
        //we got a real image -> make the cursor to a clock, create a writer
        QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
        mitk::LightBoxResultImageWriter::Pointer writer = mitk::LightBoxResultImageWriter::New();
        //SET INPUT (detailed variant)
        //->the image
        writer->SetInput( image );
        //->the level-window-property
        mitk::LevelWindowProperty::Pointer levelWindowProperty = dynamic_cast<mitk::LevelWindowProperty*>( node->GetProperty("levelwindow").GetPointer() );
        if( levelWindowProperty.IsNotNull() )
          writer->SetLevelWindow( levelWindowProperty->GetLevelWindow() );
        //->the seriesdescription
        writer->SetSeriesDescription( node->GetPropertyList()->GetProperty( "name" )->GetValueAsString() );
        //->set the propertylist
        writer->SetPropertyList( node->GetPropertyList() );
        //->the lightbox to write
        writer->SetLightBoxToNewLightBox();
        //->you have to set the image, the other inputs get set automatically if there are empty
        //SET INPUT (short variant)
        //writer->SetInputByDataTreeNode( node );
        writer->Write();//write the image to the lightbox
        //make the cursor normal
        QApplication::restoreOverrideCursor();
      }
      else QMessageBox::information( 0, "ChiliExample", "No image selected!" );
    }
    else QMessageBox::information( 0, "ChiliExample", "There is no data in the node!" );
  }
  else QMessageBox::information( 0, "ChiliExample", "No node selected!" );
}

void QmitkLoadSaveToChiliExample::FileUpload()
{
  if( mitk::ChiliPlugin::GetInstance()->GetChiliVersion() >= 38 )
  {
    mitk::DataStorage::SetOfObjects::ConstPointer m_Result = mitk::DataStorage::GetInstance()->GetAll();
    if ( m_Result->Size() > 0 )
    {
      for( mitk::DataStorage::SetOfObjects::const_iterator go = m_Result->begin(); go != m_Result->end(); go++ )
        mitk::ChiliPlugin::GetInstance()->UploadViaFile( (*go) );
    }
  }
  else QMessageBox::information( 0, "LoadSaveToChiliExample", "Your current Chili version does not support this function." );
}

void QmitkLoadSaveToChiliExample::FileDownload()
{
  if( mitk::ChiliPlugin::GetInstance()->GetChiliVersion() >= 38 )
  {
    QApplication::setOverrideCursor( QCursor( Qt::WaitCursor ) );
    mitk::ChiliPlugin::TextFileList tempTextList = mitk::ChiliPlugin::GetInstance()->GetTextFileInformation();
    for( mitk::ChiliPlugin::TextFileList::iterator iter = tempTextList.begin(); iter != tempTextList.end(); iter++ )
    {
      mitk::ChiliPlugin::GetInstance()->DownloadViaFile( iter->ChiliText, iter->MimeType, m_DataTreeIterator.GetPointer() );
    }
    m_MultiWidget->InitializeStandardViews( this->GetDataTreeIterator() );
    m_MultiWidget->Fit();
    m_MultiWidget->ReInitializeStandardViews();
    QApplication::restoreOverrideCursor();
  }
  else QMessageBox::information( 0, "LoadSaveToChiliExample", "Your current Chili version does not support this function." );
}

void QmitkLoadSaveToChiliExample::ImageSelected( mitk::DataTreeIteratorClone imageIt )
{
  //fill the textfield with the new seriesdescription if another node get selected
  m_Controls->DescriptionInput->setText( m_Controls->m_TreeNodeSelector->GetSelectedNode()->GetPropertyList()->GetProperty( "name" )->GetValueAsString().c_str() );
}

void QmitkLoadSaveToChiliExample::chiliStudySelected( const itk::EventObject& )
{
  //show the studydescription
  if( mitk::ChiliPlugin::GetInstance()->GetCurrentSelectedStudy().Description != "" )
    m_Controls->StudyDescription->setText( mitk::ChiliPlugin::GetInstance()->GetCurrentSelectedStudy().Description.c_str() );
  else m_Controls->StudyDescription->setText( "no description" );
  //clear the listview
  m_Controls->ListView->clear();
  //get the current serieslist and iterate
  mitk::ChiliPlugin::SeriesList tempSeriesList = mitk::ChiliPlugin::GetInstance()->GetCurrentSelectedSeries();
  for( mitk::ChiliPlugin::SeriesList::iterator iter = tempSeriesList.begin(); iter != tempSeriesList.end(); iter++)
  {
    //if there are no description show "no description" else show the saved one
    if( iter->Description == "" )
      new QListViewItem( m_Controls->ListView, "no description" );
    else new QListViewItem( m_Controls->ListView, iter->Description.c_str() );
  }
}

void QmitkLoadSaveToChiliExample::TreeChanged()
{
  m_Controls->m_TreeNodeSelector->SetDataTreeNodeIterator( this->GetDataTreeIterator() );
}

void QmitkLoadSaveToChiliExample::Activated()
{
  QmitkFunctionality::Activated();
}
