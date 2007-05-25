/*=========================================================================
 
Program:   Medical Imaging & Interaction Toolkit
Module:    $RCSfile$
Language:  C++
Date:      $Date: 2007-04-19 15:02:32 +0200 (Do, 19 Apr 2007) $
Version:   $Revision: 10185 $
 
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

//reader and writer
#include "mitkLightBoxImageReader.h"
#include "mitkLightBoxResultImageWriter.h"
//to add a new node
#include "mitkDataTreeNodeFactory.h"
//need it for the writer
#include <mitkLevelWindowProperty.h>
//the MITK-Chili-Plugin
#include "mitkChiliPlugin.h"
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
    if( m_Controls->m_TreeNodeSelector->GetSelectedNode() )
    {
      //set the nodeproperty "name" and update the treenodeselector
      m_Controls->m_TreeNodeSelector->GetSelectedNode()->SetProperty( "name", new mitk::StringProperty( temp ) );
      m_Controls->m_TreeNodeSelector->UpdateContent();
    }
    else QMessageBox::information( 0, "ChiliExample", "No node selected!" );
  else QMessageBox::information( 0, "ChiliExample", "We want no empty SeriesDescription!" );
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
    node->SetProperty( "name", new mitk::StringProperty( reader->GetSeriesDescription() ) );
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
        //SET THE INPUT
        //->the image
        writer->SetInput( image );
        //->the level-window-property
        mitk::LevelWindowProperty::Pointer levelWindowProperty = dynamic_cast<mitk::LevelWindowProperty*>( node->GetProperty("levelwindow").GetPointer() );
        if( levelWindowProperty.IsNotNull() )
          writer->SetLevelWindow( levelWindowProperty->GetLevelWindow() );
        //->the seriesdescription
        writer->SetSeriesDescription( node->GetPropertyList()->GetProperty( "name" )->GetValueAsString() );
        //->the lightbox to write
        writer->SetLightBoxToNewLightBox();
        //->you have to set the image, the other inputs get set automatically if there are empty
        writer->Write();//write the image to the lightbox
        //make the cursor normal
        QApplication::restoreOverrideCursor();
      }
      else QMessageBox::information( 0, "ChiliExample", "No image selected!" );
    }
    else QMessageBox::information( 0, "ChiliExample", "There are no data in the node!" );
  }
  else QMessageBox::information( 0, "ChiliExample", "No node selected!" );
}

void QmitkLoadSaveToChiliExample::ImageSelected( mitk::DataTreeIteratorClone imageIt )
{
  //fill the textfield with the new seriesdescription if another node get selected
  m_Controls->DescriptionInput->setText( m_Controls->m_TreeNodeSelector->GetSelectedNode()->GetPropertyList()->GetProperty( "name" )->GetValueAsString().c_str() );
}

void QmitkLoadSaveToChiliExample::chiliStudySelected( const itk::EventObject& )
{
  //show the studydescription
  m_Controls->StudyDescription->setText( mitk::ChiliPlugin::GetInstance()->GetCurrentStudy().StudyDescription.c_str() );
  //clear the listview
  m_Controls->ListView->clear();
  //get the current serieslist and iterate
  mitk::ChiliPlugin::SeriesList temp = mitk::ChiliPlugin::GetInstance()->GetCurrentSeries();
  for( mitk::ChiliPlugin::SeriesList::iterator iter = temp.begin(); iter != temp.end(); iter++)
  {
    //if there are no description show "no description" else show the saved one
    if( iter->SeriesDescription == "" )
      QListViewItem* item = new QListViewItem( m_Controls->ListView, "no description" );
    else QListViewItem* item = new QListViewItem( m_Controls->ListView, iter->SeriesDescription.c_str() );
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
